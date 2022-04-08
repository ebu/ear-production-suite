#include "ui/scene_frontend_backend_connector.hpp"

#include "JuceHeader.h"

#include "helper/iso_lang_codes.hpp"
#include "auto_mode_overlay.hpp"
#include "multiple_scene_plugins_overlay.hpp"
#include "items_container.hpp"
#include "object_view.hpp"
#include "programmes_container.hpp"
#include "scene_plugin_processor.hpp"
#include "helper/multi_async_updater.h"
#include "element_view.hpp"
#include "programme_view.hpp"
#include "store_metadata.hpp"

#include <optional>
#include <memory>
#include <map>

namespace ear {
namespace plugin {
namespace ui {
class EarTabbedComponent;
class ProgrammeView;
class Overlay;
class ObjectView;

class JuceSceneFrontendConnector :
    public MetadataListener,
    public SceneFrontendBackendConnector {
 public:
  explicit JuceSceneFrontendConnector(SceneAudioProcessor* processor);
  SceneAudioProcessor* p_;
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
