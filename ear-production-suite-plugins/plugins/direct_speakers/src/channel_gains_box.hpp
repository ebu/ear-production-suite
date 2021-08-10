#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "channel_gain.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ChannelGainsBox : public Component {
 public:
  ChannelGainsBox() {}
  ~ChannelGainsBox() {}

  void paint(Graphics& g) override {
    auto area = getLocalBounds();
    if (channelGains_.empty()) {
      g.fillAll(EarColours::Area01dp);
      float lineThickness = 1.f;
      g.setColour(EarColours::Error);
      g.drawRect(
          area.toFloat().reduced(lineThickness / 2.f, lineThickness / 2.f),
          lineThickness);
      area.reduce(20.f, 30.f);

      g.setColour(EarColours::Text.withAlpha(Emphasis::medium));
      g.setFont(EarFonts::Label);
      g.drawText("Channel gains not available", area.removeFromTop(25.f),
                 Justification::left);
      g.setColour(EarColours::Text.withAlpha(Emphasis::high));
      g.setFont(EarFonts::Heading);
      g.drawText("Please select a speaker layout first",
                 area.removeFromTop(25.f), Justification::left);
    }
  }

  void resized() override { updateChannelGainBounds(); }

  void updateChannelGainBounds() {
    auto area = getLocalBounds();
    for (auto channelGain : channelGains_) {
      channelGain->setBounds(area.removeFromLeft(50));
      area.removeFromLeft(6);
    }
  }

  void addChannelGain(ChannelGain* channelGain) {
    channelGains_.push_back(channelGain);
    addAndMakeVisible(channelGain);
    updateChannelGainBounds();
    repaint();
  }

  void removeAllChannelGains() {
    removeAllChildren();
    channelGains_.clear();
    repaint();
  };

 private:
  std::vector<ChannelGain*> channelGains_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelGainsBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
