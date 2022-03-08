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
    : SceneFrontendBackendConnector(), p_(processor),
      data_{processor->getData()}{

}

JuceSceneFrontendConnector::~JuceSceneFrontendConnector() {
  if (auto container = lockProgrammes()) {
    container->removeListeners(this);
  }
}

// --- Component Setter

void JuceSceneFrontendConnector::repopulateUIComponents(
    std::shared_ptr<ItemsContainer> const& itemsContainer,
    std::shared_ptr<Overlay> const& addItemsOverlay,
    std::shared_ptr<AutoModeOverlay> const& autoModeOverlay,
    std::shared_ptr<ProgrammesContainer> const& programmesContainer,
    std::shared_ptr<MultipleScenePluginsOverlay> const& multipleScenePluginsOverlay
) {
  itemsContainer->addListener(this);
  autoModeOverlay->addListener(this);
  programmesContainer->addTabListener(this);
  itemsContainer_ = itemsContainer;
  itemsOverlay_ = addItemsOverlay;
  autoModeOverlay_ = autoModeOverlay;
  programmesContainer_ = programmesContainer;
  multipleScenePluginsOverlay_ = multipleScenePluginsOverlay;
  multipleScenePluginsOverlay->setVisible(false);
  data_.refreshUI();
}

//TODO call on message thread
void JuceSceneFrontendConnector::doSetMultipleScenePluginsOverlayVisible(const bool& visible) {
  if(auto overlay = multipleScenePluginsOverlay_.lock()) {
    overlay->setVisible(visible);
  }

}

void JuceSceneFrontendConnector::triggerProgrammeStoreChanged() {
//  {
//    std::lock_guard<std::mutex> programmeStoreMutex(
//        p_->getProgrammeStoreMutex());
//    notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
//  }
//  reloadProgrammeCache();
}

// --- Restore Editor


void JuceSceneFrontendConnector::dataReset(
    proto::ProgrammeStore const& programmeStore,
    ItemMap const& items) {
  {
    if (auto programmesContainer = lockProgrammes()) {
      programmesContainer->clear();
    }
  }
  bool autoMode = false;
  auto selectedProgramme = programmeStore.selected_programme_index();
  selectedProgramme = std::max<int>(selectedProgramme, 0);


  for (int i = 0; i < programmeStore.programme_size(); ++i) {
    auto const& programme = programmeStore.programme(i);
    ProgrammeObjects programmeObjects({i, i == selectedProgramme},
                                      programme,
                                      items);
    addProgrammeView(programme);
    updateElementOverview(programmeObjects);
    for (auto const& element : programme.element()) {
      if (element.has_object()) {
        auto const& object = element.object();
        auto id = communication::ConnectionId{object.connection_id()};
        ProgrammeStatus status {i, i == selectedProgramme};
        addObjectView(status, {element.object(), items.at(id)});
      }
    }
  }

  if (auto overlay = autoModeOverlay_.lock()) {
    overlay->setVisible(programmeStore.auto_mode());
  }
  selectProgrammeView(selectedProgramme);

  if (auto container = itemsContainer_.lock()) {
    container->createOrUpdateViews(items);
  }
}


