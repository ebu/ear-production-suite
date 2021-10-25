#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class PyramidBox : public Component {
 public:
  PyramidBox(std::string name): orderLabel_(std::make_unique<Label>()) {
    orderLabel_->setText(name, dontSendNotification);
    orderLabel_->setFont(EarFonts::Items);
    orderLabel_->setColour(Label::textColourId, EarColours::Label);
    orderLabel_->setJustificationType(Justification::centred);
    addAndMakeVisible(orderLabel_.get());
  }
  ~PyramidBox() {}

  void paint(Graphics& g) override { //g.fillAll(EarColours::Area01dp);
    g.fillAll(EarColours::Label);
  }

  void resized() override {//Here we actually set the look of the level meter
    auto area = getLocalBounds();
    area.removeFromTop(5).removeFromBottom(100);//ME change
  }

  //LevelMeter* getLevelMeter() { return levelMeter_.get(); }

 private:
  //std::unique_ptr<LevelMeter> levelMeter_;
  std::unique_ptr<Label> orderLabel_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PyramidBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
