//
// Created by Richard Bailey on 01/03/2022.
//
#include "items_container.hpp"
#include "components/ear_button.hpp"
#include "item_view.hpp"
#include "item_view_list.hpp"
#include "store_metadata.hpp"
#include "programme_internal_id.hpp"
#include <iostream>

using namespace ear::plugin::ui;
using namespace ear::plugin;

ItemsContainer::~ItemsContainer() = default;

ItemsContainer::ItemsContainer(Metadata& metadata)
    : data_{metadata},
      objectsList(std::make_unique<ItemViewList>()),
      directSpeakersList(std::make_unique<ItemViewList>()),
      hoaList(std::make_unique<ItemViewList>()),
      objectsViewport_(std::make_unique<Viewport>()),
      directSpeakersViewport_(std::make_unique<Viewport>()),
      hoaViewport_(std::make_unique<Viewport>()),
      objectsLabel_(std::make_unique<Label>()),
      directSpeakersLabel_(std::make_unique<Label>()),
      hoaLabel_(std::make_unique<Label>()),
      addButton_(std::make_unique<EarButton>()) {
  objectsViewport_->setViewedComponent(objectsList.get(), false);
  objectsViewport_->setScrollBarsShown(true, false);
  objectsViewport_->getVerticalScrollBar().setColour(ScrollBar::thumbColourId,
                                                     EarColours::Area04dp);
  addAndMakeVisible(objectsViewport_.get());

  directSpeakersViewport_->setViewedComponent(directSpeakersList.get(),
                                              false);
  directSpeakersViewport_->setScrollBarsShown(true, false);
  directSpeakersViewport_->getVerticalScrollBar().setColour(
      ScrollBar::thumbColourId, EarColours::Area04dp);
  addAndMakeVisible(directSpeakersViewport_.get());

  hoaViewport_->setViewedComponent(hoaList.get(), false);
  hoaViewport_->setScrollBarsShown(true, false);
  hoaViewport_->getVerticalScrollBar().setColour(ScrollBar::thumbColourId,
                                                 EarColours::Area04dp);
  addAndMakeVisible(hoaViewport_.get());

  objectsLabel_->setText("Objects", dontSendNotification);
  objectsLabel_->setFont(EarFontsSingleton::instance().Label);
  objectsLabel_->setColour(Label::backgroundColourId, EarColours::Area04dp);
  objectsLabel_->setColour(Label::textColourId, EarColours::Heading);
  addAndMakeVisible(objectsLabel_.get());

  directSpeakersLabel_->setText("Direct Speakers", dontSendNotification);
  directSpeakersLabel_->setFont(EarFontsSingleton::instance().Label);
  directSpeakersLabel_->setColour(Label::backgroundColourId,
                                  EarColours::Area04dp);
  directSpeakersLabel_->setColour(Label::textColourId, EarColours::Heading);
  addAndMakeVisible(directSpeakersLabel_.get());

  hoaLabel_->setText("Higher Order Ambisonics", dontSendNotification);
  hoaLabel_->setFont(EarFontsSingleton::instance().Label);
  hoaLabel_->setColour(Label::backgroundColourId, EarColours::Area04dp);
  hoaLabel_->setColour(Label::textColourId, EarColours::Heading);
  addAndMakeVisible(hoaLabel_.get());

  addButton_->setButtonText("Add");
  addButton_->onClick = [&]() {
    std::vector<communication::ConnectionId> ids;
    for (auto const& item : this->objectsItems) {
      if (item->isEnabled() && item->isSelected()) {
        ids.push_back(item->getId());
      }
    }
    for (auto const& item : this->directSpeakersItems) {
      if (item->isEnabled() && item->isSelected()) {
        ids.push_back(item->getId());
      }
    }
    for (auto const& item : this->hoaItems) {
      if (item->isEnabled() && item->isSelected()) {
        ids.push_back(item->getId());
      }
    }

    if(!ids.empty()) {
      data_.addItemsToSelectedProgramme(ids);
    }
    Component::BailOutChecker checker(this);
    listeners_.callChecked(
          checker, [this, ids](Listener& l) {
              l.addItemsClicked(this, ids);
          });
    if (checker.shouldBailOut()) {
      return;
    }
  };

  addAndMakeVisible(addButton_.get());
}

void ear::plugin::ui::ItemsContainer::paint(Graphics &g) {
  auto area = getLocalBounds();
  auto buttonArea = area.removeFromBottom(50);
  g.setColour(EarColours::WindowBorder);
  g.fillRect(buttonArea.removeFromTop(2));
  g.setColour(EarColours::Area01dp);
  g.fillRect(buttonArea);

  auto objectsArea = area.removeFromLeft(getWidth() / 3);
  auto remainingArea = area.removeFromRight(getWidth() * 2 / 3);
  auto directSpeakersArea = remainingArea.removeFromLeft(getWidth() / 3);
  auto hoaArea = remainingArea.removeFromRight(getWidth() / 3);

  g.setColour(EarColours::WindowBorder);
  g.fillRect(objectsArea.removeFromRight(2));
  g.fillRect(objectsArea.removeFromLeft(2));
  // g.setColour(EarColours::WindowBorder);
  g.fillRect(directSpeakersArea.removeFromLeft(2));
  // g.setColour(EarColours::WindowBorder);
  g.fillRect(hoaArea.removeFromLeft(2));
}

