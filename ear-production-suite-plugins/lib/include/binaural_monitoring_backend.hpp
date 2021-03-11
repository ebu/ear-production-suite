#pragma once

#include "communication/monitoring_control_connection.hpp"
#include "scene_store.pb.h"
#include "log.hpp"
#include "ear-plugin-base/export.h"
#include "scene_gains_calculator.hpp"

#include <string>
#include <memory>
#include <mutex>
#include <optional>
#include <map>

using ConnId = std::string;

template <typename Key, typename Value>
std::optional<Value> getFromMap(std::map<Key, Value>& targetMap, Key key) {
  auto it = targetMap.find(key);
  if (it == targetMap.end()) return std::optional<Value>();
  return std::optional<Value>(it->second);
}

template <typename Key, typename Value>
void removeFromMap(std::map<Key, Value>& targetMap, Key key) {
  auto it = targetMap.find(key);
  if (it != targetMap.end()) targetMap.erase(it);
}

template <typename Key, typename Value>
Value* getValuePointerFromMap(std::map<Key, Value>& targetMap, Key key) {
  auto it = targetMap.find(key);
  if (it == targetMap.end()) return nullptr;
  return &(it->second);
}

template <typename Key, typename Value>
void setInMap(std::map<Key, Value>& targetMap, Key key, Value value) {
  auto it = targetMap.find(key);
  if (it == targetMap.end()) {
    targetMap.insert(std::make_pair(key, value));
  } else {
    it->second = value;
  }
}

namespace ear {
namespace plugin {
namespace ui {
class EAR_PLUGIN_BASE_EXPORT MonitoringFrontendBackendConnector {};
}  // namespace ui
namespace communication {
class MonitoringMetadataReceiver;
}

class BinauralMonitoringBackend {
 public:
  EAR_PLUGIN_BASE_EXPORT BinauralMonitoringBackend(
      ui::MonitoringFrontendBackendConnector*, int inputChannelCount);
  EAR_PLUGIN_BASE_EXPORT ~BinauralMonitoringBackend();
  BinauralMonitoringBackend(const BinauralMonitoringBackend&) = delete;
  BinauralMonitoringBackend(BinauralMonitoringBackend&&) = delete;
  BinauralMonitoringBackend& operator=(BinauralMonitoringBackend&&) = delete;
  BinauralMonitoringBackend& operator=(const BinauralMonitoringBackend&) =
      delete;

  std::vector<ConnId> getActiveObjectIds();
  size_t getTotalObjectChannels();
  std::vector<ConnId> getActiveDirectSpeakersIds();
  size_t getTotalDirectSpeakersChannels();

  struct ObjectsEarMetadataAndRouting {
    int channel;
    ear::ObjectsTypeMetadata earMetadata;
  };

  struct DirectSpeakersEarMetadataAndRouting {
    int startingChannel;
    std::vector<ear::DirectSpeakersTypeMetadata> earMetadata;
  };

  std::optional<ObjectsEarMetadataAndRouting> getLatestObjectsTypeMetadata(ConnId id);
  std::optional<DirectSpeakersEarMetadataAndRouting> getLatestDirectSpeakersTypeMetadata(ConnId id);

 private:
  void onSceneReceived(proto::SceneStore store);
  void onConnection(communication::ConnectionId connectionId,
                    const std::string& streamEndpoint);
  void onConnectionLost();

  std::mutex latestMonitoringItemMetadataMutex_;
  std::map<ConnId, ear::plugin::proto::MonitoringItemMetadata>
      latestMonitoringItemMetadata;

  std::mutex activeDirectSpeakersIdsMutex_;
  std::vector<ConnId> activeDirectSpeakersIds;
  size_t directSpeakersChannelCount{ 0 };
  std::mutex activeObjectIdsMutex_;
  std::vector<ConnId> activeObjectIds;
  size_t objectChannelCount{ 0 };

  std::mutex latestDirectSpeakersTypeMetadataMutex_;
  std::map<ConnId, DirectSpeakersEarMetadataAndRouting>
      latestDirectSpeakersTypeMetadata;
  std::mutex latestObjectsTypeMetadataMutex_;
  std::map<ConnId, ObjectsEarMetadataAndRouting>
      latestObjectsTypeMetadata;

  std::shared_ptr<spdlog::logger> logger_;
  ui::MonitoringFrontendBackendConnector* frontendConnector_;
  std::unique_ptr<communication::MonitoringMetadataReceiver> metadataReceiver_;
  communication::MonitoringControlConnection controlConnection_;
};

}  // namespace plugin
}  // namespace ear
