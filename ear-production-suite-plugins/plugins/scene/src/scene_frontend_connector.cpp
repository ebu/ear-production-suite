#include "scene_frontend_connector.hpp"

#include "speaker_setups.hpp"
#include "helper/move.hpp"
#include "components/overlay.hpp"
#include "helper/iso_lang_codes.hpp"

#include <numeric>

namespace ear {
namespace plugin {
namespace ui {

JuceSceneFrontendConnector::JuceSceneFrontendConnector(
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
    overlay->setVisible(p_->getProgrammeStore()->auto_mode());
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
    notifyProgrammeStoreChanged(p_->getProgrammeStoreCopy());
  }
  reloadProgrammeCache();
}

// --- Restore Editor

inline bool isItemInElement(communication::ConnectionId id,
                            const proto::ProgrammeElement& element) {
  if (element.has_object()) {
    return communication::ConnectionId{element.object().connection_id()} == id;
  } else if (element.has_toggle()) {
    for (auto toggleElement : element.toggle().element()) {
      if (isItemInElement(id, toggleElement)) {
        return true;
      }
    }
  } else if (element.has_group()) {
    for (auto groupElement : element.group().element()) {
      if (isItemInElement(id, groupElement)) {
        return true;
      }
    }
  }
  return false;
}
inline bool isItemInProgramme(communication::ConnectionId id,
                              const proto::Programme& programme) {
  for (auto element : programme.element()) {
    if (isItemInElement(id, element)) {
      return true;
    }
  }
  return false;
}

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
    const auto programmeStore_ = p_->getProgrammeStore();
    if (programmeStore_->auto_mode()) {
      autoMode = true;
      programmeStore_->clear_programme();
      auto defaultProgramme = programmeStore_->add_programme();
      defaultProgramme->set_name("Default");
      defaultProgramme->set_language("");
      programmeStore_->set_selected_programme_index(0);
      if (programmeStore_->auto_mode()) {
        std::lock_guard<std::mutex> itemStoreLock(itemStoreMutex_);
        std::multimap<int, communication::ConnectionId> items;
        std::transform(itemStore_.cbegin(), itemStore_.cend(),
                       std::inserter(items, items.begin()),
                       [](auto const& idItemPair) {
                         return std::make_pair(idItemPair.second.routing(),
                                               idItemPair.first);
                       });
        for (auto const& item : items) {
          addObject(defaultProgramme, item.second);
        }
      }
    }
    selectedProgramme = programmeStore_->selected_programme_index();
    selectedProgramme = selectedProgramme == -1 ? 0 : selectedProgramme;