void JuceSceneFrontendConnector::reloadItemListCache() {
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

//void JuceSceneFrontendConnector::updateElementOverviews() {
//  if (auto programmesContainer = lockProgrammes()) {
//    for (int programmeIndex = 0;
//         programmeIndex < programmesContainer->programmeCount();
//         ++programmeIndex) {
//      updateElementOverview(programmeIndex);
//    }
//  }
//}

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



void JuceSceneFrontendConnector::programmeUpdated(int programmeIndex,
                                                  proto::Programme const& programme) {
//  updateElementOverview(programmeIndex, programme);
  setProgrammeViewName(programmeIndex, programme.name());
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
  data_.withProgrammeStore([index](auto& store){
    store.selectProgramme(index);
  });
}

void JuceSceneFrontendConnector::selectProgrammeView(int index) {
  if (auto container = lockProgrammes()) {
    container->selectTab(index);
  }
}

void JuceSceneFrontendConnector::moveProgrammeView(int oldIndex, int newIndex) {
  if (auto container = lockProgrammes()) {
    container->moveProgrammeView(oldIndex, newIndex);
  }
}

void JuceSceneFrontendConnector::removeProgrammeView(int index) {
  if (auto container = lockProgrammes()) {
    container->removeProgrammeView(index);
  }
}

void JuceSceneFrontendConnector::setProgrammeViewName(int programmeIndex,
                                                      const String& newName) {
  if (auto container = lockProgrammes()) {
    container->setProgrammeViewName(programmeIndex, newName);
  }
}

void JuceSceneFrontendConnector::setProgrammeLanguage(int programmeIndex,
                                                      int languageIndex) {
  if (languageIndex >= 0 && languageIndex < LANGUAGES.size()) {
    auto language = LANGUAGES.at(languageIndex).alpha3;
    data_.withProgrammeStore([programmeIndex, &language](auto& store) {
      store.setProgrammeLanguage(programmeIndex, language);
    });
  } else {
    data_.withProgrammeStore([programmeIndex](auto& store) {
      store.clearProgrammeLanguage(programmeIndex);
    });
  }
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
//    auto index = getProgrammeIndex(view);
//    auto programme = p_->getProgrammeStore().programmeAtIndex(index);
//    assert(programme);
//    itemsContainer->themeItemsFor(*programme);
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
    data_.withProgrammeStore([index, &newName](auto& store){
      store.setProgrammeName(index, newName.toStdString());
    });
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

  data_.withProgrammeStore([](auto& store){
    store.addProgramme();
  });
}

void JuceSceneFrontendConnector::programmeAdded(
    int programmeIndex,
    proto::Programme const& programme) {
  addProgrammeView(programme);
  setProgrammeViewName(programmeIndex, programme.name());
  selectProgramme(programmeIndex);
}

void JuceSceneFrontendConnector::programmeSelected(ProgrammeObjects const& objects) {

  selectProgrammeView(objects.index());
  updateAddItemsContainer(objects);
}

void JuceSceneFrontendConnector::updateAddItemsContainer(ProgrammeObjects const& objects) {
  auto overlay = itemsOverlay_.lock();
  auto itemsContainer = itemsContainer_.lock();
  if (overlay && itemsContainer) {
    itemsContainer->themeItemsFor(objects);
  }
}

void JuceSceneFrontendConnector::tabSelected(EarTabbedComponent*, int index) {
  selectProgramme(index);
}

void JuceSceneFrontendConnector::tabMoved(EarTabbedComponent*, int oldIndex,
                                          int newIndex) {
  data_.withProgrammeStore([oldIndex, newIndex](auto& store) {
    store.moveProgramme(oldIndex, newIndex);
  });
}

void JuceSceneFrontendConnector::programmeMoved(Movement motion,
                                                const proto::Programme& programme) {
  moveProgrammeView(motion.to, motion.from);
}

void JuceSceneFrontendConnector::removeTabClicked(
    EarTabbedComponent* tabbedComponent, int index) {
  auto progCount = tabbedComponent->tabCount();
  if (auto programmesContainer = lockProgrammes(); programmesContainer && progCount == 1) {
    NativeMessageBox::showMessageBox(MessageBoxIconType::NoIcon,
                                     String("Cannot delete last programme"),
                                     "The Scene must always have at least one programme.",
                                     programmesContainer.get());
    return;
  }
  auto programmeName = tabbedComponent->getName();
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
      data_.withProgrammeStore([index](auto& store) {
        store.removeProgramme(index);
      });
    }
  }
}

void JuceSceneFrontendConnector::programmeRemoved(int programmeIndex) {
  removeProgrammeView(programmeIndex);
}

void JuceSceneFrontendConnector::tabBarDoubleClicked(
    EarTabbedComponent* tabbedComponent) {
  addTabClicked(tabbedComponent);
}

// ItemsContainer::Listener
void JuceSceneFrontendConnector::addItemsClicked(
    ItemsContainer* container, std::vector<communication::ConnectionId> ids) {
  data_.withProgrammeStore([&ids](auto& store) {
    store.addItemsToSelectedProgramme(ids);
  });

  if (auto overlay = itemsOverlay_.lock()) {
    overlay->setVisible(false);
  }
}

void JuceSceneFrontendConnector::itemsAddedToProgramme(ProgrammeStatus status, std::vector<ProgrammeObject> const& items) {
  for(auto const& item : items) {
    addObjectView(status, item);
  }
  if (auto programmesContainer = lockProgrammes()) {
    programmesContainer->itemsAddedToProgramme(status, items);
  }
//  updateElementOverview(items.back().programmeElement.index);
}

void JuceSceneFrontendConnector::itemRemovedFromProgramme(ProgrammeStatus status, ProgrammeObject const& item) {
  if (auto programmesContainer = lockProgrammes()) {
    programmesContainer->itemRemovedFromProgramme(status, item);
  }
}

void JuceSceneFrontendConnector::programmeItemUpdated(ProgrammeStatus status, ProgrammeObject const& item) {
  if (auto programmesContainer = lockProgrammes()) {
    programmesContainer->programmeItemUpdated(status, item);
  }
}

void JuceSceneFrontendConnector::updateElementOverview(ProgrammeObjects const& objects) {
  if (auto programmesContainer = lockProgrammes()) {
    programmesContainer->updateElementOverview(objects);
  }
}

// --- Programme Element Management

//proto::Object* JuceSceneFrontendConnector::addObject(
//    proto::Programme* programme, const communication::ConnectionId id) {
//  auto element = programme->add_element();
//  auto object = new proto::Object{};
//  object->set_connection_id(id.string());
//  element->set_allocated_object(object);
//  notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
//  return object;
//}

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

