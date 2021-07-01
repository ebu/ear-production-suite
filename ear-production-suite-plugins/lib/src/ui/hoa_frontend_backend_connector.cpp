#include "ui/hoa_frontend_backend_connector.hpp"
#include "log.hpp"

namespace ear {
namespace plugin {
namespace ui {

HoaFrontendBackendConnector::HoaFrontendBackendConnector() {}
HoaFrontendBackendConnector::~HoaFrontendBackendConnector() {}

void HoaFrontendBackendConnector::onParameterChanged(
    ParameterChangedCallback callback) {
  paramCallback_ = callback;
}

void HoaFrontendBackendConnector::notifyParameterChanged(//(3.)
    ParameterId parameterID, const ParameterValue& newValue) {
  if (paramCallback_) {
    paramCallback_(parameterID, newValue);
  }
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
