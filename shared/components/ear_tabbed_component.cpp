#include "ear_tabbed_component.hpp"

#include "../helper/graphics.hpp"
#include "helper/move.hpp"
#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"

#include <cmath>

using namespace ear::plugin::ui;

EarTabbedComponent::EarTabbedComponent()
    : buttonBar_(std::make_unique<EarTabButtonBar>()),
      addTabButton_(std::make_unique<EarButton>()) {
  addTabButton_->setOffStateIcon(
      std::unique_ptr<Drawable>(Drawable::createFromImageData(
          binary_data::add_icon_svg, binary_data::add_icon_svgSize)));
  addTabButton_->onClick = [&]() {
    Component::BailOutChecker checker(this);
    listeners_.callChecked(checker,
                           [this](Listener& l) { l.addTabClicked(this); });
    if (checker.shouldBailOut()) {
      return;
    }
  };

  buttonBar_->onDoubleClick = [&]() {
    Component::BailOutChecker checker(this);
    listeners_.callChecked(
        checker, [this](Listener& l) { l.tabBarDoubleClicked(this); });
    if (checker.shouldBailOut()) {
      return;
    }
  };

  buttonBar_->setTabs(&tabs_);

  addAndMakeVisible(buttonBar_.get());
  addAndMakeVisible(addTabButton_.get());
}

void EarTabbedComponent::addTab(const String& name, Component* component,
                                bool select, bool scroll) {
  buttons_.push_back(std::make_unique<EarTabButton>(name));
  auto button = buttons_.back().get();
  tabs_.push_back(EarTab{button, component});

  button->addMouseListener(this, false);
  button->onClick = [this](EarTabButton* src) {
    selectTab(this->getIndexForTabButton(src));
  };
  button->onCloseClick = [this](EarTabButton* button) {
    auto index = getIndexForTabButton(button);
    Component::BailOutChecker checker(this);
    listeners_.callChecked(checker, [this, index](Listener& l) {
      l.removeTabClicked(this, index);
    });
    if (checker.shouldBailOut()) {
      return;
    }
  };
  buttonBar_->addButtonAndMakeVisible(button);
  buttonWidth_ = buttonBar_->updateTabBounds();
  component->setBounds(contentArea_);
  addChildComponent(component);
  if (select) {
    selectTab(tabs_.size() - 1);
  }
  if (scroll) {
    buttonBar_->scrollIfNecessaryTo(tabs_.size() - 1);
  }
}

void EarTabbedComponent::setTabName(int index, const String& name) {
  if (index < tabs_.size()) {
    if (auto button = tabs_.at(index).button) {
      button->setText(name);
    }
  }
}

String ear::plugin::ui::EarTabbedComponent::getTabName(int index)
{
    if(index >= 0 && index < tabs_.size()) {
      if(auto button = tabs_.at(index).button) {
          return button->getText();
      }
    }
    return String();
}

void EarTabbedComponent::selectTab(int index, bool scroll) {
  clearSelected();
  if (index < tabs_.size()) {
    tabs_[index].component->setVisible(true);
    tabs_[index].button->setSelected(true);
    selectedTabIndex_ = index;
    if (scroll) {
      buttonBar_->scrollIfNecessaryTo(index);
    }
  } else {
    selectedTabIndex_ = -1;
  }
  Component::BailOutChecker checker(this);
  listeners_.callChecked(
      checker, [this, index](Listener& l) { l.tabSelected(this, index); });
  if (checker.shouldBailOut()) {
    return;
  }
}

int EarTabbedComponent::getSelectedTabIndex() { return selectedTabIndex_; }

void EarTabbedComponent::moveTabTo(int oldIndex, int newIndex) {
  if (oldIndex < tabs_.size() && newIndex < tabs_.size() &&
      oldIndex != newIndex) {
    auto selectedButton = tabs_.at(selectedTabIndex_).button;
    move(tabs_.begin(), oldIndex, newIndex);
    selectedTabIndex_ = getIndexForTabButton(selectedButton);
  }
  Component::BailOutChecker checker(this);
  listeners_.callChecked(checker, [this, oldIndex, newIndex](Listener& l) {
    l.tabMoved(this, oldIndex, newIndex);
  });
  if (checker.shouldBailOut()) {
    return;
  }
}

void EarTabbedComponent::removeTab(int index) {
  if (index < tabs_.size()) {
    auto component = tabs_[index].component;
    removeChildComponent(tabs_[index].component);
    buttonBar_->removeButton(tabs_[index].button);
    tabs_.erase(tabs_.begin() + index);
    index = index < tabs_.size() ? index : index - 1;
    selectTab(index);
    buttonWidth_ = buttonBar_->updateTabBounds();
  }
}

void EarTabbedComponent::removeAllTabs() {
  for (int i = tabs_.size() - 1; i >= 0; --i) {
    removeTab(i);
  }
}

Component* EarTabbedComponent::getComponent(int index) {
  if (index < tabs_.size()) {
    return tabs_.at(index).component;
  }
  return nullptr;
}

void EarTabbedComponent::resized() {
  auto area = getLocalBounds();
  auto barArea = area.removeFromTop(buttonBarHeight);
  addTabButton_->setBounds(
      barArea.removeFromRight(buttonBarHeight).reduced(4, 4));
  buttonBar_->setBounds(barArea);
  buttonWidth_ = buttonBar_->updateTabBounds();
  contentArea_ = area;
  for (auto tab : tabs_) {
    tab.component->setBounds(contentArea_);
  }
}

void EarTabbedComponent::mouseDown(const MouseEvent& event) {
  tabDragger_.startDraggingComponent(event.eventComponent, event);
}

void EarTabbedComponent::mouseDrag(const MouseEvent& event) {
  tabDragger_.dragComponent(event.eventComponent, event,
                            &tabDraggerConstrainer_);
  int newButtonIndex = getButtonIndexForXPosition(event.eventComponent->getX());
  auto it = std::find_if(tabs_.begin(), tabs_.end(), [&](EarTab tab) {
    return tab.button == event.eventComponent;
  });
  int oldButtonIndex = std::distance(tabs_.begin(), it);
  if (newButtonIndex != oldButtonIndex) {
    moveTabTo(oldButtonIndex, newButtonIndex);
    buttonWidth_ = buttonBar_->updateTabBounds();
  }
}

void EarTabbedComponent::mouseUp(const MouseEvent& event) {
  buttonWidth_ = buttonBar_->updateTabBounds();
}

int EarTabbedComponent::getButtonIndexForXPosition(int xPos) {
  return std::min(
      static_cast<long>(std::floor((xPos / float(buttonWidth_)) + 0.5)),
      static_cast<long>(tabs_.size() - 1));
}

int EarTabbedComponent::getIndexForTabButton(EarTabButton* button) {
  auto it = std::find_if(tabs_.begin(), tabs_.end(), [button](EarTab& tab) {
    return tab.button == button;
  });
  if (it != tabs_.end()) {
    return std::distance(tabs_.begin(), it);
  }
  return -1;
}

int EarTabbedComponent::getIndexForComponent(Component* component) {
  auto it = std::find_if(tabs_.begin(), tabs_.end(), [component](auto tab) {
    return tab.component == component;
  });
  if (it != tabs_.end()) {
    return std::distance(tabs_.begin(), it);
  }
  return -1;
}

void EarTabbedComponent::clearSelected() {
  for (auto tab : tabs_) {
    tab.component->setVisible(false);
    tab.button->setSelected(false);
  }
}
