#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class HeadphoneChannelMeterBox : public Component {
 public:
   HeadphoneChannelMeterBox(const String& labelText) : label_(std::make_unique<Label>()) {
    setColour(backgroundColourId, EarColours::Area01dp);

    label_->setFont(EarFontsSingleton::instance().Heading);
    label_->setColour(Label::textColourId,
                      EarColours::Text.withAlpha(Emphasis::high));
    label_->setText(labelText, dontSendNotification);
    label_->setJustificationType(Justification::bottomLeft);
    addAndMakeVisible(label_.get());
  }

  ~HeadphoneChannelMeterBox() {}

  void paint(Graphics& g) override {
    g.fillAll(findColour(backgroundColourId));
  }

  void resized() override {
    auto area = getLocalBounds();
    area.reduce(10, 5);
    label_->setBounds(area.removeFromTop(30));
  }

  enum ColourIds {
    backgroundColourId = 0xdc7e4ba3,
  };

 private:
  std::unique_ptr<Label> label_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeadphoneChannelMeterBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
