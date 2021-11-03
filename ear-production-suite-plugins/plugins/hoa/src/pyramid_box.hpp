#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class PyramidBox : public Component, private Timer {
 public:
  PyramidBox(std::weak_ptr<LevelMeterCalculator> levelMeterCalculator,
      int channel,
    int routing/*,
      std::shared_ptr<ear::plugin::LevelMeterCalculator> levelMeterCalculator*/)
      : channelLabel_(std::make_unique<Label>()), levelMeterCalculator_(levelMeterCalculator), channel_(channel), routing_(routing)
        /*levelMeterCalculator_(levelMeterCalculator),*/
  {
    channelLabel_->setText(std::to_string(channel), dontSendNotification);
    channelLabel_->setFont(EarFonts::Items);
    channelLabel_->setColour(Label::textColourId, EarColours::Label);
    channelLabel_->setJustificationType(Justification::centred);
    addAndMakeVisible(channelLabel_.get());

    setBox();
  }
  ~PyramidBox() {}

  void paint(Graphics& g) override { //g.fillAll(EarColours::Area01dp);    
    auto levelMeterCalculatorLocked_ = levelMeterCalculator_.lock();    
    if (hasClipped_) {
      g.fillAll(EarColours::Error);
    } else if (!hasSignal_) {
    //if (value_ < 0.00005 && value_ > -0.00005) {
      g.fillAll(EarColours::Transparent);
      g.setColour(EarColours::Primary);
      g.drawRect(getLocalBounds(), 1);
    } else {
      g.fillAll(EarColours::Primary);
    }
  }

  void resized() override {//Here we actually set the look of the level meter
   auto area = getLocalBounds();
    //area.removeFromTop(5).removeFromBottom(100);//ME change
    channelLabel_->setBounds(area);
  }

  void setBox() {
    level_ = 0.f;
    hasSignal_ = false;
    hasClipped_ = false;
    if (!isTimerRunning()) startTimer(50);
  }

  void timerCallback() override {
    if (auto meter = levelMeterCalculator_.lock()) {
      meter->decayIfNeeded(60);
      level_ = meter->getLevel(routing_);
      hasSignal_ = meter->hasSignal(routing_);
      hasClipped_ = meter->hasClipped();
      repaint();
    }
  }

  //LevelMeter* getLevelMeter() { return levelMeter_.get(); }

 private:
  //std::unique_ptr<LevelMeter> levelMeter_;
  std::unique_ptr<Label> channelLabel_;
  std::weak_ptr<LevelMeterCalculator> levelMeterCalculator_;
  //std::shared_ptr<ear::plugin::LevelMeterCalculator> levelMeterCalculator_;
  int channel_;
  int routing_;
  float level_;
  bool hasSignal_;
  bool hasClipped_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PyramidBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
