#include "scene_frontend_connector.hpp"

#include "speaker_setups.hpp"
#include "helper/move.hpp"
#include "components/overlay.hpp"
#include "helper/iso_lang_codes.hpp"

#include <numeric>

namespace ear {
namespace plugin {
namespace ui {

JuceSceneFrontendConnector::JuceSceneFrontendConnector (
    SceneAudioProcessor* processor)
    : SceneFrontendBackendConnector(), p_(processor) {
  reloadProgrammeCache();
}

JuceSceneFrontendConnector::~JuceSceneFrontendConnector() {
  if (auto container = programmesContainer_.lock()) {
    container->tabs->removeListener(this);
    for (auto view : container->programmes) {
      view->removeListener(this);
    }
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
  container->tabs->addListener(this);
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
    std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
    if (auto programmesContainer = programmesContainer_.lock()) {
      programmesContainer->tabs->removeAllTabs();
      programmesContainer->programmes.clear();
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

namespace {
communication::ConnectionId getId(proto::InputItemMetadata const& item) {
  return {item.connection_id()};
}

std::shared_ptr<ItemView> itemViewFor(communication::ConnectionId const& id,
                                      std::vector<std::shared_ptr<ItemView>> const& views) {
  std::shared_ptr<ItemView> itemView;
  auto it = std::find_if(views.cbegin(),
                         views.cend(),
                         [&id](auto const& item) {
                           return item->getId() == id;
                         });
  if(it != views.cend()) {
    itemView = *it;
  }
  return itemView;
}

void createOrUpdateView(proto::InputItemMetadata const& item,
                        std::vector<std::shared_ptr<ItemView>>& views,
                        ItemViewList& viewList) {

  auto view = itemViewFor(getId(item), views);
  if (view) {
    view->setMetadata(item);
  } else {
    view = std::make_shared<ItemView>();
    view->setMetadata(item);
    views.push_back(view);
    viewList.addItem(view.get());
  }
}
}

void JuceSceneFrontendConnector::reloadItemListCache() {
  std::lock_guard<std::mutex> itemViewLock(itemViewMutex_);
  auto items = itemStore_.allItems();
  if (auto container = itemsContainer_.lock()) {
    for (auto const& entry : items) {
      auto item = entry.second;
      if (item.has_ds_metadata()) {
        createOrUpdateView(item,
                           container->directSpeakersItems,
                           *container->directSpeakersList);
      } else if (item.has_obj_metadata()) {
        createOrUpdateView(item,
                           container->objectsItems,
                           *container->objectsList);
      } else if (item.has_hoa_metadata()) {
        createOrUpdateView(item,
                           container->hoaItems,
                           *container->hoaList);
      }
    }
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
  if (auto programmesContainer = programmesContainer_.lock()) {
    for (int programmeIndex = 0;
         programmeIndex < programmesContainer->programmes.size();
         ++programmeIndex) {
      updateElementOverview(programmeIndex);
    }
  }
}

void JuceSceneFrontendConnector::updateItemView(communication::ConnectionId id,
                                                proto::InputItemMetadata item) {
  std::lock_guard<std::mutex> itemViewLock(itemViewMutex_);

  if (auto container = itemsContainer_.lock()) {
    if (item.has_ds_metadata()) {
      auto it = std::find_if(container->directSpeakersItems.begin(),
                             container->directSpeakersItems.end(),
                             [id](auto entry) { return id == entry->getId(); });
      if (it != container->directSpeakersItems.end()) {
        (*it)->setMetadata(item);
      } else {
        auto view = std::make_shared<ItemView>();
        view->setMetadata(item);
        container->directSpeakersItems.push_back(view);
        container->directSpeakersList->addItem(view.get());
      }
    } else if (item.has_obj_metadata()) {
      auto it = std::find_if(container->objectsItems.begin(),
                             container->objectsItems.end(),
                             [id](auto entry) { return id == entry->getId(); });
      if (it != container->objectsItems.end()) {
        (*it)->setMetadata(item);
      } else {
        auto view = std::make_shared<ItemView>();
        view->setMetadata(item);
        container->objectsItems.push_back(view);
        container->objectsList->addItem(view.get());
      }
    } else if (item.has_hoa_metadata()) {
      auto it =
          std::find_if(container->hoaItems.begin(), container->hoaItems.end(),
                       [id](auto entry) { return id == entry->getId(); });
      if (it != container->hoaItems.end()) {
        (*it)->setMetadata(item);
      } else {
        auto view = std::make_shared<ItemView>();
        view->setMetadata(item);
        container->hoaItems.push_back(view);
        container->hoaList->addItem(view.get());
      }
    }
  }
}

void JuceSceneFrontendConnector::updateObjectViews(
    communication::ConnectionId id, proto::InputItemMetadata item) {
  std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
  if (auto programmesContainer = programmesContainer_.lock()) {
    for (auto programme : programmesContainer->programmes) {
      auto elements = programme->getElementsContainer()->elements;
      auto it = std::find_if(
          elements.begin(), elements.end(), [id](auto elementView) {
            if (auto objectView =
                    std::dynamic_pointer_cast<ObjectView>(elementView)) {
              return communication::ConnectionId(
                         objectView->getData().object.connection_id()) == id;
            }
            return false;
          });
      if (it != elements.end()) {
        auto view = std::dynamic_pointer_cast<ObjectView>(*it);
        int numberOfChannels = 1;
        if (item.has_ds_metadata()) {
          auto layoutIndex = item.ds_metadata().layout();
          if (layoutIndex >= 0 && layoutIndex < SPEAKER_SETUPS.size()) {
            numberOfChannels = SPEAKER_SETUPS.at(layoutIndex).speakers.size();
          }
        }
        auto data = view->getData();
        data.item = item;
        data.object.set_connection_id(item.connection_id());
        view->setData(data);
        std::vector<int> routing(numberOfChannels);
        std::iota(routing.begin(), routing.end(), item.routing());
        view->getLevelMeter()->setMeter(p_->getLevelMeter(), routing);
      }
    }
  }
}

//void JuceSceneFrontendConnector::doRemoveItem(communication::ConnectionId id) {
//}

namespace {
void removeItemFromViews(communication::ConnectionId const& id,
                         std::vector<std::shared_ptr<ItemView>>& views,
                         ItemViewList& viewList) {
  auto it = std::find_if(views.begin(), views.end(), [&id](auto const& view) {
    return id == view->getId();
  });
  if (it != views.end()) {
    viewList.removeItem(it->get());
    views.erase(it);
  }
}
}

void JuceSceneFrontendConnector::removeFromItemView(
    communication::ConnectionId id) {
  std::lock_guard<std::mutex> itemViewLock(itemViewMutex_);
  if (auto container = itemsContainer_.lock()) {
    removeItemFromViews(id, container->directSpeakersItems, *container->directSpeakersList);
    removeItemFromViews(id, container->objectsItems, *container->objectsList);
    removeItemFromViews(id, container->hoaItems, *container->hoaList);
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
  std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
  for(auto index : changedProgIndices) {
    updateElementOverview(index);
  }
}

void JuceSceneFrontendConnector::removeFromObjectViews(
    communication::ConnectionId id) {
  std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
  if (auto programmesContainer = programmesContainer_.lock()) {
    for (int programmeIndex = 0;
         programmeIndex < programmesContainer->programmes.size();
         ++programmeIndex) {
      auto container = programmesContainer->programmes.at(programmeIndex)
                           ->getElementsContainer();
      auto& elements = container->elements;
      auto it = std::find_if(
          elements.begin(), elements.end(), [id](auto elementView) {
            if (auto objectView =
                    std::dynamic_pointer_cast<ObjectView>(elementView)) {
              return communication::ConnectionId(
                         objectView->getData().object.connection_id()) == id;
            }
            return false;
          });
      if (it != container->elements.end()) {
        auto index = std::distance(container->elements.begin(), it);
        container->removeElement(index);
      }
    }
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
  std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
  if (auto container = programmesContainer_.lock()) {
    auto view = std::make_shared<ProgrammeView>(this);
    view->getNameTextEditor()->setText(programme.name(), false);
    view->getLanguageComboBox()->selectEntry(
        getIndexForAlphaN(programme.language()), dontSendNotification);
    view->addListener(this);
    view->getElementsContainer()->addListener(this);
    view->getElementOverview()->setProgramme(programme);
    container->programmes.push_back(view);
    container->tabs->addTab(programme.name(), view.get(), false);
  }
}

void JuceSceneFrontendConnector::selectProgramme(int index) {
  p_->getProgrammeStore().selectProgramme(index);
  notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
}

void JuceSceneFrontendConnector::selectProgrammeView(int index) {
  std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
  if (auto container = programmesContainer_.lock()) {
    container->tabs->selectTab(index);
  }
}

void JuceSceneFrontendConnector::moveProgramme(int oldIndex, int newIndex) {
  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
  if(p_->getProgrammeStore().moveProgramme(oldIndex, newIndex)) {
    notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
  }
}

void JuceSceneFrontendConnector::moveProgrammeView(int oldIndex, int newIndex) {
  std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
  if (auto container = programmesContainer_.lock()) {
    auto size = container->programmes.size();
    if (oldIndex >= 0 && newIndex >= 0 && oldIndex < size && newIndex < size &&
        oldIndex != newIndex) {
      move(container->programmes.begin(), oldIndex, newIndex);
    }
  }
}

void JuceSceneFrontendConnector::removeProgramme(int index) {
  p_->getProgrammeStore().removeProgramme(index);
  notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
}

void JuceSceneFrontendConnector::removeProgrammeView(int index) {
  std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
  if (auto container = programmesContainer_.lock()) {
    container->tabs->removeTab(index);
    container->programmes.erase(container->programmes.begin() + index);
  }
}

void JuceSceneFrontendConnector::setProgrammeName(int programmeIndex,
                                                  const std::string& newName) {
  p_->getProgrammeStore().setProgrammeName(programmeIndex, newName);
  notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
}

void JuceSceneFrontendConnector::setProgrammeViewName(int programmeIndex,
                                                      const String& newName) {
  std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
  if (auto container = programmesContainer_.lock()) {
    container->tabs->setTabName(programmeIndex, newName);
    container->programmes.at(programmeIndex)
        ->getNameTextEditor()
        ->setText(newName);
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
  std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
  if (auto container = programmesContainer_.lock()) {
    auto it =
        std::find_if(container->programmes.begin(), container->programmes.end(),
                     [view](auto entry) { return view == entry.get(); });
    if (it != container->programmes.end()) {
      return std::distance(container->programmes.begin(), it);
    }
  }
  return -1;
}

namespace {
void setPresentTheme(ItemView& view) {
  view.setEnabled(false);
  view.setAlpha(Emphasis::disabled);
  view.setSelected(true);
}

void setMissingTheme(ItemView& view) {
  view.setEnabled(true);
  view.setAlpha(Emphasis::full);
  view.setSelected(false);
}

void setItemTheme(ItemView& view, proto::Programme const& programme) {
  if (isItemInProgramme(view.getId(), programme)) {
    setPresentTheme(view);
  } else {
    setMissingTheme(view);
  }
}

void themeItemsFor(std::vector<std::shared_ptr<ItemView>> const& items,
                   proto::Programme const& programme) {
  for (auto const& item : items) {
    setItemTheme(*item, programme);
  }
}
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
    themeItemsFor(itemsContainer->directSpeakersItems, *programme);
    themeItemsFor(itemsContainer->objectsItems, *programme);
    themeItemsFor(itemsContainer->hoaItems, *programme);
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

  auto [programmeIndex, name] = p_->getProgrammeStore().addProgramme();
  setProgrammeViewName(programmeIndex, name);
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
  if (auto programmesContainer = programmesContainer_.lock(); programmesContainer && progCount == 1) {
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
  if( auto programmesContainer = programmesContainer_.lock()) {
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
  if (auto programmesContainer = programmesContainer_.lock()) {
    auto overview = programmesContainer->programmes.at(programmeIndex)
                        ->getElementOverview();
    auto programme = p_->getProgrammeStore().programmeAtIndex(programmeIndex);
    assert(programme);
    overview->setProgramme(*programme);
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
  std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
  auto objectType = ObjectView::ObjectType::Other;
  if (auto programmesContainer = programmesContainer_.lock()) {
    auto id = communication::ConnectionId{object.connection_id()};
    auto container = programmesContainer->programmes.at(programmeIndex)
                         ->getElementsContainer();

    auto item = itemStore_.get(id);
    String speakerSetup;
    int numberOfChannels = 1;
    if (item.has_obj_metadata()) {
      objectType = ObjectView::ObjectType::Object;
    }
    if (item.has_ds_metadata()) {
      objectType = ObjectView::ObjectType::DirectSpeakers;
      auto layoutIndex = item.ds_metadata().layout();
      if (layoutIndex >= 0) {
        speakerSetup = SPEAKER_SETUPS.at(layoutIndex).commonName;
        numberOfChannels = SPEAKER_SETUPS.at(layoutIndex).speakers.size();
      }
    }
    if (item.has_hoa_metadata()) {
      objectType = ObjectView::ObjectType::HOA;
      auto packFormatId = item.hoa_metadata().packformatidvalue();
      if (packFormatId >= 0) {
        auto commonDefinitionHelper = AdmCommonDefinitionHelper::getSingleton();
        auto elementRelationships =
            commonDefinitionHelper->getElementRelationships();
        auto pfData =
            commonDefinitionHelper->getPackFormatData(4, packFormatId);
        size_t cfCount(0);
        if (pfData) {
          numberOfChannels =
              static_cast<size_t>(pfData->relatedChannelFormats.size());
        }
      }
    }

    auto view = std::make_shared<ObjectView>(objectType);
    view->addListener(this);
    view->setData({item, object});

    std::vector<int> routing(numberOfChannels);
    std::iota(routing.begin(), routing.end(), item.routing());
    view->getLevelMeter()->setMeter(p_->getLevelMeter(), routing);
    container->addElement(view);
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
    std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
    if (auto programmesContainer = programmesContainer_.lock()) {
      auto it = std::find_if(
          programmesContainer->programmes.begin(),
          programmesContainer->programmes.end(), [list](auto entry) {
            return entry->getElementsContainer()->list.get() == list;
          });
      programmeIndex =
          std::distance(programmesContainer->programmes.begin(), it);
    }
  }
  assert(programmeIndex >= 0);
  moveElement(programmeIndex, oldIndex, newIndex);
}

void JuceSceneFrontendConnector::removeElementClicked(ElementViewList* list,
                                                      int index) {
  int programmeIndex = -1;
  {
    std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
    if (auto programmesContainer = programmesContainer_.lock()) {
      auto it = std::find_if(
          programmesContainer->programmes.begin(),
          programmesContainer->programmes.end(), [list](auto entry) {
            return entry->getElementsContainer()->list.get() == list;
          });
      programmeIndex =
          std::distance(programmesContainer->programmes.begin(), it);
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
  auto id = getId(data.item);
  if(p_->getProgrammeStore().updateElement(id, data.object)) {
    notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
  }
}

ItemStore& JuceSceneFrontendConnector::doGetItemStore() {
  return itemStore_;
}

void JuceSceneFrontendConnector::addItem(const proto::InputItemMetadata& item) {
  updater_.callOnMessageThread([this, item]() {
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
  });
}

void JuceSceneFrontendConnector::changeItem(proto::InputItemMetadata const& oldItem,
                                            proto::InputItemMetadata const& newItem) {
  auto id = newItem.connection_id();
  updater_.callOnMessageThread([this, id, newItem]() {
    auto previousItem = itemStore_.setItem(id, newItem);
    auto storeChanged = updateAndCheckPendingElements(id, newItem);
    auto autoMode = p_->getProgrammeStore().autoModeEnabled();
    if (storeChanged || (routingChanged(previousItem, newItem) && autoMode)) {
      triggerProgrammeStoreChanged();
    }
    updateElementOverviews();
    updateItemView(id, newItem);
    updateObjectViews(id, newItem);
  });
}

void JuceSceneFrontendConnector::removeItem(proto::InputItemMetadata const& oldItem) {
  auto const& id = oldItem.connection_id();
  updater_.callOnMessageThread([this, id]() {
    itemStore_.removeItem(id);
    removeFromProgrammes(id);
    removeFromObjectViews(id);
    removeFromItemView(id);
    updateElementOverviews();
  });

}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
