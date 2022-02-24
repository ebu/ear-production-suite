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

#include <optional>
#include <memory>
#include <map>

namespace ear {
namespace plugin {
namespace ui {
class InputItemList;
class EarTabbedComponent;
class ProgrammeView;
class Overlay;
class ObjectView;
class GroupView;
class ToggleView;

class JuceSceneFrontendConnector :
    public ItemStore::Listener,
    public SceneFrontendBackendConnector,
                                   private ProgrammeView::Listener,
                                   private ElementsContainer::Listener,
                                   private ItemsContainer::Listener,
                                   private AutoModeOverlay::Listener,
                                   private EarTabbedComponent::Listener,
                                   private ObjectView::Listener {
 public:
  JuceSceneFrontendConnector(SceneAudioProcessor* processor);
  ~JuceSceneFrontendConnector();

  void setItemsContainer(std::shared_ptr<ItemsContainer> container);
  void setProgrammesContainer(std::shared_ptr<ProgrammesContainer> container);
  void setItemsOverlay(std::shared_ptr<Overlay> overlay);
  void setAutoModeOverlay(std::shared_ptr<AutoModeOverlay> overlay);

  void triggerProgrammeStoreChanged();
  void setMultipleScenePluginsOverlay(
      std::shared_ptr<MultipleScenePluginsOverlay> multipleScenePluginsOverlay);

 protected:
//  void doAddItem(communication::ConnectionId id) override;
//  void doRemoveItem(communication::ConnectionId id) override;
//  void doUpdateItem(communication::ConnectionId id,
//                    proto::InputItemMetadata item) override;
  void updateElementOverviews();
  void updateItemView(communication::ConnectionId id,
                      proto::InputItemMetadata item);
  void updateObjectViews(communication::ConnectionId id,
                         proto::InputItemMetadata item);
  void removeFromProgrammes(communication::ConnectionId id);
  void removeFromObjectViews(communication::ConnectionId id);
  void removeFromItemView(communication::ConnectionId id);
  void doSetMultipleScenePluginsOverlayVisible(const bool& visible) override;
  ItemStore& doGetItemStore() override;
  // ItemStore listener
  void addItem(const proto::InputItemMetadata& item) override;
  void changeItem(proto::InputItemMetadata const& oldItem,
                  proto::InputItemMetadata const& newItem) override;
  void removeItem(proto::InputItemMetadata const& oldItem) override;

 private:
  // --- Restore Editor
  void reloadItemListCache();
  void reloadProgrammeCache();

  // --- Programme Management
//  proto::Programme* addProgramme();
  void addProgrammeView(const proto::Programme& programme);
  void selectProgramme(int index);
  void selectProgrammeView(int index);
  void moveProgramme(int oldIndex, int newIndex);
  void moveProgrammeView(int oldIndex, int newIndex);
  void removeProgramme(int index);
  void removeProgrammeView(int index);
  void setProgrammeName(int programmeIndex, const std::string& newName);
  void setProgrammeViewName(int programmeIndex, const String& newName);
  void setProgrammeLanguage(int programmeIndex, int languageIndex);
  int getProgrammeIndex(ProgrammeView* view);

  // --- Programme Element Management

  proto::Object* addObject(proto::Programme* programme,
                           const communication::ConnectionId id);
  proto::Group* addGroup(proto::Programme* programme);
  proto::Toggle* addToggle(proto::Programme* programme);
  void addObjectView(int programmeIndex, const proto::Object& object);
  void addGroupView(int programmeIndex, const proto::Group& group);
  void addToggleView(int programmeIndex, const proto::Toggle& toggle);
  void moveElement(int programmeIndex, int oldIndex, int newIndex);
  void removeElement(int programmeIndex, int elmentIndex);

  // --- ElementOverview
  void updateElementOverview(int programmeIndex);

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
  void removeElementClicked(ElementViewList* list, int index) override;

  // AutoModeOverlay::Listener
  void autoModeChanged(AutoModeOverlay* overlay, bool state) override;

  // ObjectView::Listener
  void objectDataChanged(ObjectView::Data data) override;

  SceneAudioProcessor* p_;

  std::mutex itemStoreMutex_;
  ItemStore itemStore_;

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
