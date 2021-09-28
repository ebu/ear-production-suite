#include "scene_gains_calculator.hpp"
#include "ear/metadata.hpp"
#include "helper/eps_to_ear_metadata_converter.hpp"
#include <algorithm>

namespace ear {
namespace plugin {

SceneGainsCalculator::SceneGainsCalculator(ear::Layout outputLayout,
                                           int inputChannelCount)
    : objectCalculator_{outputLayout}, directSpeakersCalculator_{outputLayout} {
  resize(outputLayout, static_cast<std::size_t>(inputChannelCount));
  allActiveIds.reserve(inputChannelCount);
}

bool SceneGainsCalculator::update(proto::SceneStore store) {
  for (auto id : removedIds(store)) {
    auto routing = routingCache_[id];
    for (int i = 0; i < routing.size; ++i) {
      std::fill(direct_[routing.track + i].begin(),
                direct_[routing.track + i].end(), 0.0f);
      std::fill(diffuse_[routing.track + i].begin(),
                diffuse_[routing.track + i].end(), 0.0f);
    }
    routingCache_.erase(id);
  }
  for (auto routing : updateRoutingCache(store)) {
    for (int i = 0; i < routing.size; ++i) {
      std::fill(direct_[routing.track + i].begin(),
                direct_[routing.track + i].end(), 0.0f);
      std::fill(diffuse_[routing.track + i].begin(),
                diffuse_[routing.track + i].end(), 0.0f);
    }
  }

  for (const auto& item : store.items()) {
    bool newItem = std::find(allActiveIds.begin(), allActiveIds.end(), item.connection_id()) == allActiveIds.end();
    if (newItem || item.changed()) {
      if (item.has_ds_metadata()) {
        auto earMetadata =
            EpsToEarMetadataConverter::convert(item.ds_metadata());
        auto routing = static_cast<std::size_t>(item.routing());
        if (item.routing() >= 0 &&
            routing + earMetadata.size() < direct_.size()) {
          for (int i = 0; i < earMetadata.size(); ++i) {
            directSpeakersCalculator_.calculate(earMetadata.at(i),
                                                direct_[routing + i]);
          }
        }
      }
      if (item.has_mtx_metadata()) {
        throw std::runtime_error("received unsupported Matrix type metadata");
      }
      if (item.has_obj_metadata()) {
        auto earMetadata =
            EpsToEarMetadataConverter::convert(item.obj_metadata());
        auto routing = static_cast<std::size_t>(item.routing());
        if (item.routing() >= 0 && routing < direct_.size()) {
          objectCalculator_.calculate(earMetadata, direct_[routing],
                                      diffuse_[routing]);
        }
      }
      if (item.has_hoa_metadata()) {
        throw std::runtime_error("received unsupported HOA type metadata");
      }
      if (item.has_bin_metadata()) {
        throw std::runtime_error("received unsupported binaural type metadata");
      }
    }
  }

  // Used for setting the newItem flag next time around
  allActiveIds.clear();
  for(const auto& item : store.items()) {
    allActiveIds.push_back(item.connection_id());
  }

  return true;
}

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
}  // namespace

Eigen::MatrixXf SceneGainsCalculator::directGains() {
  return toEigenMat(direct_);
}

Eigen::MatrixXf SceneGainsCalculator::diffuseGains() {
  return toEigenMat(diffuse_);
}

void SceneGainsCalculator::clear() {
  for (auto& gainVec : direct_) {
    std::fill(gainVec.begin(), gainVec.end(), 0.0f);
  }
  for (auto& gainVec : diffuse_) {
    std::fill(gainVec.begin(), gainVec.end(), 0.0f);
  }
}

std::vector<communication::ConnectionId> SceneGainsCalculator::removedIds(
    const proto::SceneStore& store) const {
  std::vector<communication::ConnectionId> removedIds;
  for (auto& entry : routingCache_) {
    auto id = entry.first;
    auto it = std::find_if(
        store.items().begin(), store.items().end(), [id](auto& item) {
          return communication::ConnectionId{item.connection_id()} == id;
        });
    if (it == store.items().end()) {
      removedIds.push_back(entry.first);
    }
  }
  return removedIds;
}

std::vector<Routing> SceneGainsCalculator::updateRoutingCache(
    const proto::SceneStore& store) {
  std::vector<Routing> changedRouting;
  for (auto& item : store.items()) {
    if (item.changed()) {
      int size = 1;
      if (item.has_ds_metadata()) {
        size = item.ds_metadata().speakers().size();
      }
      Routing newRouting{item.routing(), size};
      auto it = routingCache_.find(item.connection_id());
      if (it != routingCache_.end() && (it->second.track != newRouting.track ||
                                        it->second.size != newRouting.size)) {
        changedRouting.push_back(it->second);
      }
      if (newRouting.track >= 0) {
        routingCache_[item.connection_id()] = newRouting;
      } else {
        routingCache_.erase(item.connection_id());
      }
    }
  }
  return changedRouting;
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

}  // namespace plugin
}  // namespace ear
