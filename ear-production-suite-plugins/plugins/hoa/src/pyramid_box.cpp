#pragma once

#include "pyramid_box.hpp"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/level_meter_calculator.hpp"
#include "value_box_order_display.hpp"
#include "components/ear_button.hpp"

namespace ear {
namespace plugin {
namespace ui {

PyramidBox::PyramidBox(std::weak_ptr<LevelMeterCalculator> levelMeterCalculator,
                       ValueBoxOrderDisplay* valueBoxOrderDisplay, int channel)
    : channelLabel_(std::make_unique<Label>()),
      levelMeterCalculator_(levelMeterCalculator),
      valueBoxOrderDisplay_(valueBoxOrderDisplay),
      channel_(channel) {

  channelLabel_->setName("Label (PyramidBox::channelLabel_)");

  channelLabel_->setText(std::to_string(channel + 1), dontSendNotification);
  channelLabel_->setFont(EarFonts::Items);
  channelLabel_->setColour(Label::textColourId, EarColours::Label);
  channelLabel_->setJustificationType(Justification::centred);
  addAndMakeVisible(channelLabel_.get());

  setBox();
}
PyramidBox::~PyramidBox() {}

void PyramidBox::paint(Graphics& g) {
  auto levelMeterCalculatorLocked_ = levelMeterCalculator_.lock();
  if (channelHasClipped_) {
    g.setColour(EarColours::Error);
    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 4);
  } else if (!hasSignal_) {
    g.setColour(EarColours::Transparent);
    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 4);
    g.setColour(EarColours::Primary);
    g.drawRoundedRectangle(0, 0, getWidth(), getHeight(), 4, 1);
  } else {
    g.setColour(EarColours::Primary);
    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 4);
  }

  if(valueBoxOrderDisplay_){
    if(auto resetClippingButton = valueBoxOrderDisplay_->getResetClippingButton()) {
      resetClippingButton->setVisible(trackHasClipped_);
    }
  }
}

void PyramidBox::resized() {
  auto area = getLocalBounds();
  channelLabel_->setBounds(area);
}

void PyramidBox::setBox() {
  level_ = 0.f;
  hasSignal_ = false;
  trackHasClipped_ = false;
  channelHasClipped_ = false;
  if (!isTimerRunning()) startTimer(50);
}

void PyramidBox::timerCallback() {
  if (auto meter = levelMeterCalculator_.lock()) {
    meter->decayIfNeeded();
    level_ = meter->getLevel(channel_);
    hasSignal_ = meter->hasSignal(channel_);
    trackHasClipped_ = meter->thisTrackHasClipped();
    channelHasClipped_ = meter->thisChannelHasClipped(channel_);
    repaint();
  }
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
