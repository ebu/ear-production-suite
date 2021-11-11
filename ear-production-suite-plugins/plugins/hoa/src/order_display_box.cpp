#include "order_display_box.hpp"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "order_box.hpp"

namespace ear {
namespace plugin {
namespace ui {

OrderDisplayBox::OrderDisplayBox() {}
OrderDisplayBox::~OrderDisplayBox() {}

void OrderDisplayBox::paint(Graphics& g) {
  auto area = getLocalBounds();
  if (orderBoxes_.empty()) {
    g.fillAll(EarColours::Area01dp);
    float lineThickness = 1.f;
    g.setColour(EarColours::Error);
    g.drawRect(area.toFloat().reduced(lineThickness / 2.f, lineThickness / 2.f),
               lineThickness);
    area.reduce(20.f, 30.f);

    g.setColour(EarColours::Text.withAlpha(Emphasis::medium));
    g.setFont(EarFonts::Label);
    g.drawText("Order Boxes not available", area.removeFromTop(25.f),
               Justification::left);
    g.setColour(EarColours::Text.withAlpha(Emphasis::high));
    g.setFont(EarFonts::Heading);
    g.drawText("Please select a Higher Order Ambisonics type first",
               area.removeFromTop(25.f), Justification::left);
  }
}

void OrderDisplayBox::resized() { updateOrderBoxBounds(); }

void OrderDisplayBox::updateOrderBoxBounds() {
  auto area = getLocalBounds();

  for (auto orderBox : orderBoxes_) {
    orderBox->setBounds(area.removeFromTop(50));
    area.removeFromTop(6);
  }
}

void OrderDisplayBox::addOrderBox(OrderBox* orderBox) {
  orderBoxes_.push_back(orderBox);
  addAndMakeVisible(orderBox);
  updateOrderBoxBounds();
  repaint();
}

void OrderDisplayBox::removeAllOrderBoxes() {
  removeAllChildren();
  orderBoxes_.clear();
  repaint();
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
