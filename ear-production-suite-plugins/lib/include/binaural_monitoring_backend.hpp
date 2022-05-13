#pragma once

#include "communication/monitoring_control_connection.hpp"
#include "scene_store.pb.h"
#include "log.hpp"
#include "ear-plugin-base/export.h"
#include "scene_gains_calculator.hpp"
#include "listener_orientation.hpp"
#include "helper/common_definition_helper.h"

#include <string>
#include <memory>
#include <mutex>
#include <optional>
#include <map>

using ConnId = std::string;

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
  std::vector<ConnId> getActiveHoaIds();
  size_t getTotalHoaChannels();

  struct ObjectsEarMetadataAndRouting {
    int channel;
    ear::ObjectsTypeMetadata earMetadata;
  };

  struct DirectSpeakersEarMetadataAndRouting {
    int startingChannel;
    std::vector<ear::DirectSpeakersTypeMetadata> earMetadata;
  };

  struct HoaEarMetadataAndRouting {
    int startingChannel;
    ear::HOATypeMetadata earMetadata;
  };

  std::optional<ObjectsEarMetadataAndRouting> getLatestObjectsTypeMetadata(
      ConnId id);
  std::optional<DirectSpeakersEarMetadataAndRouting>
  getLatestDirectSpeakersTypeMetadata(ConnId id);
  std::optional<HoaEarMetadataAndRouting> getLatestHoaTypeMetadata(ConnId id);

  std::shared_ptr<ear::plugin::ListenerOrientation> listenerOrientation;

  bool isExporting() { return isExporting_; }

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
  size_t directSpeakersChannelCount{0};
  std::mutex activeObjectIdsMutex_;
  std::vector<ConnId> activeObjectIds;
  size_t objectChannelCount{0};
  std::mutex activeHoaIdsMutex_;
  std::vector<ConnId> activeHoaIds;
  size_t hoaChannelCount{0};

  std::vector<ConnId> allActiveIds;

  std::mutex latestDirectSpeakersTypeMetadataMutex_;
  std::map<ConnId, DirectSpeakersEarMetadataAndRouting>
      latestDirectSpeakersTypeMetadata;
  std::mutex latestObjectsTypeMetadataMutex_;
  std::map<ConnId, ObjectsEarMetadataAndRouting> latestObjectsTypeMetadata;
  std::mutex latestHoaTypeMetadataMutex_;
  std::map<ConnId, HoaEarMetadataAndRouting> latestHoaTypeMetadata;

  std::shared_ptr<spdlog::logger> logger_;
  ui::MonitoringFrontendBackendConnector* frontendConnector_;
  std::unique_ptr<communication::MonitoringMetadataReceiver> metadataReceiver_;
  communication::MonitoringControlConnection controlConnection_;

  std::mutex commonDefinitionHelperMutex_;
  AdmCommonDefinitionHelper commonDefinitionHelper{};

  bool isExporting_{ false };
};

}  // namespace plugin
}  // namespace ear
