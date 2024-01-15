#pragma once

#include "JuceHeader.h"

#include "components/ear_combo_box.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class ValueBoxDataFile : public Component {
 public:
  ValueBoxDataFile(){
    label_.setFont(EarFontsSingleton::instance().Label);
    label_.setColour(Label::textColourId, EarColours::Label);
    label_.setText("BEAR Data File:",
                   juce::NotificationType::dontSendNotification);
    label_.setJustificationType(Justification::left);
    addAndMakeVisible(label_);

    comboBox_ = std::make_shared<EarComboBox>();
    comboBox_->setDefaultText("Select data file");
    comboBox_->setCanClearSelection(false);
    addAndMakeVisible(comboBox_.get());
  }

  void paint(Graphics& g) override { g.fillAll(EarColours::Area01dp); } 
  
  void resized() override {
    auto area = getLocalBounds();
    auto vPad = (area.getHeight() - rowHeight_) / 2;
    if (vPad < 0) vPad = 0;
    area.reduce(margin_, vPad);
    label_.setBounds(area.removeFromLeft(labelWidth_));
    comboBox_->setBounds(area);
  }

  std::shared_ptr<EarComboBox> getDataFileComboBox() { return comboBox_; }

 private:
  const float labelWidth_ = 115.f;
  const float rowHeight_ = 30.f;
  const float margin_ = 10.f;

  Label label_;
  std::shared_ptr<EarComboBox> comboBox_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxDataFile)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