    for (int i = 0; i < programmeStore_->programme_size(); ++i) {
      auto programme = programmeStore_->programme(i);
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
  std::lock_guard<std::mutex> itemViewLock(itemViewMutex_);
  if (auto container = itemsContainer_.lock()) {
    std::lock_guard<std::mutex> itemStoreLock(itemStoreMutex_);
    for (auto entry : itemStore_) {
      auto id = entry.first;
      auto item = entry.second;
      if (item.has_ds_metadata()) {
        auto it = std::find_if(
            container->directSpeakersItems.begin(),
            container->directSpeakersItems.end(), [item](auto entry) {
              return communication::ConnectionId{item.connection_id()} ==
                     entry->getId();
            });
        if (it == container->directSpeakersItems.end()) {
          auto view = std::make_shared<ItemView>();
          view->setMetadata(item);
          container->directSpeakersItems.push_back(view);
          container->directSpeakersList->addItem(view.get());
        } else {
          (*it)->setMetadata(item);
        }
      } else if (item.has_obj_metadata()) {
        auto it = std::find_if(
            container->objectsItems.begin(), container->objectsItems.end(),
            [item](auto entry) {
              return communication::ConnectionId{item.connection_id()} ==
                     entry->getId();
            });
        if (it == container->objectsItems.end()) {
          auto view = std::make_shared<ItemView>();
          view->setMetadata(item);
          container->objectsItems.push_back(view);
          container->objectsList->addItem(view.get());
        } else {
          (*it)->setMetadata(item);
        }
      } else if (item.has_hoa_metadata()) {
        auto it = std::find_if(
            container->hoaItems.begin(), container->hoaItems.end(),
            [item](auto entry) {
              return communication::ConnectionId{item.connection_id()} ==
                     entry->getId();
            });
        if (it == container->hoaItems.end()) {
          auto view = std::make_shared<ItemView>();
          view->setMetadata(item);
          container->hoaItems.push_back(view);
          container->hoaList->addItem(view.get());
        } else {
          (*it)->setMetadata(item);
        }
      }
    }
  }
}

// --- ItemList Management

void JuceSceneFrontendConnector::doAddItem(communication::ConnectionId id) {
  updater_.callOnMessageThread([this, id]() {
    {
      std::lock_guard<std::mutex> itemStoreLock(itemStoreMutex_);
      proto::InputItemMetadata emptyItemMetadata;
      emptyItemMetadata.set_connection_id(id.string());
      itemStore_[id] = emptyItemMetadata;
    }
    {
      std::lock_guard<std::mutex> programmeStoreLock(
          p_->getProgrammeStoreMutex());
      if (p_->getProgrammeStore()->auto_mode()) {
        auto programmeIndex =
            p_->getProgrammeStore()->selected_programme_index();
        auto programme =
            p_->getProgrammeStore()->mutable_programme(programmeIndex);
        auto object = addObject(programme, id);
        addObjectView(programmeIndex, *object);
        notifyProgrammeStoreChanged(p_->getProgrammeStoreCopy());
      }
    }
  });
}

void JuceSceneFrontendConnector::doUpdateItem(communication::ConnectionId id,
                                              proto::InputItemMetadata item) {
  updater_.callOnMessageThread([this, id, item]() {
    bool channelsChanged{false};
    {
      std::lock_guard<std::mutex> lock(itemStoreMutex_);
      auto currentItemPos = itemStore_.find(id);
      if (currentItemPos != itemStore_.end() &&
          currentItemPos->second.routing() != item.routing()) {
        channelsChanged = true;
      }

      itemStore_[id] = item;
    }

    bool storeChanged{false};
    bool autoMode{false};
    {
      std::lock_guard<std::mutex> programmeStoreMutex(
          p_->getProgrammeStoreMutex());
      storeChanged = updateAndCheckPendingStore(id, item);
      autoMode = p_->getProgrammeStore()->auto_mode();
    }
    if (storeChanged || (channelsChanged && autoMode)) {
      triggerProgrammeStoreChanged();
    }
    updateElementOverviews();
    updateItemView(id, item);
    updateObjectViews(id, item);
  });
}

bool JuceSceneFrontendConnector::updateAndCheckPendingStore(
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
  std::lock_guard<std::mutex> itemStoreLock(itemStoreMutex_);
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

void JuceSceneFrontendConnector::doRemoveItem(communication::ConnectionId id) {
  updater_.callOnMessageThread([this, id]() {
    {
      std::lock_guard<std::mutex> lock(itemStoreMutex_);
      itemStore_.erase(id);
    }
    removeFromProgrammes(id);
    removeFromObjectViews(id);
    removeFromItemView(id);
    updateElementOverviews();
  });
}

void JuceSceneFrontendConnector::removeFromItemView(
    communication::ConnectionId id) {
  std::lock_guard<std::mutex> itemViewLock(itemViewMutex_);
  if (auto container = itemsContainer_.lock()) {
    auto it_ds =
        std::find_if(container->directSpeakersItems.begin(),
                     container->directSpeakersItems.end(),
                     [id](auto entry) { return id == entry->getId(); });
    if (it_ds != container->directSpeakersItems.end()) {
      container->directSpeakersList->removeItem(it_ds->get());
      container->directSpeakersItems.erase(it_ds);
    }
    auto it_obj = std::find_if(
        container->objectsItems.begin(), container->objectsItems.end(),
        [id](auto entry) { return id == entry->getId(); });
    if (it_obj != container->objectsItems.end()) {
      container->objectsList->removeItem(it_obj->get());
      container->objectsItems.erase(it_obj);
    }
    auto it_hoa =
        std::find_if(container->hoaItems.begin(), container->hoaItems.end(),
                     [id](auto entry) { return id == entry->getId(); });
    if (it_hoa != container->hoaItems.end()) {
      container->hoaList->removeItem(it_hoa->get());
      container->hoaItems.erase(it_hoa);
    }
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
  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
  auto* progStore = p_->getProgrammeStore();
  for (int programmeIndex = 0; programmeIndex < progStore->programme_size();
       ++programmeIndex) {
    auto elements =
        progStore->mutable_programme(programmeIndex)->mutable_element();
    auto element =
        std::find_if(elements->begin(), elements->end(), [id](auto entry) {
          return communication::ConnectionId(entry.object().connection_id()) ==
                 id;
        });
    if (element != elements->end()) {
      auto elementIndex = std::distance(elements->begin(), element);
      elements->erase(elements->begin() + elementIndex);
      notifyProgrammeStoreChanged(p_->getProgrammeStoreCopy());

      std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
      updateElementOverview(programmeIndex);
    }
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

proto::Programme* JuceSceneFrontendConnector::addProgramme() {
  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
  auto newProgramme = p_->getProgrammeStore()->add_programme();
  newProgramme->set_name("");
  newProgramme->set_language("");
  notifyProgrammeStoreChanged(p_->getProgrammeStoreCopy());
  return newProgramme;
}

void JuceSceneFrontendConnector::addProgrammeView(
    const proto::Programme& programme) {
  std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
  if (auto container = programmesContainer_.lock()) {
    auto view = std::make_shared<ProgrammeView>();
    view->getNameTextEditor()->setText(programme.name(), false);
    view->getLanguageComboBox()->selectEntry(
        getIndexForAlphaN(programme.language()), dontSendNotification);
    view->addListener(this);
    view->getElementsContainer()->addListener(this);
    view->getElementOverview()->setProgramme(programme, itemStore_);
    container->programmes.push_back(view);
    container->tabs->addTab(programme.name(), view.get(), false);
  }
}

void JuceSceneFrontendConnector::selectProgramme(int index) {
  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
  p_->getProgrammeStore()->set_selected_programme_index(index);
  notifyProgrammeStoreChanged(p_->getProgrammeStoreCopy());
}

void JuceSceneFrontendConnector::selectProgrammeView(int index) {
  std::lock_guard<std::mutex> programmeViewsLock(programmeViewsMutex_);
  if (auto container = programmesContainer_.lock()) {
    container->tabs->selectTab(index);
  }
}

void JuceSceneFrontendConnector::moveProgramme(int oldIndex, int newIndex) {
  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
  auto programmes = p_->getProgrammeStore()->mutable_programme();
  auto size = programmes->size();
  if (oldIndex >= 0 && newIndex >= 0 && oldIndex < size && newIndex < size &&
      oldIndex != newIndex) {
    move(programmes->begin(), oldIndex, newIndex);
    notifyProgrammeStoreChanged(p_->getProgrammeStoreCopy());
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
  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
  auto programme = p_->getProgrammeStore()->mutable_programme();
  programme->erase(programme->begin() + index);
  auto selected_index = p_->getProgrammeStore()->selected_programme_index();
  if(selected_index >= programme->size()) {
    p_->getProgrammeStore()->set_selected_programme_index(std::max<int>(programme->size() - 1, 0));
  }
  notifyProgrammeStoreChanged(p_->getProgrammeStoreCopy());
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
  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
  p_->getProgrammeStore()->mutable_programme(programmeIndex)->set_name(newName);
  notifyProgrammeStoreChanged(p_->getProgrammeStoreCopy());
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
    p_->getProgrammeStore()
        ->mutable_programme(programmeIndex)
        ->set_language(LANGUAGES.at(languageIndex).alpha3);
  } else {
    p_->getProgrammeStore()
        ->mutable_programme(programmeIndex)
        ->clear_language();
  }
  notifyProgrammeStoreChanged(p_->getProgrammeStoreCopy());
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

void JuceSceneFrontendConnector::addItemClicked(ProgrammeView* view) {
  std::lock_guard<std::mutex> itemViewLock(itemViewMutex_);
  auto overlay = itemsOverlay_.lock();
  auto itemsContainer = itemsContainer_.lock();
  if (overlay && itemsContainer) {
    overlay->setVisible(true);
    auto index = getProgrammeIndex(view);
    auto programme = p_->getProgrammeStore()->programme(index);
    for (auto item : itemsContainer->directSpeakersItems) {
      if (isItemInProgramme(item->getId(), programme)) {
        item->setEnabled(false);
        item->setAlpha(Emphasis::disabled);
        item->setSelected(true);
      } else {
        item->setEnabled(true);
        item->setAlpha(Emphasis::full);
        item->setSelected(false);
      }
    }
    for (auto item : itemsContainer->objectsItems) {
      if (isItemInProgramme(item->getId(), programme)) {
        item->setEnabled(false);
        item->setAlpha(Emphasis::disabled);
        item->setSelected(true);
      } else {
        item->setEnabled(true);
        item->setAlpha(Emphasis::full);
        item->setSelected(false);
      }
    }
    for (auto item : itemsContainer->hoaItems) {
      if (isItemInProgramme(item->getId(), programme)) {
        item->setEnabled(false);
        item->setAlpha(Emphasis::disabled);
        item->setSelected(true);
      } else {
        item->setEnabled(true);
        item->setAlpha(Emphasis::full);
        item->setSelected(false);
      }
    }
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
  auto programme = addProgramme();
  addProgrammeView(*programme);
  String name("Programme_");
  name += String(p_->getProgrammeStore()->programme().size());
  setProgrammeName(p_->getProgrammeStore()->programme().size() - 1,
                   name.toStdString());
  setProgrammeViewName(p_->getProgrammeStore()->programme_size() - 1, name);
  selectProgramme(p_->getProgrammeStore()->programme().size() - 1);
  selectProgrammeView(p_->getProgrammeStore()->programme().size() - 1);
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
  int progCount = 0;
  {
    std::lock_guard<std::mutex> programmeStoreLock(
        p_->getProgrammeStoreMutex());
    progCount = p_->getProgrammeStore()->programme_size();
  }
  if (progCount == 1) {
    AlertWindow::showNativeDialogBox(
        String("Cannot delete last programme"),
        "The Scene must always have at least one programme.", false);
    return;
  }
  String programmeName;
  {
    std::lock_guard<std::mutex> programmeStoreLock(
        p_->getProgrammeStoreMutex());
    programmeName = p_->getProgrammeStore()->programme(index).name();
  }
  auto text = String("Do you really want to delete \"");
  text += String(programmeName);
  text += String("\"?");
  if (AlertWindow::showNativeDialogBox(String("Delete Programme?"), text,
                                       true)) {
    removeProgramme(index);
    removeProgrammeView(index);
  }
}

void JuceSceneFrontendConnector::tabBarDoubleClicked(
    EarTabbedComponent* tabbedComponent) {
  addTabClicked(tabbedComponent);
}

// ItemsContainer::Listener
void JuceSceneFrontendConnector::addItemsClicked(
    ItemsContainer* container, std::vector<communication::ConnectionId> ids) {
  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
  auto programmeIndex = p_->getProgrammeStore()->selected_programme_index();
  auto programme = p_->getProgrammeStore()->mutable_programme(programmeIndex);
  for (auto id : ids) {
    auto object = addObject(programme, id);
    addObjectView(programmeIndex, *object);
  }

  if (auto overlay = itemsOverlay_.lock()) {
    overlay->setVisible(false);
  }
  updateElementOverview(programmeIndex);
}

void JuceSceneFrontendConnector::updateElementOverview(int programmeIndex) {
  if (auto programmesContainer = programmesContainer_.lock()) {
    auto overview = programmesContainer->programmes.at(programmeIndex)
                        ->getElementOverview();
    overview->setProgramme(p_->getProgrammeStore()->programme(programmeIndex),
                           itemStore_);
  }
}

// --- Programme Element Management

proto::Object* JuceSceneFrontendConnector::addObject(
    proto::Programme* programme, const communication::ConnectionId id) {
  auto element = programme->add_element();
  auto object = new proto::Object{};
  object->set_connection_id(id.string());
  element->set_allocated_object(object);
  notifyProgrammeStoreChanged(p_->getProgrammeStoreCopy());
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

    std::lock_guard<std::mutex> itemStoreLock(itemStoreMutex_);
    auto item = itemStore_[id];
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
  auto elements = p_->getProgrammeStore()
                      ->mutable_programme(programmeIndex)
                      ->mutable_element();
  if (oldIndex >= 0 &&  //
      newIndex >= 0 &&  //
      oldIndex < elements->size() &&  //
      newIndex < elements->size() &&  //
      oldIndex != newIndex) {
    move(elements->begin(), oldIndex, newIndex);
    notifyProgrammeStoreChanged(p_->getProgrammeStoreCopy());
  }
}

void JuceSceneFrontendConnector::removeElement(int programmeIndex,
                                               int elementIndex) {
  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
  auto elements = p_->getProgrammeStore()
                      ->mutable_programme(programmeIndex)
                      ->mutable_element();
  elements->erase(elements->begin() + elementIndex);
  notifyProgrammeStoreChanged(p_->getProgrammeStoreCopy());
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
  p_->getProgrammeStore()->set_auto_mode(state);
}

// ObjectView::Listener
void JuceSceneFrontendConnector::objectDataChanged(ObjectView::Data data) {
  std::lock_guard<std::mutex> programmeStoreLock(p_->getProgrammeStoreMutex());
  auto programmeIndex = p_->getProgrammeStore()->selected_programme_index();
  auto elements = p_->getProgrammeStore()
                      ->mutable_programme(programmeIndex)
                      ->mutable_element();

  auto it =
      std::find_if(elements->begin(), elements->end(), [data](auto element) {
        if (element.has_object() &&
            element.object().connection_id() == data.item.connection_id()) {
          return true;
        } else {
          return false;
        }
      });

  if (it != elements->end()) {
    it->mutable_object()->Swap(&data.object);
    notifyProgrammeStoreChanged(p_->getProgrammeStoreCopy());
  }
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
