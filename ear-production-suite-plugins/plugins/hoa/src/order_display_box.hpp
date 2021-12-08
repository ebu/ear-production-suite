#pragma once

#include "JuceHeader.h"

class HoaAudioProcessor;

namespace ear {
namespace plugin {
namespace ui {

class OrderBox;
class ValueBoxOrderDisplay;

class OrderDisplayBox : public Component {
 public:
  OrderDisplayBox();
  ~OrderDisplayBox();

  void paint(Graphics& g) override;

  void resized() override;

  void updateOrderBoxBounds();

  void addOrderBoxes(HoaAudioProcessor* p,
                     ValueBoxOrderDisplay* valueBoxOrderDisplay,
                     int orderCount);

  void removeAllOrderBoxes();

 private:
  std::vector<std::unique_ptr<OrderBox>> orderBoxes_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrderDisplayBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
