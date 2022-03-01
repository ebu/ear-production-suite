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

ProgrammesContainer::ProgrammesContainer()
    : tabs_(std::make_shared<EarTabbedComponent>()) {
  addAndMakeVisible(tabs_.get());
}

void ear::plugin::ui::ProgrammesContainer::resized() {
  std::lock_guard<std::mutex> lock(mutex_);
  tabs_->setBounds(getLocalBounds());
}

std::shared_ptr<ObjectView> ear::plugin::ui::ProgrammesContainer::addObjectView(
    int programmeIndex, const proto::InputItemMetadata &inputItem,
    const proto::Object &programmeElement,
    const std::shared_ptr<LevelMeterCalculator> &meterCalculator) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto objectType = ObjectView::ObjectType::Other;

  auto container = programmes_.at(programmeIndex)
      ->getElementsContainer();

  String speakerSetup;
  int numberOfChannels = 1;

  if (inputItem.has_obj_metadata()) {
    objectType = ObjectView::ObjectType::Object;
  }

  if (inputItem.has_ds_metadata()) {
    objectType = ObjectView::ObjectType::DirectSpeakers;
    auto layoutIndex = inputItem.ds_metadata().layout();
    if (layoutIndex >= 0) {
      speakerSetup = SPEAKER_SETUPS.at(layoutIndex).commonName;
      numberOfChannels = SPEAKER_SETUPS.at(layoutIndex).speakers.size();
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
      size_t cfCount(0);
      if (pfData) {
        numberOfChannels =
            static_cast<size_t>(pfData->relatedChannelFormats.size());
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
  std::lock_guard<std::mutex> lock(mutex_);

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
  std::lock_guard<std::mutex> lock(mutex_);
  tabs_->removeAllTabs();
  programmes_.clear();
}

void ProgrammesContainer::removeListeners(
    ear::plugin::ui::JuceSceneFrontendConnector *connector) {
  std::lock_guard<std::mutex> lock(mutex_);
  tabs_->removeListener(connector);
  for (auto const& view : programmes_) {
    view->removeListener(connector);
  }
}

void ProgrammesContainer::addTabListener(
    EarTabbedComponent::Listener *listener) {
  std::lock_guard<std::mutex> lock(mutex_);
  tabs_->addListener(listener);
}

void ProgrammesContainer::updateElementOverview(
    int programmeIndex, const ear::plugin::proto::Programme &programme) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto overview = programmes_.at(programmeIndex)->getElementOverview();
  overview->setProgramme(programme);
}

int ProgrammesContainer::programmeCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return programmes_.size();
}

void ProgrammesContainer::updateViews(
    const ear::plugin::proto::InputItemMetadata &item,
    const std::shared_ptr<LevelMeterCalculator> &meterCalculator) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto id = item.connection_id();
  for(auto const& programme : programmes_) {
    auto const& elements = programme->getElementsContainer()->elements;
    auto it = std::find_if(elements.begin(), elements.end(), [id](auto const& element) {
      if(auto objectView = std::dynamic_pointer_cast<ObjectView>(element)) {
        return id == objectView->getData().object.connection_id();
      }
      return false;
    });

    if(it != elements.end()) {
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
      assert(data.object.connection_id() == data.item.connection_id());
      view->setData(data);
      std::vector<int> routing(numberOfChannels);
      std::iota(routing.begin(), routing.end(), item.routing());
      view->getLevelMeter()->setMeter(meterCalculator, routing);
    }
  }
}

void ProgrammesContainer::removeFromElementViews(
    const ear::plugin::communication::ConnectionId &id) {
  std::lock_guard<std::mutex> lock(mutex_);

  for (int programmeIndex = 0;
       programmeIndex < programmeCount();
       ++programmeIndex) {
    auto const& container = programmes_.at(programmeIndex)
        ->getElementsContainer();
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
      container->removeElement(index);
    }
  }
}

void ProgrammesContainer::selectTab(int index) {
  std::lock_guard<std::mutex> lock(mutex_);
  tabs_->selectTab(index);
}

void ProgrammesContainer::moveProgrammeView(int oldIndex, int newIndex) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto size = programmes_.size();
  if (oldIndex >= 0 && newIndex >= 0 && oldIndex < size && newIndex < size &&
      oldIndex != newIndex) {
    move(programmes_.begin(), oldIndex, newIndex);
  }
}

void ProgrammesContainer::removeProgrammeView(int index) {
  std::lock_guard<std::mutex> lock(mutex_);
  tabs_->removeTab(index);
  programmes_.erase(programmes_.begin() + index);
}

void ProgrammesContainer::setProgrammeViewName(int programmeIndex,
                                               const String &newName) {
  std::lock_guard<std::mutex> lock(mutex_);
  tabs_->setTabName(programmeIndex, newName);
  programmes_.at(programmeIndex)
      ->getNameTextEditor()
      ->setText(newName);
}

int ProgrammesContainer::getProgrammeIndex(ProgrammeView *view) const {
  std::lock_guard<std::mutex> lock(mutex_);
  if(!view) {
    return -1;
  }
  auto it =
      std::find_if(programmes_.begin(), programmes_.end(),
                   [view](auto const& entry) { return view == entry.get(); });
  if (it != programmes_.end()) {
    return std::distance(programmes_.begin(), it);
  }
  return -1;
}

int ProgrammesContainer::getProgrammeIndex(ElementViewList *list) const {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = std::find_if(
      programmes_.begin(),
      programmes_.end(), [list](auto const& entry) {
        return entry->getElementsContainer()->list.get() == list;
      });
  if(it != programmes_.end()) {
    return std::distance(programmes_.begin(), it);
  }
  return -1;
}
