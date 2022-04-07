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
    public SceneFrontendBackendConnector,
    private ItemsContainer::Listener,
    private AutoModeOverlay::Listener {
 public:
  explicit JuceSceneFrontendConnector(SceneAudioProcessor* processor);

  void repopulateUIComponents(
      std::shared_ptr<ItemsContainer> const& itemsContainer,
      std::shared_ptr<AutoModeOverlay> const& autoModeOverlay,
      std::shared_ptr<MultipleScenePluginsOverlay> const& multipleScenePluginsOverlay);


  // MetadataListener
  void dataReset (proto::ProgrammeStore const& programmes,
                  ItemMap const& items) override;
  void duplicateSceneDetected(bool isDuplicate) override;
  void programmeSelected(ProgrammeObjects const& objects) override;
  void programmeItemUpdated(ProgrammeStatus status, ProgrammeObject const& item) override;
  void itemsAddedToProgramme(ProgrammeStatus status, std::vector<ProgrammeObject> const& items) override;
  void itemRemovedFromProgramme(ProgrammeStatus status, communication::ConnectionId const& id) override;
  void inputRemoved(communication::ConnectionId const& id) override;
  void autoModeChanged(bool enabled) override;
  void inputAdded(const InputItem& item) override;
  void inputUpdated(const InputItem& item) override;

 private:
  void removeFromItemView(communication::ConnectionId id);
  void updateAddItemsContainer(ProgrammeObjects const& objects);
  void updateAndCheckPendingElements(const communication::ConnectionId& id,
                                     const proto::InputItemMetadata& item) const;

  // ItemsContainer::Listener
  void addItemsClicked(ItemsContainer* container,
                       std::vector<communication::ConnectionId> ids) override;

  // AutoModeOverlay::Listener
  void autoModeChanged(AutoModeOverlay* overlay, bool state) override;

  SceneAudioProcessor* p_;
  Metadata& data_;

  std::weak_ptr<MultipleScenePluginsOverlay> multipleScenePluginsOverlay_;
  std::weak_ptr<Overlay> itemsOverlay_;
  std::weak_ptr<AutoModeOverlay> autoModeOverlay_;
  std::weak_ptr<ItemsContainer> itemsContainer_;
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
