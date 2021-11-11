#pragma once

#include <memory>

#include "JuceHeader.h"

#include "components/ear_button.hpp"
#include "communication/common_types.hpp"
#include "item_view.hpp"
#include "item_view_list.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ItemsContainer : public Component {
 public:
  ItemsContainer()
      : objectsList(std::make_unique<ItemViewList>()),
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
    objectsLabel_->setFont(EarFonts::Label);
    objectsLabel_->setColour(Label::backgroundColourId, EarColours::Area04dp);
    objectsLabel_->setColour(Label::textColourId, EarColours::Heading);
    addAndMakeVisible(objectsLabel_.get());

    directSpeakersLabel_->setText("Direct Speakers", dontSendNotification);
    directSpeakersLabel_->setFont(EarFonts::Label);
    directSpeakersLabel_->setColour(Label::backgroundColourId,
                                    EarColours::Area04dp);
    directSpeakersLabel_->setColour(Label::textColourId, EarColours::Heading);
    addAndMakeVisible(directSpeakersLabel_.get());

    hoaLabel_->setText("Higher Order Ambisonics", dontSendNotification);
    hoaLabel_->setFont(EarFonts::Label);
    hoaLabel_->setColour(Label::backgroundColourId, EarColours::Area04dp);
    hoaLabel_->setColour(Label::textColourId, EarColours::Heading);
    addAndMakeVisible(hoaLabel_.get());

    addButton_->setButtonText("Add");
    addButton_->onClick = [&]() {
      std::vector<communication::ConnectionId> ids;
      for (auto item : this->objectsItems) {
        if (item->isEnabled() && item->isSelected()) {
          ids.push_back(item->getId());
        }
      }
      for (auto item : this->directSpeakersItems) {
        if (item->isEnabled() && item->isSelected()) {
          ids.push_back(item->getId());
        }
      }
      for (auto item : this->hoaItems) {
        if (item->isEnabled() && item->isSelected()) {
          ids.push_back(item->getId());
        }
      }
      Component::BailOutChecker checker(this);
      listeners_.callChecked(
          checker, [this, ids](Listener& l) { l.addItemsClicked(this, ids); });
      if (checker.shouldBailOut()) {
        return;
      }
    };

    addAndMakeVisible(addButton_.get());
  }

  void paint(Graphics& g) override {
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

  void resized() override {
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

  std::unique_ptr<ItemViewList> objectsList;
  std::unique_ptr<ItemViewList> directSpeakersList;
  std::unique_ptr<ItemViewList> hoaList;
  std::vector<std::shared_ptr<ItemView>> objectsItems;
  std::vector<std::shared_ptr<ItemView>> directSpeakersItems;
  std::vector<std::shared_ptr<ItemView>> hoaItems;

  class Listener {
   public:
    virtual ~Listener() = default;

    virtual void addItemsClicked(
        ItemsContainer* container,
        std::vector<communication::ConnectionId> ids) = 0;
  };

  void addListener(Listener* l) { listeners_.add(l); }
  void removeListener(Listener* l) { listeners_.remove(l); }

 private:
  std::unique_ptr<Viewport> objectsViewport_;
  std::unique_ptr<Viewport> directSpeakersViewport_;
  std::unique_ptr<Viewport> hoaViewport_;
  std::unique_ptr<Label> objectsLabel_;
  std::unique_ptr<Label> directSpeakersLabel_;
  std::unique_ptr<Label> hoaLabel_;
  std::unique_ptr<EarButton> addButton_;

  ListenerList<Listener> listeners_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ItemsContainer)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
