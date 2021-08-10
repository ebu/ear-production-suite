#include "mockup_scene.hpp"
#include <boost/dll/alias.hpp>
#include <memory>

namespace ear {

std::shared_ptr<SceneMockup> createScenePlugin() {
  return std::make_shared<SceneMockup>();
}

BOOST_DLL_ALIAS(ear::createScenePlugin, createPlugin);

}  // namespace ear
