#include "mockup_monitoring.hpp"
#include <boost/dll/alias.hpp>
#include <memory>

namespace ear {

std::shared_ptr<MonitoringMockup> createMonitoringPlugin() {
  return std::make_shared<MonitoringMockup>();
}

BOOST_DLL_ALIAS(ear::createMonitoringPlugin, createPlugin);

}  // namespace ear
