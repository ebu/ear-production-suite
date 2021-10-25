#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class LevelDisplayBox : public Component {
 public:
  LevelDisplayBox() {}
  ~LevelDisplayBox() {}

  void updateLevelMeterBounds() {  // This seems to be where we place the
                                    // channel gain boxes
    auto area = getLocalBounds();
    area.removeFromBottom(350);
    for (auto levelMeter : levelMeters_) {
      levelMeter->setBounds(area.removeFromLeft(50));
      area.removeFromLeft(6);
    }
  }

    void paint(Graphics& g) override {
    auto area = getLocalBounds();
    if (levelMeters_.empty()) {
      g.fillAll(EarColours::Area01dp);
      float lineThickness = 1.f;
      g.setColour(EarColours::Error);
      g.drawRect(
          area.toFloat().reduced(lineThickness / 2.f, lineThickness / 2.f),
          lineThickness);
      area.reduce(20.f, 30.f);

      g.setColour(EarColours::Text.withAlpha(Emphasis::medium));
      g.setFont(EarFonts::Label);
      g.drawText("Level Meters not available", area.removeFromTop(25.f),
                 Justification::left);
      g.setColour(EarColours::Text.withAlpha(Emphasis::high));
      g.setFont(EarFonts::Heading);
      g.drawText("Please select a speaker layout first",
                 area.removeFromTop(25.f), Justification::left);
    }
  }

  void resized() override { updateLevelMeterBounds(); }

  void addOrderBox(LevelMeter* levelMeter) {
    levelMeters_.push_back(levelMeter);
    addAndMakeVisible(levelMeter);
    updateLevelMeterBounds();
    repaint();
  }

  void removeAllOrderBoxes() {
    removeAllChildren();
    levelMeters_.clear();
    repaint();
  };


 private:
  std::vector<LevelMeter*> levelMeters_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelDisplayBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
