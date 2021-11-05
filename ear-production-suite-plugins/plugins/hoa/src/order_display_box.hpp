#pragma once

#include "JuceHeader.h"

namespace ear {
namespace plugin {
namespace ui {

class OrderBox;

class OrderDisplayBox : public Component {
 public:
  OrderDisplayBox();
  ~OrderDisplayBox();

  void paint(Graphics& g) override;

  void resized() override;

  void updateOrderBoxBounds();

  void addOrderBox(OrderBox* orderBox);

  void removeAllOrderBoxes();

 private:
  std::vector<OrderBox*> orderBoxes_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrderDisplayBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
