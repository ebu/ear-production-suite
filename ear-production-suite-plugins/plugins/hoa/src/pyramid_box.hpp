#pragma once

#include "JuceHeader.h"

namespace ear {
namespace plugin {

class LevelMeterCalculator;

namespace ui {

class ValueBoxOrderDisplay;

class PyramidBox : public Component, private Timer {
 public:
  PyramidBox(std::weak_ptr<LevelMeterCalculator> levelMeterCalculator,ValueBoxOrderDisplay* valueBoxOrderDisplay,
      int channel,
    int routing);
  ~PyramidBox();

  void paint(Graphics& g) override;

  void resized() override;

  void setBox();

  void timerCallback() override;

  //bool getTrackHasClipped();

  //bool getChannelHasClipped();

 private:
  std::unique_ptr<Label> channelLabel_;
  std::weak_ptr<LevelMeterCalculator> levelMeterCalculator_;
  int channel_;
  int routing_;
  float level_;
  bool hasSignal_;
  bool channelHasClipped_;
  bool trackHasClipped_;
  ValueBoxOrderDisplay* valueBoxOrderDisplay_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PyramidBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
