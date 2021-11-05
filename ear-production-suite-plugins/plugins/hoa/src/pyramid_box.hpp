#pragma once

#include "JuceHeader.h"
//#include "value_box_order_display.hpp"

namespace ear {
namespace plugin {

class LevelMeterCalculator;

namespace ui {

class ValueBoxOrderDisplay;

class PyramidBox : public Component, private Timer {
 public:
  PyramidBox(std::weak_ptr<LevelMeterCalculator> levelMeterCalculator,ValueBoxOrderDisplay* valueBoxOrderDisplay,
      int channel,
    int routing/*,
      std::shared_ptr<ear::plugin::LevelMeterCalculator> levelMeterCalculator*/);
  ~PyramidBox();

  void paint(Graphics& g) override;

  void resized() override;

  void setBox();

  void timerCallback() override;

  bool getHasClipped();
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
  ValueBoxOrderDisplay* valueBoxOrderDisplay_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PyramidBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
