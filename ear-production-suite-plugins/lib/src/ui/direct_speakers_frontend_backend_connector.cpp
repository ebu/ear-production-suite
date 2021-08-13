#include "ui/direct_speakers_frontend_backend_connector.hpp"
#include "log.hpp"

namespace ear {
namespace plugin {
namespace ui {

DirectSpeakersFrontendBackendConnector::
    DirectSpeakersFrontendBackendConnector() {}
DirectSpeakersFrontendBackendConnector::
    ~DirectSpeakersFrontendBackendConnector() {}

void DirectSpeakersFrontendBackendConnector::onParameterChanged(
    ParameterChangedCallback callback) {
  paramCallback_ = callback;
}

void DirectSpeakersFrontendBackendConnector::notifyParameterChanged(
    ParameterId parameterID, const ParameterValue& newValue) {
  if (paramCallback_) {
    paramCallback_(parameterID, newValue);
  }
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
