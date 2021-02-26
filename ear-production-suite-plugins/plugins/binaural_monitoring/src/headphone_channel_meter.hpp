#pragma once

#include "JuceHeader.h"

#include "components/level_meter.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class HeadphoneChannelMeter : public Component {
 public:
  HeadphoneChannelMeter(const String& index, const String& channelName)
      : index_(index),
        levelMeter_(std::make_unique<LevelMeter>()),
        channelName_(channelName) {
    setColour(backgroundColourId, EarColours::Area01dp);

    levelMeter_->setOrientation(LevelMeter::Orientation::vertical);
    addAndMakeVisible(levelMeter_.get());
  }

  ~HeadphoneChannelMeter() {}

  void paint(Graphics& g) override {
    g.fillAll(findColour(backgroundColourId));
    auto area = getLocalBounds();
    area.reduce(5, 5);
    g.setFont(EarFonts::Values);
    g.setColour(EarColours::Text.withAlpha(Emphasis::medium));
    g.drawText(index_, area.removeFromTop(30).reduced(5, 5),
               Justification::centredBottom, false);

    g.setFont(EarFonts::Label);
    g.setColour(EarColours::Text);
    g.drawText(channelName_, area.removeFromBottom(30), Justification::centred,
               false);
  }

  void resized() override {
    auto area = getLocalBounds();
    area = area.reduced(5, 5).withTrimmedTop(30).withTrimmedBottom(15 + 15);
    levelMeter_->setBounds(area.withSizeKeepingCentre(12, area.getHeight()));
  }

  LevelMeter* getLevelMeter() { return levelMeter_.get(); }

  enum ColourIds {
    backgroundColourId = 0xa6c8fb5e,
  };

 private:
  String index_;
  std::unique_ptr<LevelMeter> levelMeter_;
  String channelName_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeadphoneChannelMeter)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
