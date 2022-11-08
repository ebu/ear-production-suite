#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class SpeakerMeterBox : public Component {
 public:
  SpeakerMeterBox(const String& labelText) : label_(std::make_unique<Label>()) {
    setColour(backgroundColourId, EarColours::Area01dp);

    label_->setFont(EarFontsSingleton::instance().Label);
    label_->setColour(Label::textColourId,
                      EarColours::Text.withAlpha(Emphasis::high));
    label_->setText(labelText, dontSendNotification);
    label_->setJustificationType(Justification::left);
    addAndMakeVisible(label_.get());
  }

  ~SpeakerMeterBox() {}

  void paint(Graphics& g) override {
    g.fillAll(findColour(backgroundColourId));
  }

  void resized() override {
    auto area = getLocalBounds();
    area.reduce(12, 12);
    label_->setBounds(area.removeFromTop(16));
  }

  enum ColourIds {
    backgroundColourId = 0xdc7e4ba3,
  };

 private:
  std::unique_ptr<Label> label_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpeakerMeterBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
