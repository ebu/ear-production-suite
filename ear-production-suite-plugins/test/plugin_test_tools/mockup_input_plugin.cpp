#include "mockup_plugin_base.hpp"
#include "mockup_input_plugin.hpp"
#include <spdlog/spdlog.h>
#include <memory>

namespace ear {

InputPluginMockup::InputPluginMockup() {
  spdlog::info("InputPluginMockup plugin created");
}

InputPluginMockup::~InputPluginMockup() {
  spdlog::info("InputPluginMockup plugin destroyed");
}

}  // namespace ear
