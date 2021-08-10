#pragma once

#include "JuceHeader.h"

#include "components/level_meter.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class SpeakerMeter : public Component {
 public:
  SpeakerMeter(const String& index, const String& speakerName,
               const String& speakerLabel)
      : index_(index),
        levelMeter_(std::make_unique<LevelMeter>()),
        speakerName_(speakerName),
        speakerLabel_(speakerLabel) {
    setColour(backgroundColourId, EarColours::Area01dp);

    levelMeter_->setOrientation(LevelMeter::Orientation::vertical);
    addAndMakeVisible(levelMeter_.get());
  }

  ~SpeakerMeter() {}

  void paint(Graphics& g) override {
    g.fillAll(findColour(backgroundColourId));
    auto area = getLocalBounds();
    area.reduce(5, 5);
    g.setFont(EarFonts::Values);
    g.setColour(EarColours::Text.withAlpha(Emphasis::medium));
    g.drawText(index_, area.removeFromTop(30).reduced(5, 5),
               Justification::centredBottom, false);

    g.setFont(EarFonts::Measures);
    g.setColour(EarColours::Text.withAlpha(Emphasis::high));
    g.drawText(speakerLabel_, area.removeFromBottom(15), Justification::centred,
               false);

    g.setFont(EarFonts::Label);
    g.setColour(EarColours::Text);
    g.drawText(speakerName_, area.removeFromBottom(30), Justification::centred,
               false);
  }

  void resized() override {
    auto area = getLocalBounds();
    area = area.reduced(5, 5).withTrimmedTop(30).withTrimmedBottom(15 + 30);
    levelMeter_->setBounds(area.withSizeKeepingCentre(12, area.getHeight()));
  }

  LevelMeter* getLevelMeter() { return levelMeter_.get(); }

  enum ColourIds {
    backgroundColourId = 0xa6c8fb5e,
  };

 private:
  String index_;
  std::unique_ptr<LevelMeter> levelMeter_;
  String speakerName_;
  String speakerLabel_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpeakerMeter)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
