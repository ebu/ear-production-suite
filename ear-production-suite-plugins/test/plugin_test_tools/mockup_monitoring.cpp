#include "mockup_plugin_base.hpp"
#include "mockup_monitoring.hpp"
#include <spdlog/spdlog.h>
#include <memory>

namespace ear {

MonitoringMockup::MonitoringMockup() {
  spdlog::info("MonitoringMockup plugin created");
}

MonitoringMockup::~MonitoringMockup() {
  spdlog::info("MonitoringMockup plugin destroyed");
}

}  // namespace ear
