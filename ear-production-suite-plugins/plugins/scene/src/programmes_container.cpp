//
// Created by Richard Bailey on 28/02/2022.
//
#include "programmes_container.hpp"
#include "element_view_list.hpp"
#include "object_view.hpp"
#include "programme_view.hpp"
#include "helper/move.hpp"
#include "components/overlay.hpp"
#include "communication/common_types.hpp"

namespace {

template <typename ComponentType>
ComponentType* getAncestorComponentOfType(Component* startingComponent) {
  auto parentComponent = startingComponent->getParentComponent();
  while(parentComponent) {
    auto candidate = dynamic_cast<ComponentType*>(parentComponent);
    if(candidate) return candidate;
    parentComponent = parentComponent->getParentComponent();
  }
  return nullptr;
}

}


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
  const ProgrammeInternalId& progId, const proto::InputItemMetadata &inputItem,
    const proto::Object &programmeElement) {

  auto objectType = ObjectView::ObjectType::Other;

  if (inputItem.has_obj_metadata()) {
    objectType = ObjectView::ObjectType::Object;
  }
  if (inputItem.has_ds_metadata()) {
    objectType = ObjectView::ObjectType::DirectSpeakers;
  }
  if (inputItem.has_hoa_metadata()) {
    objectType = ObjectView::ObjectType::HOA;
  }

  auto view = std::make_shared<ObjectView>(objectType);
  view->setData({inputItem, programmeElement});
  updateMeter(inputItem, view, true);
  for(auto programme : programmes_) {
    if(programme->getProgrammeId() == progId) {
      auto container = programme->getElementsContainer();
      container->addElement(view);
    }
  }
  view->addListener(this);
}

void ear::plugin::ui::ProgrammesContainer::addProgrammeView(
    const proto::Programme &programme) {

  auto view = std::make_shared<ProgrammeView>(programme.programme_internal_id());
  view->getNameTextEditor()->setText(programme.name(), false);
  view->getLanguageComboBox()->selectEntry(
      getIndexForAlphaN(programme.language()), dontSendNotification);
  view->addListener(this);
  view->getElementsContainer()->addListener(this);
  view->getElementOverview()->setProgramme(programme);
  programmes_.push_back(view);
  tabs_->addTab(programme.name(), view.get(), programme.programme_internal_id(), false);
}

void ProgrammesContainer::clear() {
  tabs_->removeAllTabs();
  programmes_.clear();
}

void ProgrammesContainer::itemsAddedToProgramme(
    ProgrammeStatus status, std::vector<ProgrammeObject> const& items) {
  assert(!items.empty());
  for(auto& item : items) {
      addObjectView(status.id, item.inputMetadata, item.programmeObject);
  }
  for(auto programme : programmes_) {
    if(programme->getProgrammeId() == status.id) {
      programme->getElementOverview()->itemsAdded(items);
    }
  }
}

void ProgrammesContainer::itemRemovedFromProgramme(ProgrammeStatus status, communication::ConnectionId const& id) {
  for(auto programme : programmes_) {
    if(programme->getProgrammeId() == status.id) {
      auto overview = programme->getElementOverview();
      overview->itemRemoved(id);
    }
  }
}

void ProgrammesContainer::inputRemoved(communication::ConnectionId const& id) {
    removeFromElementViews(id);
}

void ProgrammesContainer::programmeItemUpdated(ProgrammeStatus status, ProgrammeObject const& item) {
  for(auto programme : programmes_) {
    if(programme->getProgrammeId() == status.id) {
      auto overview = programme->getElementOverview();
      overview->itemChanged(item);
      auto container = programme->getElementsContainer();
      auto view = container->getObjectView(item.inputMetadata.connection_id());
      if(view) {
        updateMeter(item.inputMetadata, view);
        view->setInputItemMetadata(item.inputMetadata);
      }
    }
  }
}

void ear::plugin::ui::ProgrammesContainer::programmeOrderChanged(std::vector<ProgrammeStatus> programmes)
{
  std::vector<ProgrammeInternalId> progIds;
  for(const auto &programme : programmes) {
    progIds.push_back(programme.id);
  }
  setProgrammeViewOrder(progIds);
}

void ProgrammesContainer::programmeSelected(const ProgrammeObjects &objects) {
  tabs_->selectTab(objects.id(), dontSendNotification);
}

