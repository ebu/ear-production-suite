#pragma once

#include "pyramid_box.hpp"

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/level_meter_calculator.hpp"
#include "value_box_order_display.hpp"
#include "components/ear_button.hpp"

namespace ear {
namespace plugin {
namespace ui {


PyramidBox::PyramidBox(std::weak_ptr<LevelMeterCalculator> levelMeterCalculator,
    ValueBoxOrderDisplay* valueBoxOrderDisplay,
      int channel,
    int routing)
      : channelLabel_(std::make_unique<Label>()), levelMeterCalculator_(levelMeterCalculator), valueBoxOrderDisplay_(valueBoxOrderDisplay), channel_(channel), routing_(routing)
  {
    channelLabel_->setText(std::to_string(channel), dontSendNotification);
    channelLabel_->setFont(EarFonts::Items);
    channelLabel_->setColour(Label::textColourId, EarColours::Label);
    channelLabel_->setJustificationType(Justification::centred);
    addAndMakeVisible(channelLabel_.get());

    setBox();
  }
PyramidBox::~PyramidBox() {}

void PyramidBox::paint(Graphics& g) {
    auto levelMeterCalculatorLocked_ = levelMeterCalculator_.lock();
    if (hasClipped_) {
      g.fillAll(EarColours::Error);
      g.drawRect(getLocalBounds(), 1);
      if (!valueBoxOrderDisplay_->getResetClippingButton()->isVisible()) {
        valueBoxOrderDisplay_->getResetClippingButton().get()->setVisible(true);
      }
    } else if (!hasSignal_) {
      g.fillAll(EarColours::Transparent);
      g.setColour(EarColours::Primary);
      g.drawRect(getLocalBounds(), 1);
      if (valueBoxOrderDisplay_->getResetClippingButton()->isVisible()) {
        valueBoxOrderDisplay_->getResetClippingButton().get()->setVisible(false);
      }
    } else {
      g.fillAll(EarColours::Primary);
      g.drawRect(getLocalBounds(), 1);
      if (valueBoxOrderDisplay_->getResetClippingButton()->isVisible()) {
        valueBoxOrderDisplay_->getResetClippingButton().get()->setVisible(
            false);
      }
    }
}

void PyramidBox::resized() {  // Here we actually set the look of the level meter
    auto area = getLocalBounds();
    channelLabel_->setBounds(area);
  }

void PyramidBox::setBox() {
    level_ = 0.f;
    hasSignal_ = false;
    hasClipped_ = false;
    if (!isTimerRunning()) startTimer(50);
  }

void PyramidBox::timerCallback() {
    if (auto meter = levelMeterCalculator_.lock()) {
      meter->decayIfNeeded(60);
      level_ = meter->getLevel(routing_);
      hasSignal_ = meter->hasSignal(routing_);
      hasClipped_ = meter->hasClipped();
      repaint();
    }
}

bool PyramidBox::getHasClipped() { return hasClipped_; }


}  // namespace ui
}  // namespace plugin
}  // namespace ear
