//
// Created by Richard Bailey on 28/02/2022.
//
#include "programmes_container.hpp"
#include "element_view_list.hpp"
#include "object_view.hpp"
#include "programme_view.hpp"
#include "helper/move.hpp"
#include "components/overlay.hpp"

using namespace ear::plugin::ui;
using namespace ear::plugin;

ProgrammesContainer::ProgrammesContainer(
        std::shared_ptr<LevelMeterCalculator> meterCalculator,
                                         Metadata& metadata)
    : tabs_(std::make_shared<EarTabbedComponent>()),
      meterCalculator_(std::move(meterCalculator)),
      data_{metadata} {
    tabs_->addListener(this);
  addAndMakeVisible(tabs_.get());
}

void ear::plugin::ui::ProgrammesContainer::resized() {
  tabs_->setBounds(getLocalBounds());
}

void ProgrammesContainer::addObjectView(
    int programmeIndex, const proto::InputItemMetadata &inputItem,
    const proto::Object &programmeElement) {

  auto objectType = ObjectView::ObjectType::Other;

  int numberOfChannels = 1;

  if (inputItem.has_obj_metadata()) {
    objectType = ObjectView::ObjectType::Object;
  }

  if (inputItem.has_ds_metadata()) {
    objectType = ObjectView::ObjectType::DirectSpeakers;
    auto layoutIndex = inputItem.ds_metadata().layout();
    if (layoutIndex >= 0) {
      numberOfChannels = static_cast<int>(SPEAKER_SETUPS.at(layoutIndex).speakers.size());
    }
  }
  if (inputItem.has_hoa_metadata()) {
    objectType = ObjectView::ObjectType::HOA;
    auto packFormatId = inputItem.hoa_metadata().packformatidvalue();
    if (packFormatId >= 0) {
      auto commonDefinitionHelper = AdmCommonDefinitionHelper::getSingleton();
      auto elementRelationships =
          commonDefinitionHelper->getElementRelationships();
      auto pfData =
          commonDefinitionHelper->getPackFormatData(4, packFormatId);
      if (pfData) {
        numberOfChannels =
            static_cast<int>(pfData->relatedChannelFormats.size());
      }
    }
  }

  auto view = std::make_shared<ObjectView>(objectType);
  view->setData({inputItem, programmeElement});

  std::vector<int> routing(numberOfChannels);
  std::iota(routing.begin(), routing.end(), inputItem.routing());
  view->getLevelMeter()->setMeter(meterCalculator_, routing);
  auto container = programmes_.at(programmeIndex)
        ->getElementsContainer();
  container->addElement(view);
  view->addListener(this);
}

void ear::plugin::ui::ProgrammesContainer::addProgrammeView(
    const proto::Programme &programme) {

  auto view = std::make_shared<ProgrammeView>();
  view->getNameTextEditor()->setText(programme.name(), false);
  view->getLanguageComboBox()->selectEntry(
      getIndexForAlphaN(programme.language()), dontSendNotification);
  view->addListener(this);
  view->getElementsContainer()->addListener(this);
  view->getElementOverview()->setProgramme(programme);
  programmes_.push_back(view);
  tabs_->addTab(programme.name(), view.get(), false);
}

void ProgrammesContainer::clear() {
  tabs_->removeAllTabs();
  programmes_.clear();
}

void ProgrammesContainer::itemsAddedToProgramme(
    ProgrammeStatus status, std::vector<ProgrammeObject> const& items) {
  assert(!items.empty());
  auto programmeIndex = status.index;
  for(auto& item : items) {
      addObjectView(programmeIndex, item.inputMetadata, item.programmeObject);
  }
  programmes_.at(programmeIndex)->getElementOverview()->itemsAdded(items);
}

void ProgrammesContainer::itemRemovedFromProgramme(ProgrammeStatus status, communication::ConnectionId const& id) {
  auto programmeIndex = status.index;
  auto overview = programmes_.at(programmeIndex)->getElementOverview();
  overview->itemRemoved(id);
}

void ProgrammesContainer::inputRemoved(communication::ConnectionId const& id) {
    removeFromElementViews(id);
}

