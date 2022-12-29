#include "order_display_box.hpp"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "order_box.hpp"

namespace {

std::string getOrdinal(int num) {
  const std::vector<std::string> suffixes = {"th", "st", "nd", "rd", "th"};
  return suffixes.at(std::min(num, 4));
}

}  // namespace

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
    g.setFont(EarFontsSingleton::instance().Label);
    g.setColour(EarColours::Text.withAlpha(Emphasis::high));
    g.setFont(EarFontsSingleton::instance().Heading);
    g.drawText("Please select a Higher Order Ambisonics type first",
               area.removeFromTop(25.f), Justification::left);
  }
}

void OrderDisplayBox::resized() { updateOrderBoxBounds(); }

void OrderDisplayBox::updateOrderBoxBounds() {
  auto area = getLocalBounds();

  for (auto& orderBox : orderBoxes_) {
    orderBox->setBounds(area.removeFromTop(35));
    area.removeFromTop(6);
  }
}

void OrderDisplayBox::addOrderBoxes(HoaAudioProcessor* p,
                                    ValueBoxOrderDisplay* valueBoxOrderDisplay,
                                    int orderCount) {
  for (int i = 0; i < orderCount; ++i) {
    orderBoxes_.push_back(std::make_unique<OrderBox>(
        p, valueBoxOrderDisplay, std::to_string(i) + getOrdinal(i), i,
        orderCount - 1));
    addAndMakeVisible(*orderBoxes_.back());
  }
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