void ear::plugin::ui::ItemsContainer::resized() {
  auto area = getLocalBounds();
  auto buttonArea = area.removeFromBottom(50);
  addButton_->setBounds(buttonArea.reduced(10, 10).removeFromLeft(80));

  auto objectsArea = area.removeFromLeft(getWidth() / 3);
  auto remainingArea = area.removeFromRight(getWidth() * 2 / 3);
  auto directSpeakersArea = remainingArea.removeFromLeft(getWidth() / 3);
  auto hoaArea = remainingArea.removeFromRight(getWidth() / 3);

  objectsLabel_->setBounds(objectsArea.removeFromTop(30));
  objectsViewport_->setBounds(objectsArea.reduced(2, 2));
  objectsList->setBounds(objectsArea.reduced(2, 2));

  directSpeakersLabel_->setBounds(directSpeakersArea.removeFromTop(30));
  directSpeakersViewport_->setBounds(directSpeakersArea.reduced(2, 2));
  directSpeakersList->setBounds(directSpeakersArea.reduced(2, 2));

  hoaLabel_->setBounds(hoaArea.removeFromTop(30));
  hoaViewport_->setBounds(hoaArea.reduced(2, 2));
  hoaList->setBounds(hoaArea.reduced(2, 2));
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

void ItemsContainer::createOrUpdateView(proto::InputItemMetadata const& item) {
    if (item.has_ds_metadata()) {
        ::createOrUpdateView(item,
                             directSpeakersItems,
                             *directSpeakersList);
    } else if (item.has_obj_metadata()) {
        ::createOrUpdateView(item,
                             objectsItems,
                             *objectsList);
    } else if (item.has_hoa_metadata()) {
        ::createOrUpdateView(item,
                             hoaItems,
                             *hoaList);
    }
}

void ItemsContainer::createOrUpdateViews(
    const std::map<communication::ConnectionId, proto::InputItemMetadata>&
    allItems) {
  for (auto const& entry : allItems) {
      createOrUpdateView(entry.second);
  }
}

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

void ItemsContainer::removeView(const communication::ConnectionId& id) {
  removeItemFromViews(id, directSpeakersItems, *directSpeakersList);
  removeItemFromViews(id, objectsItems, *objectsList);
  removeItemFromViews(id, hoaItems, *hoaList);
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

void themeItemsFor(std::vector<std::shared_ptr<ItemView>>& views,
                   const ProgrammeObjects& programme) {
  for (auto const& view : views) {
    if (programme.dataItem(view->getId())) {
      setPresentTheme(*view);
    } else {
      setMissingTheme(*view);
    }
  }
}
void setThemeFor(std::vector<std::shared_ptr<ItemView>>& views,
                 communication::ConnectionId const& id, bool present) {
  auto view = std::find_if(views.begin(), views.end(), [&id](auto const& view) {
    return view->getId() == id;
  });
  if (view != views.end()) {
    present ? setPresentTheme(**view) : setMissingTheme(**view);
  }
}
}

void ItemsContainer::themeItemsFor(const ProgrammeObjects& programme) {
  ::themeItemsFor(directSpeakersItems, programme);
  ::themeItemsFor(objectsItems, programme);
  ::themeItemsFor(hoaItems, programme);
}

void ItemsContainer::setMissingThemeFor(const communication::ConnectionId& id) {
  ::setThemeFor(directSpeakersItems, id, false);
  ::setThemeFor(objectsItems, id, false);
  ::setThemeFor(hoaItems, id, false);
}

void ItemsContainer::setPresentThemeFor(const communication::ConnectionId& id) {
  ::setThemeFor(directSpeakersItems, id, true);
  ::setThemeFor(objectsItems, id, true);
  ::setThemeFor(hoaItems, id, true);
}

void ItemsContainer::dataReset(const proto::ProgrammeStore &programmeStore, const ItemMap &items) {
    auto selectedId = programmeStore.selected_programme_internal_id();
    auto selectedIndex = getProgrammeIndexFromId(programmeStore, selectedId);
    createOrUpdateViews(items);
    if(selectedIndex >= 0) {
        auto const& selectedProgramme = programmeStore.programme(selectedIndex);
        themeItemsFor({{selectedProgramme.programme_internal_id(), true},
                       selectedProgramme,
                       items});
    }
}

void ItemsContainer::programmeSelected(ProgrammeObjects const& objects) {
    themeItemsFor(objects);
}

void ItemsContainer::itemsAddedToProgramme(ProgrammeStatus status, std::vector<ProgrammeObject> const& items) {
    if(status.isSelected) {
        for(auto const& item : items) {
            setPresentThemeFor(item.inputMetadata.connection_id());
        }
    }
}

void ItemsContainer::itemRemovedFromProgramme(ProgrammeStatus status, const communication::ConnectionId &id) {
    if(status.isSelected) {
        setMissingThemeFor(id);
    }
}

void ItemsContainer::inputRemoved(const communication::ConnectionId &id) {
    removeView(id);
}

void ItemsContainer::inputUpdated(const InputItem &item, proto::InputItemMetadata const&) {
    createOrUpdateView(item.data);
}
