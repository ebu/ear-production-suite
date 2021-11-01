#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "order_box.hpp"

namespace ear {
namespace plugin {
namespace ui {

class OrderDisplayBox : public Component {
 public:
  OrderDisplayBox(HoaAudioProcessor* p) {}
  ~OrderDisplayBox() {}

  void paint(Graphics& g) override {
    auto area = getLocalBounds();
    if (orderBoxes_.empty()) {
      g.fillAll(EarColours::Area01dp);
      float lineThickness = 1.f;
      g.setColour(EarColours::Error);
      g.drawRect(
          area.toFloat().reduced(lineThickness / 2.f, lineThickness / 2.f),
          lineThickness);
      area.reduce(20.f, 30.f);

      g.setColour(EarColours::Text.withAlpha(Emphasis::medium));
      g.setFont(EarFonts::Label);
      g.drawText("Order Boxes not available", area.removeFromTop(25.f),
                 Justification::left);
      g.setColour(EarColours::Text.withAlpha(Emphasis::high));
      g.setFont(EarFonts::Heading);
      g.drawText("Please select a speaker layout first",
                 area.removeFromTop(25.f), Justification::left);
    }
  }

  void resized() override { updateOrderBoxBounds(); }

  void updateOrderBoxBounds() {  // This seems to be where we place the channel
                                 // gain boxes
    auto area = getLocalBounds();

    //ME add
    /*
    int levels(ceil(sqrt(channelGains_.size())));
    double levelMeterSize(area.getHeight() /
                          static_cast<double>(levels));  // ME experiment
    //double middleWidth = area.getWidth() / 2;
    // for (auto channelGain : channelGains_) {
    
    for (int i(0); i < channelGains_.size(); i++) {  // ME experiment
      int level(ceil(sqrt(i + 1)));
      auto channelGain(channelGains_[i]);  // ME experiment
      channelGain->setBounds(
          area.withLeft(50 * (i - pow(level - 1, 2)) )
              .withRight((50 * (i - pow(level - 1, 2)))+ 50)
              .withTrimmedTop((level - 1) * levelMeterSize)
              .withTrimmedBottom((levels - level) * levelMeterSize));
      area.removeFromLeft(6);  // NEEDS WORK
    }
    */
      //ME end - Remember to comment bit below out if using this
     //area.removeFromBottom(350);
    for (auto orderBox : orderBoxes_) {
       orderBox->setBounds(area.removeFromTop(50));
      area.removeFromTop(6);
    }
  }

  void addOrderBox(OrderBox* orderBox) {
    orderBoxes_.push_back(orderBox);
    addAndMakeVisible(orderBox);
    updateOrderBoxBounds();
    //orderBox->addPyramidBoxesToOrderBox();
    repaint();
  }

  void removeAllOrderBoxes() {
    removeAllChildren();
    orderBoxes_.clear();
    repaint();
  };

 private:
  std::vector<OrderBox*> orderBoxes_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrderDisplayBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
