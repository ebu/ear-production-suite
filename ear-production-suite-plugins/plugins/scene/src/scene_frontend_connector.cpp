#include "scene_frontend_connector.hpp"
#include "components/overlay.hpp"
#include "object_view.hpp"
#include <memory>

namespace ear {
namespace plugin {
namespace ui {

JuceSceneFrontendConnector::JuceSceneFrontendConnector (
    SceneAudioProcessor* processor)
    : SceneFrontendBackendConnector(), p_(processor)
      {}

// --- ItemList Management

}  // namespace ui
}  // namespace plugin
}  // namespace ear
