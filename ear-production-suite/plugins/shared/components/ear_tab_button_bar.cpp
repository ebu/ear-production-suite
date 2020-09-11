#include "ear_tabbed_component.hpp"

#include "../helper/graphics.hpp"
#include "ear_button.hpp"
#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"
#include "look_and_feel/name_text_editor.hpp"
#include "look_and_feel/slider.hpp"

#include <cmath>

using namespace ear::plugin::ui;

EarTabButtonBarViewport::EarTabButtonBarViewport() : Viewport(){};

void EarTabButtonBarViewport::visibleAreaChanged(
    const juce::Rectangle<int>& newVisibleArea) {
  if (onVisibleAreaChanged) {
    onVisibleAreaChanged(newVisibleArea);
  }
}

EarTabButtonBarIndicator::EarTabButtonBarIndicator() {
  setColour(backgroundColourId, EarColours::Area01dp);
}

void EarTabButtonBarIndicator::setIcon(std::unique_ptr<Drawable> drawable) {
  drawable_ = std::move(drawable);
}

void EarTabButtonBarIndicator::paint(Graphics& g) {
  g.fillAll(findColour(backgroundColourId));
  drawable_->drawWithin(
      g, getLocalBounds().toFloat(),
      RectanglePlacement::centred | RectanglePlacement::doNotResize, 1.f);
}

EarTabButtonBar::EarTabButtonBar()
    : moreLeftIndicator_(std::make_unique<EarTabButtonBarIndicator>()),
      moreRightIndicator_(std::make_unique<EarTabButtonBarIndicator>()),
      tabButtonBarViewport_(std::make_unique<EarTabButtonBarViewport>()),
      tabButtonBarContent_(std::make_unique<EarTabButtonBarContent>()) {
  tabButtonBarViewport_->setViewedComponent(tabButtonBarContent_.get(), false);
  tabButtonBarViewport_->setScrollBarsShown(false, false, false, true);

  moreLeftIndicator_->setIcon(std::unique_ptr<Drawable>(
      Drawable::createFromImageData(binary_data::keyboard_arrow_left_svg,
                                    binary_data::keyboard_arrow_left_svgSize)));
  moreRightIndicator_->setIcon(
      std::unique_ptr<Drawable>(Drawable::createFromImageData(
          binary_data::keyboard_arrow_right_svg,
          binary_data::keyboard_arrow_right_svgSize)));

  tabButtonBarViewport_->onVisibleAreaChanged =
      [this](const juce::Rectangle<int>& newVisibleArea) {
        this->updateIndicatorVisibility();
      };

  tabButtonBarContent_->onDoubleClick = [this]() {
    if (this->onDoubleClick) {
      this->onDoubleClick();
    }
  };

  addChildComponent(moreLeftIndicator_.get());
  addChildComponent(moreRightIndicator_.get());
  addAndMakeVisible(tabButtonBarViewport_.get());
};

void EarTabButtonBar::addButtonAndMakeVisible(EarTabButton* button) {
  tabButtonBarContent_->addAndMakeVisible(button);
}
void EarTabButtonBar::removeButton(EarTabButton* button) {
  tabButtonBarContent_->removeChildComponent(button);
}

void EarTabButtonBar::setTabs(std::vector<EarTab>* tabs) { tabs_ = tabs; }

void EarTabButtonBar::resized() {
  auto area = getLocalBounds();
  tabButtonBarViewport_->setBounds(area);
  tabButtonBarContent_->setBounds(area);
  updateTabBounds();
  moreLeftIndicator_->setBounds(area.removeFromLeft(getHeight()));
  moreRightIndicator_->setBounds(area.removeFromRight(getHeight()));
  updateIndicatorVisibility();
}

void EarTabButtonBar::updateIndicatorVisibility() {
  auto newVisibleArea = tabButtonBarViewport_->getViewArea();
  if (newVisibleArea.getX() == 0) {
    moreLeftIndicator_->setVisible(false);
  } else {
    moreLeftIndicator_->setVisible(true);
  }
  if (newVisibleArea.getRight() == tabButtonBarContent_->getWidth()) {
    moreRightIndicator_->setVisible(false);
  } else {
    moreRightIndicator_->setVisible(true);
  }
}

int EarTabButtonBar::updateTabBounds() {
  if (tabs_->size() == 0) {
    return 0;
  }
  int overallButtonWidth = getWidth();
  int maxOverallButtonWidth =
      tabs_->size() * maxButtonWidth_ + (tabs_->size() - 1) * buttonPadding_;
  int minOverallButtonWidth =
      tabs_->size() * minButtonWidth_ + (tabs_->size() - 1) * buttonPadding_;
  int buttonWidth = (getWidth() / tabs_->size()) - buttonPadding_;
  if (maxOverallButtonWidth <= getWidth()) {
    buttonWidth = maxButtonWidth_;
  } else if (minOverallButtonWidth > getWidth()) {
    buttonWidth = minButtonWidth_;
    overallButtonWidth = minOverallButtonWidth;
  }
  for (int i = 0; i < tabs_->size(); ++i) {
    tabs_->at(i).button->setBounds(i * (buttonWidth + buttonPadding_), 0,
                                   buttonWidth, getHeight());
  }
  auto viewPosition = tabButtonBarViewport_->getViewPosition();
  tabButtonBarContent_->setBounds(
      juce::Rectangle<int>{0, 0, overallButtonWidth, getHeight()});
  tabButtonBarViewport_->setViewPosition(viewPosition);
  return buttonWidth;
}

void EarTabButtonBar::scrollIfNecessaryTo(int index) {
  if (index < tabs_->size()) {
    auto tabButtonBounds = tabs_->at(index).button->getBounds();
    auto viewportBounds = tabButtonBarViewport_->getViewArea();
    if (!viewportBounds.contains(tabButtonBounds)) {
      if (viewportBounds.getX() > tabButtonBounds.getX()) {
        tabButtonBarViewport_->setViewPosition(tabButtonBounds.getPosition());
      } else {
        tabButtonBarViewport_->setViewPosition(
            tabButtonBounds.getPosition().translated(
                0, -viewportBounds.getWidth() + tabButtonBounds.getWidth()));
      }
    }
  }
}
