#pragma once

#include "JuceHeader.h"

#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class EarExpansionPanel : public Component {
 public:
  EarExpansionPanel()
      : contentHeight_(0),
        desiredHeight_(0),
        headingButton_(std::make_unique<EarButton>()),
        component_(nullptr) {
    headingButton_->setColour(EarButton::textColourId, EarColours::Label);
    headingButton_->setFont(EarFontsSingleton::instance().Values);
    headingButton_->setShape(EarButton::Shape::Toggle);
    headingButton_->setClickingTogglesState(true);
    headingButton_->onClick = [this]() {
      if (this->onClick) {
        this->onClick();
      }
    };
    headingButton_->onStateChange = [this]() {
      if (this->onStateChange) {
        this->onStateChange();
      }
      this->updateDesiredHeight();
    };
    addAndMakeVisible(headingButton_.get());
    updateDesiredHeight();
  }
  ~EarExpansionPanel() {}

  void setHeading(const String& heading) {
    headingButton_->setButtonText(heading);
  }

  void setComponent(Component* component) {
    removeChildComponent(component_);
    component_ = component;
    addAndMakeVisible(component_);
  }

  void setContentHeight(int height) {
    contentHeight_ = height;
    updateDesiredHeight();
  }

  int getDesiredHeight() { return desiredHeight_; }

  void setExpanded(bool expanded) {
    headingButton_->setToggleState(expanded, dontSendNotification);
    updateDesiredHeight();
  }
  bool isExpanded() { return headingButton_->getToggleState(); }

  void paint(Graphics& g) override {
    auto area = getLocalBounds();
    g.setColour(EarColours::Area01dp);
    g.fillRect(area.removeFromTop(headerHeight_));
    g.fillRect(area.removeFromLeft(borderWidth_));
    g.fillRect(area.removeFromRight(borderWidth_));
    g.fillRect(area.removeFromBottom(borderWidth_));
  }

  void resized() override {
    auto area = getLocalBounds();
    headingButton_->setBounds(
        area.removeFromTop(headerHeight_).withTrimmedLeft(5));
    if (component_) {
      component_->setBounds(area.removeFromTop(contentHeight_)
                                .withTrimmedLeft(borderWidth_)
                                .withTrimmedRight(borderWidth_)
                                .withTrimmedBottom(borderWidth_));
    }
  }

  std::function<void()> onClick;
  std::function<void()> onStateChange;

 private:
  void updateDesiredHeight() {
    desiredHeight_ = headerHeight_;
    if (headingButton_->getToggleState()) {
      if (component_) {
        desiredHeight_ += contentHeight_;
        desiredHeight_ += borderWidth_;
      }
    }
    if (auto parent = getParentComponent()) {
      parent->resized();
    }
  }

  const int headerHeight_ = 25;
  const int borderWidth_ = 1;

  int contentHeight_;
  int desiredHeight_;

  std::unique_ptr<EarButton> headingButton_;
  Component* component_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarExpansionPanel)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
