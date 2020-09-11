#pragma once

#include "communication/input_control_connection.hpp"
#include "communication/object_metadata_sender.hpp"
#include "ui/object_frontend_backend_connector.hpp"
#include "log.hpp"
#include "ear-plugin-base/export.h"
#include <mutex>

namespace ear {
namespace plugin {

class ObjectBackend {
 public:
  EAR_PLUGIN_BASE_EXPORT ObjectBackend(
      ui::ObjectsFrontendBackendConnector* connector);
  EAR_PLUGIN_BASE_EXPORT ~ObjectBackend();
  ObjectBackend(const ObjectBackend&) = delete;
  ObjectBackend& operator=(const ObjectBackend&) = delete;
  ObjectBackend(ObjectBackend&&) = delete;
  ObjectBackend& operator=(ObjectBackend&&) = delete;

  // this will automatically reconnect using the given id
  EAR_PLUGIN_BASE_EXPORT void setConnectionId(communication::ConnectionId id);
  EAR_PLUGIN_BASE_EXPORT void triggerMetadataSend();

  EAR_PLUGIN_BASE_EXPORT communication::ConnectionId getConnectionId() {
    return controlConnection_.getConnectionId();
  }

 private:
  void onConnection(communication::ConnectionId connectionId,
                    const std::string& streamEndpoint);
  void onConnectionLost();
  void onParameterChanged(
      ui::ObjectsFrontendBackendConnector::ParameterId parameter,
      ui::ObjectsFrontendBackendConnector::ParameterValue value);
  std::shared_ptr<spdlog::logger> logger_;
  ui::ObjectsFrontendBackendConnector* connector_;
  communication::InputControlConnection controlConnection_;
  communication::ObjectMetadataSender metadataSender_;
  std::mutex mutex_;
};
}  // namespace plugin
}  // namespace ear