void ProgrammesContainer::programmeRemoved(ProgrammeStatus status) {
    tabs_->removeTab(status.id);
    for(int i = programmes_.size() - 1; i >= 0; i--) {
      if(programmes_.at(i)->getProgrammeId() == status.id) {
        programmes_.erase(programmes_.begin() + i);
      }
    }
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

void ear::plugin::ui::ProgrammesContainer::setProgrammeViewOrder(std::vector<ProgrammeInternalId> progIds)
{
  std::vector<std::shared_ptr<ProgrammeView>> orderedProgrammes;
  std::vector<bool> programmeViewChecklist(programmes_.size(), false);

  for(const auto& progId : progIds) {
    for(int i = 0; i < programmes_.size(); i++) {
      if(programmes_[i]->getProgrammeId() == progId) {
        programmeViewChecklist[i] = true;
        orderedProgrammes.push_back(programmes_[i]);
      }
    }
  }
  // Re-add anything we didn't find to the end
  for(int i = 0; i < programmeViewChecklist.size(); i++) {
    if(!programmeViewChecklist[i]) {
      orderedProgrammes.push_back(programmes_[i]);
    }
  }
  programmes_ = orderedProgrammes;
}

void ProgrammesContainer::setProgrammeViewName(const ProgrammeInternalId& progId,
                                               const String &newName) {
  tabs_->setTabName(progId, newName);
  for(auto programme : programmes_) {
    if(programme->getProgrammeId() == progId) {
      programme->getNameTextEditor()->setText(newName);
    }
  }
}

void ProgrammesContainer::setProgrammeViewLanguage(const ProgrammeInternalId& progId,
                                                   const std::optional<std::string>& language) {
  for(auto programme : programmes_) {
    if(programme->getProgrammeId() == progId) {
      auto languageIndex = language? getLanguageIndex(*language) : -1;
      programme->getLanguageComboBox()->setSelectedId(languageIndex, NotificationType::dontSendNotification);
    }
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
  for(auto programme : programmes_) {
    if(programme->getProgrammeId() == objects.id()) {
      programme->getElementOverview()->resetItems(objects);
    }
  }
}

void ear::plugin::ui::ProgrammesContainer::updateMeter(const proto::InputItemMetadata & item, std::shared_ptr<ObjectView> view, bool forceUpdate)
{
  assert(view);
  auto oldItem = view->getData().item;

  bool updateRequired = false;
  int startingChannel = item.routing();
  int channelCount = 1;

  if(item.has_obj_metadata() && oldItem.has_obj_metadata()) {
    if(forceUpdate ||
       oldItem.routing() != item.routing()) {
      updateRequired = true;
    }
  }

  if(item.has_ds_metadata() && oldItem.has_ds_metadata()) {
    if(forceUpdate ||
       oldItem.routing() != item.routing() ||
       oldItem.ds_metadata().layout() != item.ds_metadata().layout()) {
      updateRequired = true;
      auto ssIndex = item.ds_metadata().layout();
      if(ssIndex >= 0 && ssIndex < SPEAKER_SETUPS.size()) {
        channelCount = static_cast<int>(SPEAKER_SETUPS.at(ssIndex).speakers.size());
      }
    }
  }

  if(item.has_hoa_metadata() && oldItem.has_hoa_metadata()) {
    if(forceUpdate ||
       oldItem.routing() != item.routing() ||
       oldItem.hoa_metadata().packformatidvalue() != item.hoa_metadata().packformatidvalue()) {
      auto packFormatId = item.hoa_metadata().packformatidvalue();
      if(packFormatId >= 0) {
        auto commonDefinitionHelper = AdmCommonDefinitionHelper::getSingleton();
        auto elementRelationships =
          commonDefinitionHelper->getElementRelationships();
        auto pfData =
          commonDefinitionHelper->getPackFormatData(4, packFormatId);
        if(pfData) {
          updateRequired = true;
          channelCount =
            static_cast<int>(pfData->relatedChannelFormats.size());
        }
      }
    }
  }

  if(updateRequired) {
    std::vector<int> routing(channelCount);
    std::iota(routing.begin(), routing.end(), startingChannel);
    view->getLevelMeter()->setMeter(meterCalculator_, routing);
  }
}

void ProgrammesContainer::dataReset(const proto::ProgrammeStore &programmes, const ItemMap &items) {
    clear();
    auto selectedProgrammeId = programmes.selected_programme_internal_id();
    for (const auto& programme : programmes.programme()) {
        auto progId = programme.programme_internal_id();
        addProgrammeView(programme);
        ProgrammeObjects programmeObjects({progId, progId == selectedProgrammeId},
                                          programme,
                                          items);
        updateElementOverview(programmeObjects);

        for (auto const& element : programme.element()) {
            if (element.has_object()) {
                auto const& object = element.object();
                auto objId = communication::ConnectionId{object.connection_id()};
                auto itemIt = items.find(objId);
                // this might fail on project reload before inputs connect
                if(itemIt != items.end()) {
                    addObjectView(progId,
                                  itemIt->second,
                                  element.object());
                }
            }
        }
    }
    tabs_->selectTab(selectedProgrammeId, dontSendNotification);
}

void ProgrammesContainer::programmeAdded(
        ProgrammeStatus status,
        proto::Programme const& programme) {
    addProgrammeView(programme);
    setProgrammeViewName(status.id, programme.name());
    data_.selectProgramme(status.id);
}

void ProgrammesContainer::programmeUpdated(
        ProgrammeStatus status,
        proto::Programme const& programme) {
    setProgrammeViewName(status.id, programme.name());
    setProgrammeViewLanguage(status.id, programme.language());

    for(auto programmeView : programmes_) {
      if(programmeView->getProgrammeId() == status.id) {
        auto& elementViews = programmeView->getElementsContainer()->elements;
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
        programmeView->getElementsContainer()->list->resized();
      }
    }
}

void ProgrammesContainer::objectDataChanged(ObjectView::Data data) {
    data_.updateElement(data.item.connection_id(), data.object);
}

void ProgrammesContainer::nameChanged(ProgrammeView* view, const String& newName) {
    data_.setProgrammeName(view->getProgrammeId(), newName.toStdString());
}
void ProgrammesContainer::languageChanged(ProgrammeView* view, int languageIndex) {
      if (languageIndex >= 0 && languageIndex < LANGUAGES.size()) {
          auto language = LANGUAGES.at(languageIndex).alpha3;
          data_.setProgrammeLanguage(view->getProgrammeId(), language);
      } else {
          data_.clearProgrammeLanguage(view->getProgrammeId());
      }
}

void ProgrammesContainer::elementMoved(ElementViewList* list, int oldIndex, int newIndex) {
    // ElementViewList will be a few components down from the associated ProgrammeView - trace up
    ProgrammeView* programmeView = getAncestorComponentOfType<ProgrammeView>(list);
    assert(programmeView);

    if(programmeView) {
      auto associatedProgId = programmeView->getProgrammeId();
      if(auto elementsContainer = programmeView->getElementsContainer()) {
        std::vector<ear::plugin::communication::ConnectionId> ids;
        for(const auto& element : elementsContainer->elements) {
          auto objectView = std::dynamic_pointer_cast<ObjectView> (element);
          if(objectView) {
            ids.push_back(objectView->getConnectionId());
          }
        }
        data_.setElementOrder(associatedProgId, ids);
      }
    }
}

void ProgrammesContainer::removeElementClicked(ElementViewList* list, ElementView* elementView) {
    // ElementViewList will be a few components down from the associated ProgrammeView - trace up
    ProgrammeView* programmeView = getAncestorComponentOfType<ProgrammeView>(list);
    assert(programmeView);

    if(programmeView) {
      auto objectView = dynamic_cast<ObjectView*>(elementView);
      assert(objectView);
      if(objectView) {
        auto associatedProgId = programmeView->getProgrammeId();
        auto id = objectView->getData().item.connection_id();
        data_.removeElementFromProgramme(associatedProgId, id);
      }
    }
}

void ProgrammesContainer::addTabClicked(
        EarTabbedComponent* tabbedComponent) {
    data_.addProgramme();
}

void ProgrammesContainer::tabSelectedId(EarTabbedComponent*, const std::string& progId) {
    data_.selectProgramme(progId);
}

void ProgrammesContainer::tabMovedId(EarTabbedComponent*, const std::string& progId,
                                          int newIndex) {
    std::vector<ProgrammeInternalId> progIds;
    for(int i = 0; i < tabs_->tabCount(); i++) {
      progIds.push_back(tabs_->getTabIdFromIndex(i));
    }
    data_.setProgrammeOrder(progIds);
}

void ProgrammesContainer::removeTabClickedId(
        EarTabbedComponent* tabbedComponent, const std::string& progId) {
    auto progCount = tabbedComponent->tabCount();
    if (progCount == 1) {
        NativeMessageBox::showMessageBox(MessageBoxIconType::NoIcon,
                                         String("Cannot delete last programme"),
                                         "The Scene must always have at least one programme.",
                                         this);
        return;
    }
    auto programmeName = tabbedComponent->getTabName(progId);
    auto text = String("Do you really want to delete \"");
    text += String(programmeName);
    text += String("\"?");
    if (NativeMessageBox::showOkCancelBox(
            MessageBoxIconType::NoIcon,
            String("Delete Programme?"),
            text,
            this,
            nullptr)) {
        data_.removeProgramme(progId);
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
