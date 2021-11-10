#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class SpeakerLevel : public Component {
 public:
  SpeakerLevel(String name)
      : levelMeter_(std::make_unique<LevelMeter>()),
        speakerLabel_(std::make_unique<Label>()) {
    levelMeter_->setOrientation(LevelMeter::vertical);
    addAndMakeVisible(levelMeter_.get());

    speakerLabel_->setText(name, dontSendNotification);
    speakerLabel_->setFont(EarFonts::Items);
    speakerLabel_->setColour(Label::textColourId, EarColours::Label);
    speakerLabel_->setJustificationType(Justification::centred);
    addAndMakeVisible(speakerLabel_.get());

  }
  ~SpeakerLevel() {}

  void paint(Graphics& g) override { g.fillAll(EarColours::Area01dp); }

  void resized() override {
    auto area = getLocalBounds();

    area.removeFromTop(10);
    auto meterHeight = area.getHeight() - 30;
    levelMeter_->setBounds(
        area.removeFromTop(meterHeight).withSizeKeepingCentre(13, meterHeight));
    speakerLabel_->setBounds(area);
  }

  LevelMeter* getLevelMeter() { return levelMeter_.get(); }

 private:
  std::unique_ptr<LevelMeter> levelMeter_;
  std::unique_ptr<Label> speakerLabel_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpeakerLevel)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
