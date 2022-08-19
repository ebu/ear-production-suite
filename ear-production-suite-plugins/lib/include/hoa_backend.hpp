#pragma once

#include "communication/input_control_connection.hpp"
#include "communication/hoa_metadata_sender.hpp"
#include "ui/hoa_frontend_backend_connector.hpp"
#include "log.hpp"
#include "ear-plugin-base/export.h"
#include <mutex>

namespace ear {
namespace plugin {

class HoaBackend {
 public:
  EAR_PLUGIN_BASE_EXPORT HoaBackend(ui::HoaFrontendBackendConnector* connector);
  EAR_PLUGIN_BASE_EXPORT ~HoaBackend();
  HoaBackend(const HoaBackend&) = delete;
  HoaBackend& operator=(const HoaBackend&) = delete;
  HoaBackend(HoaBackend&&) = delete;
  HoaBackend& operator=(HoaBackend&&) = delete;

  // this will automatically reconnect using the given id
  EAR_PLUGIN_BASE_EXPORT void setConnectionId(communication::ConnectionId id);
  EAR_PLUGIN_BASE_EXPORT void triggerMetadataSend();

  EAR_PLUGIN_BASE_EXPORT communication::ConnectionId getConnectionId() {
    return controlConnection_.getConnectionId();
  }

  // This is the ID originally assigned to this object according to imported ADM
  EAR_PLUGIN_BASE_EXPORT void setImportedId(uint32_t id);

 private:
  void onConnection(communication::ConnectionId connectionId,
                    const std::string& streamEndpoint);
  void onConnectionLost();
  void onParameterChanged(
      ui::HoaFrontendBackendConnector::ParameterId parameter,
      ui::HoaFrontendBackendConnector::ParameterValue value);
  std::shared_ptr<spdlog::logger> logger_;
  ui::HoaFrontendBackendConnector* connector_;
  communication::InputControlConnection controlConnection_;
  communication::HoaMetadataSender metadataSender_;
  std::mutex mutex_;
};
}  // namespace plugin
}  // namespace ear
