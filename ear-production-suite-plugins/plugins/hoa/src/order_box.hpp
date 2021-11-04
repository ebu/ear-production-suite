#pragma once

#include "JuceHeader.h"

#include "hoa_plugin_processor.hpp"
#include "pyramid_box.hpp"
#include "components/level_meter.hpp"


namespace ear {
namespace plugin {
namespace ui {

class OrderBox : public Component {
 public:
  OrderBox(HoaAudioProcessor* p, String name, int rowOrder, int hoaOrder);
  ~OrderBox();

  void paint(Graphics& g) override; 

  void resized() override;

  void addPyramidBoxesToOrderBox();

  void removeAllOrderBoxes();

 private:
  HoaAudioProcessor* p_;
  //ValueBoxOrderDisplay* valueBoxOrderDisplay_;
  std::unique_ptr<LevelMeter> levelMeter_;
  std::weak_ptr<ear::plugin::LevelMeterCalculator> levelMeterCalculator_;
  std::unique_ptr<Label> orderLabel_;
  int rowOrder_;
  int hoaOrder_;
  std::vector<std::shared_ptr<ear::plugin::ui::PyramidBox>> pyramidBoxes_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrderBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
