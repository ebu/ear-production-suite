#include "scene_frontend_connector.hpp"

#include "speaker_setups.hpp"
#include "helper/move.hpp"
#include "components/overlay.hpp"
#include "helper/iso_lang_codes.hpp"
#include "element_view.hpp"

#include <numeric>
#include <memory>

namespace ear {
namespace plugin {
namespace ui {

JuceSceneFrontendConnector::JuceSceneFrontendConnector (
    SceneAudioProcessor* processor)
    : SceneFrontendBackendConnector(), p_(processor) {
  reloadProgrammeCache();
}

JuceSceneFrontendConnector::~JuceSceneFrontendConnector() {
  if (auto container = lockProgrammes()) {
    container->removeListeners(this);
  }
}

// --- Component Setter

void JuceSceneFrontendConnector::setItemsContainer(
    std::shared_ptr<ItemsContainer> container) {
  container->addListener(this);
  {
    std::lock_guard<std::mutex> itemViewLock(itemViewMutex_);
    itemsContainer_ = container;
  }
  reloadItemListCache();
}

void JuceSceneFrontendConnector::setItemsOverlay(
    std::shared_ptr<Overlay> overlay) {
  {
    std::lock_guard<std::mutex> itemViewLock(itemViewMutex_);
    itemsOverlay_ = overlay;
  }
}

void JuceSceneFrontendConnector::setAutoModeOverlay(
    std::shared_ptr<AutoModeOverlay> overlay) {
  {
    std::lock_guard<std::mutex> programmeStoreMutex(
        p_->getProgrammeStoreMutex());
    overlay->setVisible(p_->getProgrammeStore().autoModeEnabled());
  }
  overlay->addListener(this);
  autoModeOverlay_ = overlay;
}

void JuceSceneFrontendConnector::setProgrammesContainer(
    std::shared_ptr<ProgrammesContainer> container) {
  container->addTabListener(this);
  {
    std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
    programmesContainer_ = container;
  }
  reloadProgrammeCache();
}

void JuceSceneFrontendConnector::triggerProgrammeStoreChanged() {
  {
    std::lock_guard<std::mutex> programmeStoreMutex(
        p_->getProgrammeStoreMutex());
    notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
  }
  reloadProgrammeCache();
}

// --- Restore Editor


void JuceSceneFrontendConnector::reloadProgrammeCache() {
  {
    if (auto programmesContainer = lockProgrammes()) {
      programmesContainer->clear();
    }
  }
  int selectedProgramme = 0;
  bool autoMode = false;
  {
    std::lock_guard<std::mutex> programmeStoreLock(
        p_->getProgrammeStoreMutex());
    auto& programmeStore_ = p_->getProgrammeStore();
    programmeStore_.autoUpdateFrom(itemStore_);
    auto store = programmeStore_.get();
    selectedProgramme = store.selected_programme_index();
    selectedProgramme = selectedProgramme == -1 ? 0 : selectedProgramme;

    for (int i = 0; i < store.programme_size(); ++i) {
      auto programme = store.programme(i);
      addProgrammeView(programme);
      updateElementOverview(i);
      for (auto element : programme.element()) {
        if (element.has_object()) {
          addObjectView(i, element.object());
        }
      }
    }
  }
  if (auto overlay = autoModeOverlay_.lock()) {
    overlay->setVisible(autoMode);
  }
  // selectProgramme(selectedProgramme); - selectProgrammeView will trigger
  // selectProgramme anyway
  selectProgrammeView(selectedProgramme);
}

void JuceSceneFrontendConnector::reloadItemListCache() {
  if (auto container = itemsContainer_.lock()) {
    container->createOrUpdateViews(itemStore_.allItems());
  }
}

// --- ItemList Management

//void JuceSceneFrontendConnector::doAddItem(communication::ConnectionId id) {
//}

namespace {
  bool routingChanged(std::optional<proto::InputItemMetadata> const& previous,
                      proto::InputItemMetadata const& current) {
    return (previous && previous->routing() != current.routing());
  }
}

//void JuceSceneFrontendConnector::doUpdateItem(communication::ConnectionId id,
//                                              proto::InputItemMetadata item) {
//}

bool JuceSceneFrontendConnector::updateAndCheckPendingElements(
    const communication::ConnectionId& id,
    const proto::InputItemMetadata& item) const {
  bool storeChanged{false};
  auto& pendingElements = p_->getPendingElements();
  auto range = pendingElements.equal_range(item.routing());
  if (range.first != range.second) {
    for (auto el = range.first; el != range.second; ++el) {
      el->second->mutable_object()->set_connection_id(id.string());
    }
    pendingElements.erase(range.first, range.second);

    if (pendingElements.empty()) {
      p_->setStoreFromPending();
      storeChanged = true;
    }
  }
  return storeChanged;
}

void JuceSceneFrontendConnector::updateElementOverviews() {
  if (auto programmesContainer = lockProgrammes()) {
    for (int programmeIndex = 0;
         programmeIndex < programmesContainer->programmeCount();
         ++programmeIndex) {
      updateElementOverview(programmeIndex);
    }
  }
}

void JuceSceneFrontendConnector::updateItemView(communication::ConnectionId id,
                                                proto::InputItemMetadata item) {

  if (auto container = itemsContainer_.lock()) {
    container->updateView(id, item);
  }
}

void JuceSceneFrontendConnector::updateObjectViews(
    communication::ConnectionId id, proto::InputItemMetadata item) {
  if (auto programmesContainer = lockProgrammes()) {
    if(auto meterCalculator = p_->getLevelMeter().lock()) {
      programmesContainer->updateViews(item, meterCalculator);
    }
  }
}

//void JuceSceneFrontendConnector::doRemoveItem(communication::ConnectionId id) {
//}


void JuceSceneFrontendConnector::removeFromItemView(
    communication::ConnectionId id) {
  if (auto container = itemsContainer_.lock()) {
    container->removeView(id);
  }
}

void JuceSceneFrontendConnector::setMultipleScenePluginsOverlay(
    std::shared_ptr<MultipleScenePluginsOverlay> multipleScenePluginsOverlay) {
  multipleScenePluginsOverlay_ = multipleScenePluginsOverlay;
  doSetMultipleScenePluginsOverlayVisible(false);
}

void JuceSceneFrontendConnector::doSetMultipleScenePluginsOverlayVisible(
    const bool& visible) {
  auto multipleScenePluginsOverlay = multipleScenePluginsOverlay_;

  updater_.callOnMessageThread([visible, multipleScenePluginsOverlay]() {
    if (auto multipleScenePluginsOverlayLocked =
            multipleScenePluginsOverlay.lock()) {
      multipleScenePluginsOverlayLocked->setVisible(visible);
    }
  });
}

void JuceSceneFrontendConnector::removeFromProgrammes(
    communication::ConnectionId id) {
  auto changedProgIndices = p_->getProgrammeStore().removeFromAllProgrammes(id);
  if(!changedProgIndices.empty()) {
    notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
  }
  for(auto index : changedProgIndices) {
    updateElementOverview(index);
  }
}

void JuceSceneFrontendConnector::removeFromObjectViews(
    communication::ConnectionId id) {
  if (auto programmesContainer = lockProgrammes()) {
    programmesContainer->removeFromElementViews(id);
  }
}

// --- Programme Management

//proto::Programme* JuceSceneFrontendConnector::addProgramme() {
//  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
//  auto newProgramme = p_->getProgrammeStore()->add_programme();
//  newProgramme->set_name("");
//  newProgramme->set_language("");
//  notifyProgrammeStoreChanged(p_->getProgrammeStoreCopy());
//  return newProgramme;
//}

void JuceSceneFrontendConnector::addProgrammeView(
    const proto::Programme& programme) {
  if (auto container = lockProgrammes()) {
    container->addProgrammeView(programme, *this);
  }
}

void JuceSceneFrontendConnector::selectProgramme(int index) {
  p_->getProgrammeStore().selectProgramme(index);
  notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
}

void JuceSceneFrontendConnector::selectProgrammeView(int index) {
  if (auto container = lockProgrammes()) {
    container->selectTab(index);
  }
}

void JuceSceneFrontendConnector::moveProgramme(int oldIndex, int newIndex) {
  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
  if(p_->getProgrammeStore().moveProgramme(oldIndex, newIndex)) {
    notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
  }
}

void JuceSceneFrontendConnector::moveProgrammeView(int oldIndex, int newIndex) {
  if (auto container = lockProgrammes()) {
    container->moveProgrammeView(oldIndex, newIndex);
  }
}

void JuceSceneFrontendConnector::removeProgramme(int index) {
  p_->getProgrammeStore().removeProgramme(index);
  notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
}

void JuceSceneFrontendConnector::removeProgrammeView(int index) {
  if (auto container = lockProgrammes()) {
    container->removeProgrammeView(index);
  }
}

void JuceSceneFrontendConnector::setProgrammeName(int programmeIndex,
                                                  const std::string& newName) {
  p_->getProgrammeStore().setProgrammeName(programmeIndex, newName);
  notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
}

void JuceSceneFrontendConnector::setProgrammeViewName(int programmeIndex,
                                                      const String& newName) {
  if (auto container = lockProgrammes()) {
    container->setProgrammeViewName(programmeIndex, newName);
  }
}

void JuceSceneFrontendConnector::setProgrammeLanguage(int programmeIndex,
                                                      int languageIndex) {
  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
  if (languageIndex >= 0 && languageIndex < LANGUAGES.size()) {
    p_->getProgrammeStore().setProgrammeLanguage(languageIndex, LANGUAGES.at(languageIndex).alpha3);
  } else {
    p_->getProgrammeStore().clearProgrammeLanguage(languageIndex);
  }
  notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
}

int JuceSceneFrontendConnector::getProgrammeIndex(ProgrammeView* view) {
  if (auto container = lockProgrammes()) {
    return container->getProgrammeIndex(view);
  }
  return -1;
}


void JuceSceneFrontendConnector::addItemClicked(ProgrammeView* view) {
  std::lock_guard<std::mutex> itemViewLock(itemViewMutex_);
  auto overlay = itemsOverlay_.lock();
  auto itemsContainer = itemsContainer_.lock();
  if (overlay && itemsContainer) {
    overlay->setVisible(true);
    auto index = getProgrammeIndex(view);
    auto programme = p_->getProgrammeStore().programmeAtIndex(index);
    assert(programme);
    itemsContainer->themeItemsFor(*programme);
  }
}

void JuceSceneFrontendConnector::addGroupClicked(ProgrammeView* view) {
  // TODO: add group to ProgrammeStore and ProgrammeView
}

void JuceSceneFrontendConnector::addToggleClicked(ProgrammeView* view) {
  // TODO: add toggle to ProgrammeStore and ProgrammeView
}

void JuceSceneFrontendConnector::nameChanged(ProgrammeView* view,
                                             const String& newName) {
  auto index = getProgrammeIndex(view);
  if (index >= 0) {
    setProgrammeName(index, newName.toStdString());
    setProgrammeViewName(index, newName);
  }
}

void JuceSceneFrontendConnector::languageChanged(ProgrammeView* view,
                                                 int languageIndex) {
  auto index = getProgrammeIndex(view);
  if (index >= 0) {
    setProgrammeLanguage(index, languageIndex);
  }
}

void JuceSceneFrontendConnector::addTabClicked(
    EarTabbedComponent* tabbedComponent) {

  auto [programmeIndex, programme] = p_->getProgrammeStore().addProgramme();
  addProgrammeView(programme);
  setProgrammeViewName(programmeIndex, programme.name());
  selectProgramme(programmeIndex);
  selectProgrammeView(programmeIndex);
}

void JuceSceneFrontendConnector::tabSelected(EarTabbedComponent*, int index) {
  selectProgramme(index);
}

void JuceSceneFrontendConnector::tabMoved(EarTabbedComponent*, int oldIndex,
                                          int newIndex) {
  moveProgramme(oldIndex, newIndex);
  moveProgrammeView(oldIndex, newIndex);
  selectProgramme(newIndex);
}

void JuceSceneFrontendConnector::removeTabClicked(
    EarTabbedComponent* tabbedComponent, int index) {
  auto progCount = p_->getProgrammeStore().programmeCount();
  if (auto programmesContainer = lockProgrammes(); programmesContainer && progCount == 1) {
    NativeMessageBox::showMessageBox(MessageBoxIconType::NoIcon,
                                     String("Cannot delete last programme"),
                                     "The Scene must always have at least one programme.",
                                     programmesContainer.get());
    return;
  }
  String programmeName;

  programmeName = p_->getProgrammeStore().programmeName(index);
  auto text = String("Do you really want to delete \"");
  text += String(programmeName);
  text += String("\"?");
  if( auto programmesContainer = lockProgrammes()) {
    if (NativeMessageBox::showOkCancelBox(
          MessageBoxIconType::NoIcon,
          String("Delete Programme?"),
          text,
          programmesContainer.get(),
          nullptr)) {
      removeProgramme(index);
      removeProgrammeView(index);
    }
  }
}

void JuceSceneFrontendConnector::tabBarDoubleClicked(
    EarTabbedComponent* tabbedComponent) {
  addTabClicked(tabbedComponent);
}

// ItemsContainer::Listener
void JuceSceneFrontendConnector::addItemsClicked(
    ItemsContainer* container, std::vector<communication::ConnectionId> ids) {
  auto index{0};
  for(auto const& id : ids) {
    auto [programmeIndex, object]  = p_->getProgrammeStore().addItemToSelectedProgramme(id);
    addObjectView(programmeIndex, object);
    index = programmeIndex;
  }

  if (auto overlay = itemsOverlay_.lock()) {
    overlay->setVisible(false);
  }

  updateElementOverview(index);
}

void JuceSceneFrontendConnector::updateElementOverview(int programmeIndex) {
  if (auto programmesContainer = lockProgrammes()) {

    auto programme = p_->getProgrammeStore().programmeAtIndex(programmeIndex);
    assert(programme);
    programmesContainer->updateElementOverview(programmeIndex, *programme);
  }
}

// --- Programme Element Management

proto::Object* JuceSceneFrontendConnector::addObject(
    proto::Programme* programme, const communication::ConnectionId id) {
  auto element = programme->add_element();
  auto object = new proto::Object{};
  object->set_connection_id(id.string());
  element->set_allocated_object(object);
  notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
  return object;
}

proto::Group* JuceSceneFrontendConnector::addGroup(
    proto::Programme* programme) {
  throw std::runtime_error(
      "[scene_frontend_connector] adding group not implemented");
  return {};
}

proto::Toggle* JuceSceneFrontendConnector::addToggle(
    proto::Programme* programme) {
  throw std::runtime_error(
      "[scene_frontend_connector] adding toggle not implemented");
  return {};
}

void JuceSceneFrontendConnector::addObjectView(int programmeIndex,
                                               const proto::Object& object) {
  auto meterCalculator = p_->getLevelMeter().lock();
  auto programmesContainer = lockProgrammes();
  if (meterCalculator && programmesContainer) {
    auto id = communication::ConnectionId{object.connection_id()};
    auto item = itemStore_.get(id);
    auto view = programmesContainer->addObjectView(programmeIndex,
                                                   item,
                                                   object,
                                                   meterCalculator);
    view->addListener(this);
  }
}

void JuceSceneFrontendConnector::addGroupView(int programmeIndex,
                                              const proto::Group& group) {
  throw std::runtime_error("Group view not implemented");
}

void JuceSceneFrontendConnector::addToggleView(int programmeIndex,
                                               const proto::Toggle& toggle) {
  throw std::runtime_error("Toggle view not implemented");
}

void JuceSceneFrontendConnector::moveElement(int programmeIndex, int oldIndex,
                                             int newIndex) {
  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
  if(p_->getProgrammeStore().moveElement(programmeIndex, oldIndex, newIndex)) {
    notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
  }
}

void JuceSceneFrontendConnector::removeElement(int programmeIndex,
                                               int elementIndex) {
  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
  p_->getProgrammeStore().removeElement(programmeIndex, elementIndex);
  notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
}

// ElementsContainer::Listener
void JuceSceneFrontendConnector::elementMoved(ElementViewList* list,
                                              int oldIndex, int newIndex) {
  int programmeIndex = -1;
  {
    if (auto programmesContainer = lockProgrammes()) {
      programmeIndex = programmesContainer->getProgrammeIndex(list);
    }
  }
  assert(programmeIndex >= 0);
  moveElement(programmeIndex, oldIndex, newIndex);
}

void JuceSceneFrontendConnector::removeElementClicked(ElementViewList* list,
                                                      int index) {
  int programmeIndex = -1;
  {
    if (auto programmesContainer = lockProgrammes()) {
      programmeIndex = programmesContainer->getProgrammeIndex(list);
    }
  }
  assert(programmeIndex >= 0);
  removeElement(programmeIndex, index);
  updateElementOverview(programmeIndex);
}

// AutoModeOverlay::Listener
void JuceSceneFrontendConnector::autoModeChanged(AutoModeOverlay* overlay,
                                                 bool state) {
  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
  p_->getProgrammeStore().setAutoMode(state);
}

// ObjectView::Listener
void JuceSceneFrontendConnector::objectDataChanged(ObjectView::Data data) {
  auto id = communication::ConnectionId(data.item.connection_id());
  if(p_->getProgrammeStore().updateElement(id, data.object)) {
    notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
  }
}

ItemStore& JuceSceneFrontendConnector::doGetItemStore() {
  return itemStore_;
}

void JuceSceneFrontendConnector::addItem(const proto::InputItemMetadata& item) {
  auto const& id = item.connection_id();
  itemStore_.addItem(id);
  {
    std::lock_guard<std::mutex> programmeStoreLock(
        p_->getProgrammeStoreMutex());
    if (p_->getProgrammeStore().autoModeEnabled()) {
      auto result = p_->getProgrammeStore().addItemToSelectedProgramme(id);
      addObjectView(result.first, result.second);
      notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
    }
  }
}

void JuceSceneFrontendConnector::changeItem(proto::InputItemMetadata const& oldItem,
                                            proto::InputItemMetadata const& newItem) {
  auto id = newItem.connection_id();
  auto previousItem = itemStore_.setItem(id, newItem);
  auto storeChanged = updateAndCheckPendingElements(id, newItem);
  auto autoMode = p_->getProgrammeStore().autoModeEnabled();
  if (storeChanged || (routingChanged(previousItem, newItem) && autoMode)) {
    triggerProgrammeStoreChanged();
  }
  updateElementOverviews();
  updateItemView(id, newItem);
  updateObjectViews(id, newItem);
}

void JuceSceneFrontendConnector::removeItem(proto::InputItemMetadata const& oldItem) {
  auto const& id = oldItem.connection_id();
  itemStore_.removeItem(id);
  removeFromProgrammes(id);
  removeFromObjectViews(id);
  removeFromItemView(id);
  updateElementOverviews();
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
