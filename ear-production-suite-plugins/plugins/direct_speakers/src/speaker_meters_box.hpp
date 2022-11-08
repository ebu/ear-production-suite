#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "speaker_level.hpp"

namespace ear {
namespace plugin {
namespace ui {

class SpeakerMetersBox : public Component {
 public:
  SpeakerMetersBox() {}
  ~SpeakerMetersBox() {}

  void paint(Graphics& g) override {
    auto area = getLocalBounds();
    if (speakerLevels_.empty()) {
      area.reduce(20.f, 30.f);

      g.setColour(EarColours::Text.withAlpha(Emphasis::medium));
      g.setFont(EarFontsSingleton::instance().Label);
      g.drawText("Channel meters not available", area.removeFromTop(25.f),
                 Justification::left);
      g.setColour(EarColours::Text.withAlpha(Emphasis::high));
      g.setFont(EarFontsSingleton::instance().Heading);
      g.drawText("Please select a speaker layout first",
                 area.removeFromTop(25.f), Justification::left);
    }
  }

  void resized() override { updateLevelMeterBounds(); }

  void updateLevelMeterBounds() {
    auto area = getLocalBounds();
    for (auto levelMeter : speakerLevels_) {
      levelMeter->setBounds(area.removeFromLeft(meterWidth));
      area.removeFromLeft(meterSpacing);
    }
  }

  void addLevelMeter(SpeakerLevel* levelMeter) {
    speakerLevels_.push_back(levelMeter);
    addAndMakeVisible(levelMeter);
    updateLevelMeterBounds();
    repaint();
  }

  void removeAllLevelMeters() {
    removeAllChildren();
    speakerLevels_.clear();
    repaint();
  };

 private:
  std::vector<SpeakerLevel*> speakerLevels_;

  static const int meterWidth{ 52 };
  static const int meterSpacing{ 5 };

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpeakerMetersBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