//void JuceSceneFrontendConnector::addObjectView(int programmeIndex,
//                                               const proto::Object& object) {
//  auto meterCalculator = p_->getLevelMeter().lock();
//  auto programmesContainer = lockProgrammes();
//  if (meterCalculator && programmesContainer) {
//    auto id = communication::ConnectionId{object.connection_id()};
//    auto item = itemStore_.getItem(id);
//    auto view = programmesContainer->addObjectView(programmeIndex,
//                                                   item,
//                                                   object,
//                                                   meterCalculator);
//    view->addListener(this);
//  }
//}

void JuceSceneFrontendConnector::addObjectView(ProgrammeStatus status, ProgrammeObject const& item) {
  auto meterCalculator = p_->getLevelMeter().lock();
  auto programmesContainer = lockProgrammes();
  if (meterCalculator && programmesContainer) {
    auto view = programmesContainer->addObjectView(status.index,
                                                   item.inputMetadata,
                                                   item.programmeObject,
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

//void JuceSceneFrontendConnector::moveElement(int programmeIndex, int oldIndex,
//                                             int newIndex) {
//  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
//  if(p_->getProgrammeStore().moveElement(programmeIndex, oldIndex, newIndex)) {
//    notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
//  }
//}

//void JuceSceneFrontendConnector::removeElement(int programmeIndex,
//                                               int elementIndex) {
//  data_.withProgrammeStore([programmeIndex, elementIndex](auto& store) {
//    store.removeElementFromSelectedProgramme(elementIndex);
//  });
//  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
//  p_->getProgrammeStore().removeElement(programmeIndex, elementIndex);
//  notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
//}

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
  data_.withProgrammeStore([programmeIndex, oldIndex, newIndex](auto& store){
    store.moveElement(programmeIndex, oldIndex, newIndex);
  });
}

void JuceSceneFrontendConnector::removeElementClicked(ElementViewList* list,
                                                      int index) {
  int programmeIndex = -1;
  {
    ProgrammesContainer c;
    if (auto programmesContainer = lockProgrammes()) {
      programmeIndex = programmesContainer->getProgrammeIndex(list);
    }
  }
  assert(programmeIndex >= 0);
  data_.withProgrammeStore([programmeIndex, index](auto& store){
    store.removeElementFromProgramme(programmeIndex, index);
  });
}

// AutoModeOverlay::Listener
void JuceSceneFrontendConnector::autoModeChanged(AutoModeOverlay* overlay,
                                                 bool state) {
  data_.withProgrammeStore([state](auto& store) {
    store.setAutoMode(state);
  });
}

// ObjectView::Listener
void JuceSceneFrontendConnector::objectDataChanged(ObjectView::Data data) {
  data_.withProgrammeStore([&data](auto& store) {
      store.updateElement(data.item.connection_id(), data.object);
  });

  data_.withItemStore([&data](auto& store) {
    store.setItem(data.item.connection_id(), data.item);
  });
}

//void JuceSceneFrontendConnector::addItem(const proto::InputItemMetadata& item) {
//  data_.withItemStore([&item](auto& store) {
//    store.setItem(item.connection_id(), item);
//  });
////  {
////    std::lock_guard<std::mutex> programmeStoreLock(
////        p_->getProgrammeStoreMutex());
////    if (p_->getProgrammeStore().autoModeEnabled()) {
////      auto result = p_->getProgrammeStore().addItemsToSelectedProgramme({id});
////      addObjectView(result.first, result.second);
////      notifyProgrammeStoreChanged(p_->getProgrammeStore().get());
////    }
////  }
//}

//void JuceSceneFrontendConnector::changeItem(proto::InputItemMetadata const& oldItem,
//                                            proto::InputItemMetadata const& newItem) {
//  auto id = newItem.connection_id();
//  auto previousItem = itemStore_.setItem(id, newItem);
//  auto storeChanged = updateAndCheckPendingElements(id, newItem);
//  auto autoMode = p_->getProgrammeStore().autoModeEnabled();
//  if (storeChanged || (routingChanged(previousItem, newItem) && autoMode)) {
//    triggerProgrammeStoreChanged();
//  }
////  updateElementOverviews();
//  updateItemView(id, newItem);
//  updateObjectViews(id, newItem);
//}

void JuceSceneFrontendConnector::inputRemoved(communication::ConnectionId const& id) {
  removeFromObjectViews(id);
  removeFromItemView(id);
}
void JuceSceneFrontendConnector::autoModeChanged(bool enabled) {
  if(auto overlay = autoModeOverlay_.lock()) {
    overlay->setVisible(enabled);
  }
}
void JuceSceneFrontendConnector::inputAdded(const InputItem& item) {

}
void JuceSceneFrontendConnector::inputUpdated(const InputItem& item) {

}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
