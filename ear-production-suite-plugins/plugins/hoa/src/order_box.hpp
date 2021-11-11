#pragma once

#include "JuceHeader.h"

class HoaAudioProcessor;

namespace ear {
namespace plugin {

class LevelMeterCalculator;

namespace ui {

class PyramidBox;
class LevelMeter;
class ValueBoxOrderDisplay;

class OrderBox : public Component {
 public:
  OrderBox(HoaAudioProcessor* p,
           ValueBoxOrderDisplay* valueBoxOrderDisplay,
           String name,
           int rowOrder,
           int hoaOrder);
  ~OrderBox();

  void paint(Graphics& g) override; 

  void resized() override;

  void addPyramidBoxesToOrderBox();

 private:
  HoaAudioProcessor* p_;
  ValueBoxOrderDisplay* valueBoxOrderDisplay_;
  std::unique_ptr<LevelMeter> levelMeter_;
  std::unique_ptr<Label> orderLabel_;
  int rowOrder_;
  int hoaOrder_;
  std::vector<std::shared_ptr<ear::plugin::ui::PyramidBox>> pyramidBoxes_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrderBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
