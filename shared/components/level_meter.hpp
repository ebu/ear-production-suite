#pragma once

#include "JuceHeader.h"

#include "level_meter_calculator.hpp"
#include "look_and_feel/colours.hpp"

template <typename T>
T clamp(const T& n, const T& lower, const T& upper) {
  return std::max(lower, std::min(n, upper));
}

namespace ear {
namespace plugin {
namespace ui {

class LevelMeter : public Component, private Timer {
 public:
  LevelMeter() {
    setColour(backgroundColourId, EarColours::Transparent);
    setColour(outlineColorId, EarColours::Area06dp);
    setColour(highlightColourId, EarColours::Text.withAlpha(Emphasis::high));
    setColour(clippedColourId,
              EarColours::PrimaryHighlight.withAlpha(Emphasis::high));
  }

  enum Orientation { horizontal, vertical };

  void setOrientation(Orientation orientation) { orientation_ = orientation; }

  void setMeter(std::weak_ptr<ear::plugin::LevelMeterCalculator> calculator,
                int channel) {
    calculator_ = calculator;
    if (channels_.size() != 1 || channels_[0] != channel) {
      channels_ = {channel};
      values_ = {0.f};
      if (!isTimerRunning()) startTimer(50);
    }
  }

  void setMeter(std::weak_ptr<ear::plugin::LevelMeterCalculator> calculator,
                std::vector<int> channels) {
    calculator_ = calculator;
    if (channels_ != channels)
    {
      channels_ = channels;
      values_ = std::vector<float>(channels_.size(), 0.f);
      if(!isTimerRunning()) startTimer(50);
    }
  }

  void timerCallback() override {
    if (auto meter = calculator_.lock()) {
      meter->decayIfNeeded(60);
      for (int i = 0; i < channels_.size(); ++i) {
        values_.at(i) = meter->getLevel(channels_.at(i));
      }
      repaint();
    }
  }

  ~LevelMeter() {
    stopTimer();
  }

  void paint(Graphics& g) override {//Here we actually draw the rectangle
    //g.fillAll(findColour(backgroundColourId));
    g.fillAll(findColour(outlineColorId));//added temp
    g.setColour(findColour(outlineColorId));
    g.drawRect(getLocalBounds());

    g.setColour(findColour(highlightColourId));
    auto area = getLocalBounds().toFloat();
    area.reduce(outlineWidth_, outlineWidth_);
    float channelHeight =
        area.getHeight() / static_cast<float>(channels_.size());
    float channelWidth = area.getWidth() / static_cast<float>(channels_.size());
    for (int i = 0; i < channels_.size(); ++i) {
      float scalingFactor =
          std::pow(clamp<float>(values_.at(i), 0.f, 1.f), 0.3);
      if (orientation_ == Orientation::horizontal) {
          g.fillRect(area.removeFromTop(channelHeight).removeFromLeft(scalingFactor * area.getWidth()));
      } else {
          g.fillRect(area.removeFromLeft(channelWidth/5)
                  .removeFromBottom(scalingFactor * area.getHeight()));
      }
    }

    // draw outline
    g.setColour(findColour(outlineColorId));
    //g.drawRect(
      //  getLocalBounds().reduced(outlineWidth_ / 2.f, outlineWidth_ / 2.f),
        //outlineWidth_);
    
    
    
    //g.drawVerticalLine(getLocalBounds().reduced(outlineWidth_ / 2.f, outlineWidth_ / 2.f).getX(), getLocalBounds().reduced(outlineWidth_ / 2.f, outlineWidth_ / 2.f).getTopLeft().getY(), getLocalBounds().reduced(outlineWidth_ / 2.f, outlineWidth_ / 2.f).getBottomLeft().getY());
    //ME change but it doesn't work
    /*for (int i = 0; i < channels_.size(); ++i) {
        g.drawRect(
            getLocalBounds().reduced(outlineWidth_ + i/ 2.f, outlineWidth_ / 2.f),
            outlineWidth_*10);
    }*/
    //ME end
  }

  enum ColourIds {
    backgroundColourId = 0x00020001,
    outlineColorId = 0x00020002,
    highlightColourId = 0x00020003,
    clippedColourId = 0x00020004
  };

 private:
  std::vector<int> channels_;
  std::vector<float> values_;

  Orientation orientation_ = Orientation::horizontal;
  float outlineWidth_ = 1.f;
  std::weak_ptr<ear::plugin::LevelMeterCalculator> calculator_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};  // namespace ui

}  // namespace ui
}  // namespace plugin
}  // namespace ear
