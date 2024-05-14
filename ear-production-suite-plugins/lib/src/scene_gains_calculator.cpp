#include "scene_gains_calculator.hpp"
#include "ear/metadata.hpp"
#include "helper/eps_to_ear_metadata_converter.hpp"
#include "helper/container_helpers.hpp"
#include <algorithm>

namespace {

int inputCount(ear::plugin::ItemGains const& itemGains) {
  assert(itemGains.direct_.size() == itemGains.diffuse_.size());
  return static_cast<int>(itemGains.direct_.size());
}

void addToEigenMat(Eigen::MatrixXf& mat,
                   std::vector<std::vector<float>> const& gainsTable,
                   int inputStartingChannelOffset) {
  for (int inputChannelCounter = 0; inputChannelCounter < gainsTable.size(); ++inputChannelCounter) {
    int inputChannel = inputStartingChannelOffset + inputChannelCounter;
    if (inputChannel < mat.cols()) {
      mat.col(inputChannel) += Eigen::VectorXf::Map(gainsTable[inputChannelCounter].data(),
                               gainsTable[inputChannelCounter].size());
    }
  }
}

void resize2dVector(std::vector<std::vector<float>>& vec, int inputs,
                    int outputs, float fill = 0.f) {
  vec.resize(inputs);
  for (auto& subVec : vec) {
    subVec.resize(outputs, fill);
  }
}

void resizeGainTables(ear::plugin::ItemGains& itemGains, 
            int inputCount, int outputCount) {
  resize2dVector(itemGains.direct_, inputCount, outputCount);
  resize2dVector(itemGains.diffuse_, inputCount, outputCount);
}

}


namespace ear {
namespace plugin {

SceneGainsCalculator::SceneGainsCalculator(ear::Layout outputLayout,
                                           int inputChannelCount)
    : objectCalculator_{outputLayout},
      directSpeakersCalculator_{outputLayout},
      hoaCalculator_{outputLayout},
      totalOutputChannels{static_cast<int>(outputLayout.channels().size())},
      totalInputChannels{inputChannelCount} {}

bool SceneGainsCalculator::update(const proto::SceneStore& store) {
  // First figure out what we need to process updates for
  std::vector<communication::ConnectionId> cachedIdsChecklist;
  cachedIdsChecklist.reserve(routingCache_.size());
  for(auto const&[key, val] : routingCache_) {
    cachedIdsChecklist.push_back(key);
  }
  /// Check-off found items, and also delete changed items from routing cache to be re-evaluated
  for(const auto& item : store.monitoring_items()) {
    auto itemId = communication::ConnectionId{ item.connection_id() };
    cachedIdsChecklist.erase(std::remove(cachedIdsChecklist.begin(), cachedIdsChecklist.end(), itemId), cachedIdsChecklist.end());
    if(item.changed()) {
      removeItem(itemId);
    }
  }
  /// Delete removed items from routing cache  (i.e, those that weren't checked-off and therefore remain in cachedIdsChecklist)
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

  return true;
}

Eigen::MatrixXf SceneGainsCalculator::directGains() {
  Eigen::MatrixXf mat = Eigen::MatrixXf::Zero(totalOutputChannels, totalInputChannels);
  for (auto const& [itemId, routing] : routingCache_) {
    if (routing.inputStartingChannel >= 0 &&
        (routing.inputStartingChannel + inputCount(routing)) <
            totalInputChannels) {
      addToEigenMat(mat, routing.direct_, routing.inputStartingChannel);
    }
  }
  return mat;
}

Eigen::MatrixXf SceneGainsCalculator::diffuseGains() {
  Eigen::MatrixXf mat = Eigen::MatrixXf::Zero(totalOutputChannels, totalInputChannels);
  for (auto const& [itemId, routing] : routingCache_) {
    if (routing.inputStartingChannel >= 0 &&
        (routing.inputStartingChannel + inputCount(routing)) <
            totalInputChannels) {
      addToEigenMat(mat, routing.diffuse_, routing.inputStartingChannel);
    }
  }
  return mat;
}

void SceneGainsCalculator::removeItem(const communication::ConnectionId &itemId)
{
  if (mapHasKey(routingCache_, itemId)) {
    routingCache_.erase(itemId);
  }
}

void SceneGainsCalculator::addOrUpdateItem(const proto::MonitoringItemMetadata & item)
{
  ItemGains* routing = setInMap(routingCache_, communication::ConnectionId{ item.connection_id() }, {});

  if(item.has_ds_metadata()) {
    auto earMetadata = EpsToEarMetadataConverter::convert(item.ds_metadata());
    routing->inputStartingChannel = item.routing();
    int inputChannelCount = static_cast<int>(earMetadata.size());
    resizeGainTables(*routing, static_cast<int>(earMetadata.size()),
                     totalOutputChannels);
    for (int inputChannelCounter = 0; inputChannelCounter < inputChannelCount; inputChannelCounter++) {
      directSpeakersCalculator_.calculate(
          earMetadata.at(inputChannelCounter),
          routing->direct_[inputChannelCounter]);
    }
  }

  if(item.has_obj_metadata()) {
    auto earMetadata = EpsToEarMetadataConverter::convert(item.obj_metadata());
    routing->inputStartingChannel = item.routing();
    resizeGainTables(*routing, 1, totalOutputChannels);
    objectCalculator_.calculate(earMetadata,
                                routing->direct_[0],
                                routing->diffuse_[0]);
  }

  if(item.has_hoa_metadata()) {
    ear::HOATypeMetadata earMetadata;
    earMetadata = EpsToEarMetadataConverter::convert(item.hoa_metadata());
    routing->inputStartingChannel = item.routing();
    int inputChannelCount = static_cast<int>(earMetadata.degrees.size());
    resizeGainTables(*routing, inputChannelCount, totalOutputChannels);
    hoaCalculator_.calculate(earMetadata, routing->direct_);
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
