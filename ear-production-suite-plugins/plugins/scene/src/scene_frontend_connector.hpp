#include "ui/scene_frontend_backend_connector.hpp"

#include "JuceHeader.h"

#include "helper/iso_lang_codes.hpp"
#include "auto_mode_overlay.hpp"
#include "multiple_scene_plugins_overlay.hpp"
#include "item_store.hpp"
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
    public EarTabbedComponent::Listener,
    public ProgrammeView::Listener,
    public ElementsContainer::Listener,
    private ItemsContainer::Listener,
    private AutoModeOverlay::Listener,
    private ObjectView::Listener {
 public:
  explicit JuceSceneFrontendConnector(SceneAudioProcessor* processor);
  ~JuceSceneFrontendConnector() override;

  void itemsAddedToProgramme(ProgrammeStatus status, std::vector<ProgrammeObject> const& items) override;
  void repopulateUIComponents(
      std::shared_ptr<ItemsContainer> const& itemsContainer,
      std::shared_ptr<Overlay> const& addItemsOverlay,
      std::shared_ptr<AutoModeOverlay> const& autoModeOverlay,
      std::shared_ptr<ProgrammesContainer> const& programmesContainer,
      std::shared_ptr<MultipleScenePluginsOverlay> const& multipleScenePluginsOverlay
      );

  void doSetMultipleScenePluginsOverlayVisible(const bool& visible) override;

  // MetadataListener
  void dataReset (proto::ProgrammeStore const& programmes,
                  ItemMap const& items) override;
  void programmeAdded(int programmeIndex, proto::Programme const& programme) override;
  void programmeRemoved(int programmeIndex) override;
  void programmeSelected(ProgrammeObjects const& objects) override;
  void programmeUpdated(int programmeIndex, proto::Programme const& programme) override;
  void programmeMoved(Movement motion, proto::Programme const& programme) override;
  void programmeItemUpdated(ProgrammeStatus status, ProgrammeObject const& item) override;
  void itemRemovedFromProgramme(ProgrammeStatus status, communication::ConnectionId const& id) override;
  void inputRemoved(communication::ConnectionId const& id) override;
  void autoModeChanged(bool enabled) override;
  void inputAdded(const InputItem& item) override;
  void inputUpdated(const InputItem& item) override;

 protected:
  void updateObjectViews(communication::ConnectionId id,
                         proto::InputItemMetadata item);
  void removeFromObjectViews(communication::ConnectionId id);
  void removeFromItemView(communication::ConnectionId id);

 private:
  std::shared_ptr<ProgrammesContainer> lockProgrammes() {
    std::lock_guard<std::mutex> lock(programmeViewsMutex_);
    return programmesContainer_.lock();
  }

  void updateAddItemsContainer(ProgrammeObjects const& objects);

  // --- Restore Editor
  void reloadItemListCache();

  // --- Programme Management
  void addProgrammeView(const proto::Programme& programme);
  void selectProgramme(int index);
  void selectProgrammeView(int index);
  void moveProgrammeView(int oldIndex, int newIndex);
  void removeProgrammeView(int index);
  void setProgrammeViewName(int programmeIndex, const String& newName);
  void setProgrammeLanguage(int programmeIndex, int languageIndex);
  int getProgrammeIndex(ProgrammeView* view);

  // --- Programme Element Management

  proto::Group* addGroup(proto::Programme* programme);
  proto::Toggle* addToggle(proto::Programme* programme);
  void addObjectView(ProgrammeStatus status, ProgrammeObject const& item);
  void addGroupView(int programmeIndex, const proto::Group& group);
  void addToggleView(int programmeIndex, const proto::Toggle& toggle);

  // --- ElementOverview
  void updateElementOverview(ProgrammeObjects const& objects);

  // ProgrammeView::Listener
  void addItemClicked(ProgrammeView* view) override;
  void addGroupClicked(ProgrammeView* view) override;
  void addToggleClicked(ProgrammeView* view) override;
  void nameChanged(ProgrammeView* view, const String& newName) override;
  void languageChanged(ProgrammeView* view, int index) override;

  // ItemsContainer::Listener
  void addItemsClicked(ItemsContainer* container,
                       std::vector<communication::ConnectionId> ids) override;

  // EarTabbedComponent::Listener
  void addTabClicked(EarTabbedComponent* tabbedComponent) override;
  void tabSelected(EarTabbedComponent* tabbedComponent, int index) override;
  void tabMoved(EarTabbedComponent* tabbedComponent, int oldIndex,
                int newIndex) override;
  void removeTabClicked(EarTabbedComponent* tabbedComponent,
                        int index) override;
  void tabBarDoubleClicked(EarTabbedComponent* tabbedComponent) override;

  // ElementViewList::Listener
  void elementMoved(ElementViewList* list, int oldIndex, int newIndex) override;
  void removeElementClicked(ElementViewList* list, ElementView* view) override;

  // AutoModeOverlay::Listener
  void autoModeChanged(AutoModeOverlay* overlay, bool state) override;

  // ObjectView::Listener
  void objectDataChanged(ObjectView::Data data) override;

  SceneAudioProcessor* p_;
  Metadata& data_;

  std::mutex itemViewMutex_;
  std::mutex programmeViewsMutex_;

  MultiAsyncUpdater updater_;

  std::weak_ptr<MultipleScenePluginsOverlay> multipleScenePluginsOverlay_;
  std::weak_ptr<Overlay> itemsOverlay_;
  std::weak_ptr<AutoModeOverlay> autoModeOverlay_;
  std::weak_ptr<ItemsContainer> itemsContainer_;
  std::weak_ptr<ProgrammesContainer> programmesContainer_;

  bool updateAndCheckPendingElements(const communication::ConnectionId& id,
                                  const proto::InputItemMetadata& item) const;
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