void ProgrammesContainer::programmeItemUpdated(ProgrammeStatus status, ProgrammeObject const& item) {
  auto programmeIndex = status.index;
  auto overview = programmes_.at(programmeIndex)->getElementOverview();
  overview->itemChanged(item);
  auto container = programmes_.at(programmeIndex)->getElementsContainer();
  auto view = container->getObjectView(item.inputMetadata.connection_id());
  if(view) {
    view->setInputItemMetadata(item.inputMetadata);
    view->getLevelMeter()->setMeter(meterCalculator_, item.inputMetadata.routing());
  }
}

void ProgrammesContainer::programmeMoved(Movement motion,
                                         const proto::Programme& programme) {
    moveProgrammeView(motion.to, motion.from);
}

void ProgrammesContainer::programmeSelected(const ProgrammeObjects &objects) {
    tabs_->selectTab(objects.index());
}

void ProgrammesContainer::programmeRemoved(int programmeIndex) {
    tabs_->removeTab(programmeIndex);
    programmes_.erase(programmes_.begin() + programmeIndex);
}

void ProgrammesContainer::removeFromElementViews(
    const ear::plugin::communication::ConnectionId &id) {
  for (auto const& programme : programmes_) {
    auto const& container = programme->getElementsContainer();
    auto const& elements = container->elements;
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
      auto index = std::distance(elements.begin(), it);
      container->removeElement(static_cast<int>(index));
    }
  }
}

void ProgrammesContainer::moveProgrammeView(int oldIndex, int newIndex) {
  auto size = programmes_.size();
  if (oldIndex >= 0 && newIndex >= 0 && oldIndex < size && newIndex < size &&
      oldIndex != newIndex) {
    move(programmes_.begin(), oldIndex, newIndex);
  }
}

void ProgrammesContainer::setProgrammeViewName(int programmeIndex,
                                               const String &newName) {
  tabs_->setTabName(programmeIndex, newName);
  programmes_.at(programmeIndex)
      ->getNameTextEditor()
      ->setText(newName);
}

void ProgrammesContainer::setProgrammeViewLanguage(int programmeIndex,
                                                   const std::optional<std::string>& language) {

    if(language) {
        auto languageIndex = getLanguageIndex(*language);
        programmes_.at(programmeIndex)->getLanguageComboBox()->setSelectedId(languageIndex, NotificationType::dontSendNotification);
    }
}

int ProgrammesContainer::getProgrammeIndex(ProgrammeView *view) const {
  if(!view) {
    return -1;
  }
  auto it =
      std::find_if(programmes_.begin(), programmes_.end(),
                   [view](auto const& entry) { return view == entry.get(); });
  if (it != programmes_.end()) {
    return static_cast<int>(std::distance(programmes_.begin(), it));
  }
  return -1;
}

int ProgrammesContainer::getProgrammeIndex(ElementViewList *list) const {
  auto it = std::find_if(
      programmes_.begin(),
      programmes_.end(), [list](auto const& entry) {
        return entry->getElementsContainer()->list.get() == list;
      });
  if(it != programmes_.end()) {
    return static_cast<int>(std::distance(programmes_.begin(), it));
  }
  return -1;
}

void ProgrammesContainer::updateElementOverview(
ProgrammeObjects const& objects) {
  auto view = programmes_.at(objects.index());
  view->getElementOverview()->resetItems(objects);
}

void ProgrammesContainer::dataReset(const proto::ProgrammeStore &programmes, const ItemMap &items) {
    clear();
    auto selectedProgramme = programmes.selected_programme_index();
    for (int i = 0; i < programmes.programme_size(); ++i) {
        auto const& programme = programmes.programme(i);
        addProgrammeView(programme);
        ProgrammeObjects programmeObjects({i, i == selectedProgramme},
                                          programme,
                                          items);
        updateElementOverview(programmeObjects);

        for (auto const& element : programme.element()) {
            if (element.has_object()) {
                auto const& object = element.object();
                auto id = communication::ConnectionId{object.connection_id()};
                auto itemIt = items.find(id);
                // this might fail on project reload before inputs connect
                if(itemIt != items.end()) {
                    addObjectView(i,
                                  itemIt->second,
                                  element.object());
                }
            }
        }
    }
    tabs_->selectTab(selectedProgramme);
}

void ProgrammesContainer::programmeAdded(
        ProgrammeStatus status,
        proto::Programme const& programme) {
    addProgrammeView(programme);
    setProgrammeViewName(status.index, programme.name());
    data_.selectProgramme(status.index);
}

