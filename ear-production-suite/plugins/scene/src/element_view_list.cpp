#pragma once

#include "JuceHeader.h"

#include "element_view_list.hpp"
#include "elements_container.hpp"
#include "../../shared/components/ear_colour_indicator.hpp"
#include "../../shared/components/level_meter.hpp"
#include "../../shared/components/look_and_feel/colours.hpp"
#include "../../shared/components/look_and_feel/fonts.hpp"

#include <cassert>

namespace ear {
namespace plugin {
namespace ui {

ElementViewList::ElementViewList(ElementsContainer* parentContainer)
    : dropIndicator_(std::make_unique<EarDropIndicator>()),
      parentContainer{parentContainer},
      helpLabel_(std::make_unique<Label>()) {
  addChildComponent(dropIndicator_.get());
  assert(parentContainer);
  helpLabel_->setText("Add Items by clicking the Buttons above",
                      dontSendNotification);
  helpLabel_->setFont(EarFonts::Items);
  helpLabel_->setColour(Label::textColourId,
                        EarColours::Text.withAlpha(Emphasis::medium));
  addAndMakeVisible(helpLabel_.get());
}

void ElementViewList::paint(Graphics& g) {
  g.fillAll(EarColours::Background);
}

void ElementViewList::parentSizeChanged() {
  setSize(getParentWidth(),
          std::max(getParentHeight(), getHeightOfAllItems()));
}

void ElementViewList::resized() {
  setSize(getParentWidth(),
          std::max(getParentHeight(), getHeightOfAllItems()));
  auto labelBounds = getLocalBounds().removeFromTop(60);
  helpLabel_->setVisible(parentContainer->elements.size() == 0);
  helpLabel_->setBounds(labelBounds.reduced(30, 20));
  auto area = getLocalBounds();
  for (int i = 0; i < parentContainer->elements.size(); ++i) {
    if (dropIndicator_->isVisible() && i == dropIndex_) {
      dropIndicator_->setBounds(
          area.removeFromTop(indicatorHeight_).reduced(0, 2));
      area.removeFromTop(margin_);
    }
    parentContainer->elements.at(i)->setBounds(
        area.removeFromTop(parentContainer->elements.at(i)->getDesiredHeight()));
    area.removeFromTop(margin_);
  }
  if (dropIndicator_->isVisible() && dropIndex_ == parentContainer->elements.size()) {
    dropIndicator_->setBounds(
        area.removeFromTop(indicatorHeight_).reduced(0, 2));
    area.removeFromTop(margin_);
  }
}

int ElementViewList::getHeightOfAllItems() const {
  int ret = 0;
  for (const auto element : parentContainer->elements) {
    ret += element->getDesiredHeight() + margin_;
  }
  if (dropIndicator_->isVisible()) {
    ret += indicatorHeight_ + margin_;
  }
  return ret;
}

bool ElementViewList::isInterestedInDragSource(
    const SourceDetails& dragSourceDetails) {
  return true;
};

void ElementViewList::itemDragEnter(const SourceDetails& dragSourceDetails) {
  dropIndicator_->setVisible(true);
}

int ElementViewList::getDropIndexForPosition(int yPos) {
  int y = 0;
  int previousHeight = 0;
  int currentHeight = 0;
  for (int i = 0; i < parentContainer->elements.size(); ++i) {
    currentHeight = parentContainer->elements.at(i)->getHeight();
    if (y - 0.5f * previousHeight < yPos &&
        yPos < y + 0.51f * currentHeight) {
      return i;
    }
    y += currentHeight;
    previousHeight = currentHeight;
  }
  return parentContainer->elements.size();
}

void ElementViewList::itemDragMove(const SourceDetails& dragSourceDetails) {
  dropIndex_ =
      getDropIndexForPosition(dragSourceDetails.localPosition.getY());
  resized();
}

void ElementViewList::itemDropped(const SourceDetails& dragSourceDetails) {
  if (auto component = dragSourceDetails.sourceComponent.get()) {
    if (auto element = dynamic_cast<ElementView*>(component)) {
      auto it = std::find_if(
        parentContainer->elements.begin(), parentContainer->elements.end(),
        [element](auto candidate) { return candidate.get() == element; });
      size_t oldIndex = std::distance(parentContainer->elements.begin(), it);
      int newIndex = dropIndex_ > oldIndex ? dropIndex_ - 1 : dropIndex_;
      parentContainer->moveElement(oldIndex, newIndex);
    }
  }
  dropIndicator_->setVisible(false);
  resized();
}

void ElementViewList::itemDragExit(const SourceDetails& dragSourceDetails) {
  dropIndicator_->setVisible(false);
  resized();
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
