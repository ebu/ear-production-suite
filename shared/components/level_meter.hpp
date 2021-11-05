#pragma once

#include "JuceHeader.h"

#include "level_meter_calculator.hpp"
#include "look_and_feel/colours.hpp"
#include <numeric>

template <typename T>
T clamp(const T& n, const T& lower, const T& upper) {
  return std::max(lower, std::min(n, upper));
}

namespace ear {
namespace plugin {
namespace ui {

class LevelMeter : public Component, private Timer {
 public:
  LevelMeter(): averageEnabled_(false) {
    setColour(backgroundColourId, EarColours::Transparent);
    setColour(outlineColorId, EarColours::Area06dp);
    setColour(highlightColourId, EarColours::Text.withAlpha(Emphasis::high));
    setColour(clippedColourId,
              EarColours::PrimaryHighlight.withAlpha(Emphasis::high));

    //averageEnabled_ = false;
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

  std::vector<float> getValues() { return values_; }//ME temporary add

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

    g.setColour(findColour(outlineColorId));

    if (averageEnabled_) {
        auto count = static_cast<float>(values_.size());
        float average = std::reduce(values_.begin(), values_.end()) / count;
        float scalingFactorAverage = std::pow(clamp<float>(average, 0.f, 1.f), 0.3);
        int averageLevel = static_cast<int>(scalingFactorAverage * area.getWidth());
        g.setColour(EarColours::Primary);
        g.setOpacity(1);
        g.fillRect(averageLevel,0, 5,
            getLocalBounds().getHeight());
    }
  }
  
  void enableAverage(bool averageEnabled) {
      averageEnabled_ = averageEnabled;
      repaint();
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
  bool averageEnabled_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};  // namespace ui

}  // namespace ui
}  // namespace plugin
}  // namespace ear