void ProgrammesContainer::programmeUpdated(
        ProgrammeStatus status,
        proto::Programme const& programme) {
    setProgrammeViewName(status.index, programme.name());
    setProgrammeViewLanguage(status.index, programme.language());
    auto& elementViews = programmes_.at(status.index)->getElementsContainer()->elements;
    auto const& progElements = programme.element();
    std::stable_sort(elementViews.begin(), elementViews.end(), [&progElements](auto const& lhs, auto const& rhs) {
       auto lhsObject = std::dynamic_pointer_cast<ObjectView>(lhs);
       auto rhsObject = std::dynamic_pointer_cast<ObjectView>(rhs);
       if(!lhsObject || !rhsObject) return false;
       auto lhPosition = std::find_if(progElements.begin(), progElements.end(), [&lhsObject](auto const& el) {
           if(!el.has_object()) return false;
           return el.object().connection_id() == lhsObject->getData().item.connection_id();
       });
       auto rhPosition = std::find_if(progElements.begin(), progElements.end(), [&rhsObject](auto const& el) {
          if(!el.has_object()) return false;
          return el.object().connection_id() == rhsObject->getData().item.connection_id();
       });
       return lhPosition < rhPosition;
    });
    programmes_.at(status.index)->getElementsContainer()->list->resized();
}

void ProgrammesContainer::objectDataChanged(ObjectView::Data data) {
    data_.updateElement(data.item.connection_id(), data.object);
}

void ProgrammesContainer::nameChanged(ProgrammeView* view, const String& newName) {
    auto index = getProgrammeIndex(view);
    if (index >= 0) {
        data_.setProgrammeName(index, newName.toStdString());
    }
}
void ProgrammesContainer::languageChanged(ProgrammeView* view, int languageIndex) {
    auto programmeIndex = getProgrammeIndex(view);
    if (programmeIndex >= 0) {
        if (languageIndex >= 0 && languageIndex < LANGUAGES.size()) {
            auto language = LANGUAGES.at(languageIndex).alpha3;
            data_.setProgrammeLanguage(programmeIndex, language);
        } else {
            data_.clearProgrammeLanguage(programmeIndex);
        }
    }
}

void ProgrammesContainer::elementMoved(ElementViewList* list, int oldIndex, int newIndex) {
    auto programmeIndex = getProgrammeIndex(list);
    assert(programmeIndex >= 0);
    data_.moveElement(programmeIndex, oldIndex, newIndex);
}

void ProgrammesContainer::removeElementClicked(ElementViewList* list, ElementView* view) {
    auto programmeIndex = getProgrammeIndex(list);
    assert (programmeIndex >= 0);
    if(auto objectView = dynamic_cast<ObjectView*>(view)) {
        auto id = objectView->getData().item.connection_id();
        data_.removeElementFromProgramme(programmeIndex, id);
    }
}

void ProgrammesContainer::addTabClicked(
        EarTabbedComponent* tabbedComponent) {
    data_.addProgramme();
}

void ProgrammesContainer::tabSelected(EarTabbedComponent*, int index) {
    data_.selectProgramme(index);
}

void ProgrammesContainer::tabMoved(EarTabbedComponent*, int oldIndex,
                                          int newIndex) {
    data_.moveProgramme(oldIndex, newIndex);
}

void ProgrammesContainer::removeTabClicked(
        EarTabbedComponent* tabbedComponent, int index) {
    auto progCount = tabbedComponent->tabCount();
    if (progCount == 1) {
        NativeMessageBox::showMessageBox(MessageBoxIconType::NoIcon,
                                         String("Cannot delete last programme"),
                                         "The Scene must always have at least one programme.",
                                         this);
        return;
    }
    auto programmeName = tabbedComponent->getName();
    auto text = String("Do you really want to delete \"");
    text += String(programmeName);
    text += String("\"?");
    if (NativeMessageBox::showOkCancelBox(
            MessageBoxIconType::NoIcon,
            String("Delete Programme?"),
            text,
            this,
            nullptr)) {
        data_.removeProgramme(index);
    }
}

void ProgrammesContainer::tabBarDoubleClicked(
        EarTabbedComponent* tabbedComponent) {
    addTabClicked(tabbedComponent);
}

void ProgrammesContainer::addItemClicked(ProgrammeView *view) {
    for(auto listener : listeners_) {
        listener->addItemClicked();
    }
}

void ProgrammesContainer::addListener(Listener *listener) {
    listeners_.push_back(listener);
}
