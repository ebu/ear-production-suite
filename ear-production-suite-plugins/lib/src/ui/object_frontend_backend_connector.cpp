#include "ui/object_frontend_backend_connector.hpp"
#include "log.hpp"

namespace ear {
namespace plugin {
namespace ui {

ObjectsFrontendBackendConnector::ObjectsFrontendBackendConnector() {}
ObjectsFrontendBackendConnector::~ObjectsFrontendBackendConnector() {}

void ObjectsFrontendBackendConnector::onParameterChanged(
    ParameterChangedCallback callback) {
  paramCallback_ = callback;
}

void ObjectsFrontendBackendConnector::notifyParameterChanged(
    ParameterId parameterID, const ParameterValue& newValue) {
  if (paramCallback_) {
    paramCallback_(parameterID, newValue);
  }
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
