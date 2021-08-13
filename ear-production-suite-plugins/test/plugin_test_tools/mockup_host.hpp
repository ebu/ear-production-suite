#pragma once
#include "mockup_plugin_base.hpp"
#include <memory>
#include <functional>
#include <vector>

namespace ear {

class SceneMockup;
class MonitoringMockup;
class InputPluginMockup;

class MockupHost {
  typedef std::shared_ptr<MockupPlugin>(pluginapi_create_t)();

 public:
  MockupHost();
  std::shared_ptr<ear::SceneMockup> insertScenePlugin();
  std::shared_ptr<ear::MonitoringMockup> insertMonitoringPlugin();
  std::shared_ptr<ear::InputPluginMockup> insertInputPlugin();

 private:
  void loadPlugins();

  std::function<pluginapi_create_t> sceneMasterCreator_;
  std::function<pluginapi_create_t> inputPluginCreator_;
  std::function<pluginapi_create_t> monitoringCreator_;
  std::vector<std::shared_ptr<MockupPlugin>> plugins_;
};

}  // namespace ear
