#include "mockup_input_plugin.hpp"
#include <boost/dll/alias.hpp>
#include <memory>

namespace ear {

std::shared_ptr<InputPluginMockup> createInputPlugin() {
  return std::make_shared<InputPluginMockup>();
}

BOOST_DLL_ALIAS(ear::createInputPlugin, createPlugin);

}  // namespace ear
