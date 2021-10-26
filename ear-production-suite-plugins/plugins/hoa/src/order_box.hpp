#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "level_display_box.hpp"
#include "pyramid_box.hpp"
#include <math.h>

namespace ear {
namespace plugin {
namespace ui {

class OrderBox : public Component {
 public:
  OrderBox(String name, int rowOrder)
      : //levelMeter_(std::make_unique<LevelMeter>()),
        orderLabel_(std::make_unique<Label>()),
        levelDisplayBox_(std::make_unique<LevelDisplayBox>()),
        order_(rowOrder) {
    //levelMeter_->setOrientation(LevelMeter::vertical);
    //addAndMakeVisible(levelMeter_.get());

    orderLabel_->setText(name, dontSendNotification);
    orderLabel_->setFont(EarFonts::Items);
    orderLabel_->setColour(Label::textColourId, EarColours::Label);
    orderLabel_->setJustificationType(Justification::centred);
    addAndMakeVisible(orderLabel_.get());

    auto numChannelsOnRow = [](int value) { 
      return value < 0 ? 0 : value * 2 + 1; 
    };
    auto numChannelsInOrder = [](int value) {
      return pow(value + 1, 2);
    };

    for (int i(0); i < numChannelsOnRow(rowOrder); i++) {
      PyramidBox pyramidBox = PyramidBox(std::to_string(i + 1 +  numChannelsInOrder(rowOrder - 1)));
      pyramidBoxes_.push_back(&pyramidBox);
    }
  }
  ~OrderBox() {}

  void paint(Graphics& g) override { g.fillAll(EarColours::Area01dp); 
  }

  void resized() override {//Here we actually set the look of the level meter
    auto area = getLocalBounds();
    area.removeFromTop(5).removeFromBottom(100);//ME change
   // levelMeter_->setBounds(
        //area.removeFromTop(120).withSizeKeepingCentre(13, 120));
    orderLabel_->setBounds(area.removeFromLeft(40));
    levelDisplayBox_->setBounds(area.removeFromLeft(300));

    
    updatePyramidBoxBounds();

  }

  void updatePyramidBoxBounds() {  // This seems to be where we place the
                                    // channel gain boxes
    auto area = getLocalBounds();
    for (auto *pyramidBox : pyramidBoxes_) {
      area.removeFromLeft(getLocalBounds().getWidth() / pyramidBoxes_.size());
      pyramidBox->setBounds(area.removeFromLeft(50));
      area.removeFromLeft(6);
    }
  }

  //LevelMeter* getLevelMeter() { return levelMeter_.get(); }

  void addPyramidBox(PyramidBox* pyramidBox) {
    pyramidBoxes_.push_back(pyramidBox);
    addAndMakeVisible(pyramidBox);
    updatePyramidBoxBounds();
    repaint();
  }

  void removeAllOrderBoxes() {
    removeAllChildren();
    pyramidBoxes_.clear();
    repaint();
  };

 private:
  //std::unique_ptr<LevelMeter> levelMeter_;
  std::unique_ptr<Label> orderLabel_;
  int order_;
  std::unique_ptr<LevelDisplayBox> levelDisplayBox_;
  std::vector<PyramidBox*> pyramidBoxes_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrderBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
