#include "mockup_host.hpp"
#include "mockup_scene.hpp"
#include "mockup_monitoring.hpp"
#include "mockup_input_plugin.hpp"
#include <boost/dll/import.hpp>
#include <spdlog/spdlog.h>
#include <string>

#ifndef WIN32
static const std::string scene_plugin_file = "scene_mockup.so";
static const std::string monitoring_plugin_file = "monitoring_mockup.so";
static const std::string input_plugin_file = "input_plugin_mockup.so";
#else
static const std::string scene_plugin_file = "scene_mockup.dll";
static const std::string monitoring_plugin_file = "monitoring_mockup.dll";
static const std::string input_plugin_file = "input_plugin_mockup.dll";
#endif

namespace ear {

MockupHost::MockupHost() { loadPlugins(); }

void MockupHost::loadPlugins() {
  spdlog::debug("loading scene master mockup plugin ({})", scene_plugin_file);
  sceneMasterCreator_ = boost::dll::import_alias<pluginapi_create_t>(
      scene_plugin_file, "createPlugin",
      boost::dll::load_mode::append_decorations);

  spdlog::debug("loading input mockup plugin ({})", input_plugin_file);
  inputPluginCreator_ = boost::dll::import_alias<pluginapi_create_t>(
      input_plugin_file, "createPlugin",
      boost::dll::load_mode::append_decorations);

  spdlog::debug("loading monitoring mockup plugin ({})",
                monitoring_plugin_file);
  monitoringCreator_ = boost::dll::import_alias<pluginapi_create_t>(
      monitoring_plugin_file, "createPlugin",
      boost::dll::load_mode::append_decorations);
}

std::shared_ptr<SceneMockup> MockupHost::insertScenePlugin() {
  auto plugin = sceneMasterCreator_();
  plugins_.push_back(plugin);
  return std::dynamic_pointer_cast<SceneMockup>(plugin);
}

std::shared_ptr<InputPluginMockup> MockupHost::insertInputPlugin() {
  auto plugin = inputPluginCreator_();
  plugins_.push_back(plugin);
  return std::dynamic_pointer_cast<InputPluginMockup>(plugin);
}

std::shared_ptr<MonitoringMockup> MockupHost::insertMonitoringPlugin() {
  auto plugin = monitoringCreator_();
  plugins_.push_back(plugin);
  return std::dynamic_pointer_cast<MonitoringMockup>(plugin);
}

}  // namespace ear
