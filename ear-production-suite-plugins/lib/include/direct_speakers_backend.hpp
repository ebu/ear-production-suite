#pragma once

#include "communication/input_control_connection.hpp"
#include "communication/direct_speakers_metadata_sender.hpp"
#include "ui/direct_speakers_frontend_backend_connector.hpp"
#include "log.hpp"
#include "ear-plugin-base/export.h"
#include <mutex>

namespace ear {
namespace plugin {

class DirectSpeakersBackend {
 public:
  EAR_PLUGIN_BASE_EXPORT DirectSpeakersBackend(
      ui::DirectSpeakersFrontendBackendConnector* connector);
  EAR_PLUGIN_BASE_EXPORT ~DirectSpeakersBackend();
  DirectSpeakersBackend(const DirectSpeakersBackend&) = delete;
  DirectSpeakersBackend& operator=(const DirectSpeakersBackend&) = delete;
  DirectSpeakersBackend(DirectSpeakersBackend&&) = delete;
  DirectSpeakersBackend& operator=(DirectSpeakersBackend&&) = delete;

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
      ui::DirectSpeakersFrontendBackendConnector::ParameterId parameter,
      ui::DirectSpeakersFrontendBackendConnector::ParameterValue value);
  std::shared_ptr<spdlog::logger> logger_;
  ui::DirectSpeakersFrontendBackendConnector* connector_;
  communication::InputControlConnection controlConnection_;
  communication::DirectSpeakersMetadataSender metadataSender_;
  std::mutex mutex_;
};
}  // namespace plugin
}  // namespace ear
