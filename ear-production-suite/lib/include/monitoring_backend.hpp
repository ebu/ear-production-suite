#pragma once

#include "communication/monitoring_control_connection.hpp"
#include "scene_store.pb.h"
#include "log.hpp"
#include "ear-plugin-base/export.h"
#include "scene_gains_calculator.hpp"

#include <string>
#include <memory>
#include <mutex>

namespace ear {
namespace plugin {
namespace ui {
class EAR_PLUGIN_BASE_EXPORT MonitoringFrontendBackendConnector {};
}  // namespace ui
namespace communication {
class MonitoringMetadataReceiver;
}

class MonitoringBackend {
 public:
  EAR_PLUGIN_BASE_EXPORT MonitoringBackend(
      ui::MonitoringFrontendBackendConnector*, const Layout& targetLayout,
      int inputChannelCount);
  EAR_PLUGIN_BASE_EXPORT ~MonitoringBackend();
  MonitoringBackend(const MonitoringBackend&) = delete;
  MonitoringBackend(MonitoringBackend&&) = delete;
  MonitoringBackend& operator=(MonitoringBackend&&) = delete;
  MonitoringBackend& operator=(const MonitoringBackend&) = delete;

  GainHolder currentGains();

 private:
  void onSceneReceived(proto::SceneStore store);
  void onConnection(communication::ConnectionId connectionId,
                    const std::string& streamEndpoint);
  void onConnectionLost();
  void updateActiveGains(proto::SceneStore store);

  std::shared_ptr<spdlog::logger> logger_;
  std::mutex gainsMutex_;
  GainHolder gains_;
  std::mutex gainsCalculatorMutex_;
  SceneGainsCalculator gainsCalculator_;
  ui::MonitoringFrontendBackendConnector* frontendConnector_;
  std::unique_ptr<communication::MonitoringMetadataReceiver> metadataReceiver_;
  communication::MonitoringControlConnection controlConnection_;

};

}  // namespace plugin
}  // namespace ear
