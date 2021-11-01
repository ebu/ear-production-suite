#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "level_display_box.hpp"
#include "components/level_meter.hpp"
#include "pyramid_box.hpp"
#include <math.h>

namespace ear {
namespace plugin {
namespace ui {

class OrderBox : public Component {
 public:
  OrderBox(HoaAudioProcessor* p, String name, int rowOrder, int hoaOrder)
      : levelMeter_(std::make_unique<LevelMeter>()),
        orderLabel_(std::make_unique<Label>()),
        levelDisplayBox_(std::make_unique<LevelDisplayBox>()),
        rowOrder_(rowOrder),
        hoaOrder_(hoaOrder),
        p_(p)  {
    levelMeter_->setOrientation(LevelMeter::horizontal);
    addAndMakeVisible(levelMeter_.get());

    orderLabel_->setText(name, dontSendNotification);
    orderLabel_->setFont(EarFonts::Items);
    orderLabel_->setColour(Label::textColourId, EarColours::Label);
    orderLabel_->setJustificationType(Justification::centred);
    addAndMakeVisible(orderLabel_.get());

    addPyramidBoxesToOrderBox();
  }
  ~OrderBox() {}

  void paint(Graphics& g) override { g.fillAll(EarColours::Area01dp); 
  }

  void resized() override {//Here we actually set the look of the level meter
    auto area = getLocalBounds();

    orderLabel_->setBounds(area.removeFromLeft(40));
    //levelDisplayBox_->setBounds(area.removeFromLeft(200));
    levelMeter_->setBounds(area.removeFromLeft(250));

    area.removeFromBottom(5);
    area.removeFromTop(5);
    auto areaWidth = area.getWidth();

    int pyramidBoxWidth(40);
    int numberOfBoxPartitions = (hoaOrder_ * 2) + 2;
    int partitionNumber = (numberOfBoxPartitions / 2) - rowOrder_;

    for (auto pyramidBox : pyramidBoxes_) {
      auto removeFromLeft =
          (partitionNumber * areaWidth) / numberOfBoxPartitions;
      auto pyramidBoxArea = area.withTrimmedLeft(removeFromLeft);
      pyramidBox->setBounds(pyramidBoxArea.removeFromLeft(pyramidBoxWidth));
      partitionNumber++;
    }
  }

  //LevelMeter* getLevelMeter() { return levelMeter_.get(); }

  void addPyramidBoxesToOrderBox() {

    auto numChannelsOnRow = [](int value) { return value < 0 ? 0 : value * 2 + 1;};
    auto numChannelsInOrder = [](int value) { return pow(value + 1, 2); };

    pyramidBoxes_.reserve(numChannelsOnRow(rowOrder_));

    std::vector<int> routing;
    int routingFirstChannel(p_->getRouting()->get());

    for (int i(0); i < numChannelsOnRow(rowOrder_); i++) {
      std::shared_ptr<ear::plugin::ui::PyramidBox> pyramidBox =
          std::make_shared<PyramidBox>(
              std::to_string(i + 1 + static_cast<int>(numChannelsInOrder(rowOrder_ - 1))));
      pyramidBoxes_.push_back(pyramidBox);
      addAndMakeVisible(*pyramidBox);
      int test = numChannelsInOrder(rowOrder_ - 1);
      routing.push_back(routingFirstChannel + test + i + 1);
    }
    
    levelMeter_->setMeter(p_->getLevelMeter(), routing);

    //pyramidBoxes_.push_back(pyramidBox);
    
    //updatePyramidBoxBounds();
    //repaint();
  }

  void removeAllOrderBoxes() {
    removeAllChildren();
    pyramidBoxes_.clear();
    repaint();
  };

 private:
  HoaAudioProcessor* p_;
  std::unique_ptr<LevelMeter> levelMeter_;
  std::unique_ptr<Label> orderLabel_;
  int rowOrder_;
  int hoaOrder_;
  std::unique_ptr<LevelDisplayBox> levelDisplayBox_;
  std::vector<std::shared_ptr<ear::plugin::ui::PyramidBox>> pyramidBoxes_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrderBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
