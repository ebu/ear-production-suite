#include "scene_gains_calculator.hpp"
#include "ear/metadata.hpp"
#include "helper/eps_to_ear_metadata_converter.hpp"
#include <future>
#include <algorithm>


namespace {

Eigen::MatrixXf toEigenMat(std::vector<std::vector<float>>& vec) {
  auto outputCount = vec.size();
  auto inputCount = static_cast<int>(vec[0].size());
  Eigen::MatrixXf mat(inputCount, outputCount);
  for (std::size_t i = 0; i < outputCount; i++) {
    mat.col(static_cast<int>(i)) =
      Eigen::VectorXf::Map(vec[i].data(), inputCount);
  }
  return mat;
}

template <typename Key, typename Value>
Value* getValuePointerFromMap(std::map<Key, Value>& targetMap, Key key) {
  auto it = targetMap.find(key);
  if (it == targetMap.end()) return nullptr;
  return &(it->second);
}

template <typename Key, typename Value>
Value* setInMap(std::map<Key, Value>& targetMap, Key key, Value value) {
  auto ins = targetMap.insert_or_assign(key, value);
  return &(ins.first->second);
}

template <typename Key, typename Value>
bool mapHasKey(std::map<Key, Value>& targetMap, Key key) {
  auto it = targetMap.find(key);
  return (it != targetMap.end());
}

}


namespace ear {
namespace plugin {

SceneGainsCalculator::SceneGainsCalculator(ear::Layout outputLayout,
                                           int inputChannelCount)
    : objectCalculator_{outputLayout},
      directSpeakersCalculator_{outputLayout},
      hoaCalculator_{outputLayout} {
  resize(outputLayout, static_cast<std::size_t>(inputChannelCount));
  commonDefinitionHelper_.getElementRelationships();
}

bool SceneGainsCalculator::update(proto::SceneStore store) {
  // Called by NNG callback on thread with small stack.
  // Launch task in another thread to overcome stack limitation.
  auto future = std::async(std::launch::async, [this, store]() {

    // First figure out what we need to process updates for
    std::vector<communication::ConnectionId> cachedIdsChecklist;
    cachedIdsChecklist.reserve(routingCache_.size());
    for(auto const&[key, val] : routingCache_) {
      cachedIdsChecklist.push_back(key);
    }
    /// Check-off found items, and also zero original gains for changed items and delete from routing cache to be re-evaluated
    for(const auto& item : store.monitoring_items()) {
      auto itemId = communication::ConnectionId{ item.connection_id() };
      cachedIdsChecklist.erase(std::remove(cachedIdsChecklist.begin(), cachedIdsChecklist.end(), itemId), cachedIdsChecklist.end());
      if(item.changed()) {
        removeItem(itemId);
      }
    }
    /// Zero original gains for removed items and delete from routing cache  (i.e, those that weren't checked-off and therefore remain in cachedIdsChecklist)
    for(const auto& itemId : cachedIdsChecklist) {
      removeItem(itemId);
    }

    // Now get the gain updates we need
    for(const auto& item : store.monitoring_items()) {
      /// If it's not in routingCache_, it's new or changed, so needs re-evaluating
      if(!mapHasKey(routingCache_, communication::ConnectionId{ item.connection_id() })) {
        addOrUpdateItem(item);
      }
    }

  });

  future.get();

  return true;
}

Eigen::MatrixXf SceneGainsCalculator::directGains() {
  return toEigenMat(direct_);
}

Eigen::MatrixXf SceneGainsCalculator::diffuseGains() {
  return toEigenMat(diffuse_);
}

void SceneGainsCalculator::resize(ear::Layout& outputLayout,
                                  std::size_t inputChannelCount) {
  direct_.resize(inputChannelCount);
  diffuse_.resize(inputChannelCount);
  auto outputChannelCount = outputLayout.channels().size();
  for (auto& gainVec : direct_) {
    gainVec.resize(outputChannelCount, 0.0f);
  }
  for (auto& gainVec : diffuse_) {
    gainVec.resize(outputChannelCount, 0.0f);
  }
}

void SceneGainsCalculator::removeItem(const communication::ConnectionId &itemId)
{
  auto itemRouting = getValuePointerFromMap(routingCache_, itemId);
  if(itemRouting) {
    if(itemRouting->inputStartingChannel >= 0) {
      int channelLim = std::min((int)direct_.size(), itemRouting->inputStartingChannel + itemRouting->inputChannelCount);
      for(int ch = itemRouting->inputStartingChannel; ch < channelLim; ch++) {
        std::fill(direct_[ch].begin(), direct_[ch].end(), 0.0f);
        std::fill(diffuse_[ch].begin(), diffuse_[ch].end(), 0.0f);
      }
    }
    routingCache_.erase(itemId);
  }
}

void SceneGainsCalculator::addOrUpdateItem(const proto::MonitoringItemMetadata & item)
{
  ItemRouting* routing = setInMap(routingCache_, communication::ConnectionId{ item.connection_id() }, {});

  if(item.has_ds_metadata()) {
    auto earMetadata = EpsToEarMetadataConverter::convert(item.ds_metadata());
    routing->inputStartingChannel = item.routing();
    routing->inputChannelCount = earMetadata.size();

    if(routing->inputStartingChannel > 0) {
      for(int i = 0; i < routing->inputChannelCount; i++) {
        auto inputChannel = routing->inputStartingChannel + i;
        if(inputChannel >= 0 && inputChannel < direct_.size()) {
          directSpeakersCalculator_.calculate(earMetadata.at(i),
                                              direct_[inputChannel]);
        }
      }
    }
  }

  if(item.has_obj_metadata()) {
    auto earMetadata = EpsToEarMetadataConverter::convert(item.obj_metadata());
    routing->inputStartingChannel = item.routing();
    routing->inputChannelCount = 1;

    if(routing->inputStartingChannel >= 0 && routing->inputStartingChannel < direct_.size()) {
      objectCalculator_.calculate(earMetadata, direct_[routing->inputStartingChannel],
                                  diffuse_[routing->inputStartingChannel]);
    }
  }

  if(item.has_hoa_metadata()) {
    ear::HOATypeMetadata earMetadata;
    {
      std::lock_guard<std::mutex> lock(commonDefinitionHelperMutex_);
      earMetadata = EpsToEarMetadataConverter::convert(item.hoa_metadata(), commonDefinitionHelper_);
    }
    routing->inputStartingChannel = item.routing();
    routing->inputChannelCount = earMetadata.degrees.size();

    if(routing->inputStartingChannel > 0) {
      std::vector<std::vector<float>> hoaGains(
        routing->inputChannelCount,
        std::vector<float>(direct_[0].size(), 0));
      hoaCalculator_.calculate(earMetadata, hoaGains);

      for(int i = 0; i < routing->inputChannelCount; i++) {
        auto inputChannel = routing->inputStartingChannel + i;
        if(inputChannel >= 0 && inputChannel < direct_.size()) {
          direct_[inputChannel] = hoaGains[i];
        }
      }
    }
  }

  if(item.has_bin_metadata()) {
    throw std::runtime_error(
      "received unsupported binaural type metadata");
  }
  if(item.has_mtx_metadata()) {
    throw std::runtime_error("received unsupported Matrix type metadata");
  }
}

}  // namespace plugin
}  // namespace ear
