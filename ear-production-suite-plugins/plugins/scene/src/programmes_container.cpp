//
// Created by Richard Bailey on 28/02/2022.
//
#include "programmes_container.hpp"
#include "scene_frontend_connector.hpp"
#include "element_view_list.hpp"
#include "object_view.hpp"
#include "programme_view.hpp"
#include "helper/move.hpp"

using namespace ear::plugin::ui;
using namespace ear::plugin;

ProgrammesContainer::ProgrammesContainer()
    : tabs_(std::make_shared<EarTabbedComponent>()) {
  addAndMakeVisible(tabs_.get());
}

void ear::plugin::ui::ProgrammesContainer::resized() {
  tabs_->setBounds(getLocalBounds());
}

std::shared_ptr<ObjectView> ear::plugin::ui::ProgrammesContainer::addObjectView(
    int programmeIndex, const proto::InputItemMetadata &inputItem,
    const proto::Object &programmeElement,
    const std::shared_ptr<LevelMeterCalculator> &meterCalculator) {

  auto objectType = ObjectView::ObjectType::Other;

  auto container = programmes_.at(programmeIndex)
      ->getElementsContainer();

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
  view->getLevelMeter()->setMeter(meterCalculator, routing);
  container->addElement(view);
  return view;
}

void ear::plugin::ui::ProgrammesContainer::addProgrammeView(
    const proto::Programme &programme, JuceSceneFrontendConnector &connector) {

  auto view = std::make_shared<ProgrammeView>(&connector);
  view->getNameTextEditor()->setText(programme.name(), false);
  view->getLanguageComboBox()->selectEntry(
      getIndexForAlphaN(programme.language()), dontSendNotification);
  view->addListener(&connector);
  view->getElementsContainer()->addListener(&connector);
  view->getElementOverview()->setProgramme(programme);
  programmes_.push_back(view);
  tabs_->addTab(programme.name(), view.get(), false);
}

void ProgrammesContainer::clear() {
  tabs_->removeAllTabs();
  programmes_.clear();
}

void ProgrammesContainer::removeListeners(
    ear::plugin::ui::JuceSceneFrontendConnector *connector) {
  tabs_->removeListener(connector);
  for (auto const& view : programmes_) {
    view->removeListener(connector);
  }
}

void ProgrammesContainer::addTabListener(
    EarTabbedComponent::Listener *listener) {
  tabs_->addListener(listener);
}

void ProgrammesContainer::itemsAddedToProgramme(
    ProgrammeStatus status, std::vector<ProgrammeObject> const& items) {
  assert(!items.empty());
  auto programmeIndex = status.index;
  auto overview = programmes_.at(programmeIndex)->getElementOverview();
  overview->itemsAdded(items);
}

void ProgrammesContainer::itemRemovedFromProgramme(ProgrammeStatus status, communication::ConnectionId const& id) {
  auto programmeIndex = status.index;
  auto overview = programmes_.at(programmeIndex)->getElementOverview();
  overview->itemRemoved(id);
}

void ProgrammesContainer::programmeItemUpdated(ProgrammeStatus status, ProgrammeObject const& item) {
  auto programmeIndex = status.index;
  auto overview = programmes_.at(programmeIndex)->getElementOverview();
  overview->itemChanged(item);
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

void ProgrammesContainer::selectTab(int index) {
  tabs_->selectTab(index);
}

void ProgrammesContainer::moveProgrammeView(int oldIndex, int newIndex) {
  auto size = programmes_.size();
  if (oldIndex >= 0 && newIndex >= 0 && oldIndex < size && newIndex < size &&
      oldIndex != newIndex) {
    move(programmes_.begin(), oldIndex, newIndex);
  }
}

void ProgrammesContainer::removeProgrammeView(int index) {
  tabs_->removeTab(index);
  programmes_.erase(programmes_.begin() + index);
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
