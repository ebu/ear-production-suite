#include "ui/binaural_monitoring_frontend_backend_connector.hpp"
#include "log.hpp"

namespace ear {
namespace plugin {
namespace ui {

BinauralMonitoringFrontendBackendConnector::
    BinauralMonitoringFrontendBackendConnector() {}
BinauralMonitoringFrontendBackendConnector::
    ~BinauralMonitoringFrontendBackendConnector() {}

void BinauralMonitoringFrontendBackendConnector::onParameterChanged(
    ParameterChangedCallback callback) {
  paramCallback_ = callback;
}

void BinauralMonitoringFrontendBackendConnector::notifyParameterChanged(
    ParameterId parameterID, const ParameterValue& newValue) {
  if (paramCallback_) {
    paramCallback_(parameterID, newValue);
  }
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
