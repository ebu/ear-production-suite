#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ChannelGain : public Component {
 public:
  ChannelGain(String name)
      : levelMeter_(std::make_unique<LevelMeter>()),
        speakerLabel_(std::make_unique<Label>()),
        gainSlider_(std::make_unique<EarSlider>()) {
    levelMeter_->setOrientation(LevelMeter::vertical);
    addAndMakeVisible(levelMeter_.get());

    speakerLabel_->setText(name, dontSendNotification);
    speakerLabel_->setFont(EarFonts::Items);
    speakerLabel_->setColour(Label::textColourId, EarColours::Label);
    speakerLabel_->setJustificationType(Justification::centred);
    addAndMakeVisible(speakerLabel_.get());

    gainSlider_->setTicks(std::vector<Tick>{
        {String::fromUTF8("–inf"), -100.f, Justification::bottomRight},
        {String::fromUTF8("–40"), -40.f, Justification::right},
        {String::fromUTF8("–20"), -20.f, Justification::right},
        {String::fromUTF8("–6"), -6.f, Justification::right},
        {String::fromUTF8("0"), 0.f, Justification::right},
        {String::fromUTF8("+6"), 6.f, Justification::topRight}});
    gainSlider_->setUnit("dB");
    gainSlider_->setValueOrigin(-100.f);
    gainSlider_->textFromValueFunction = [&](double val) {
      if (val <= -100.0) return String::fromUTF8("–inf");
      return String(val, 1).replaceCharacters(StringRef(String::fromUTF8("-")),
                                              StringRef(String::fromUTF8("–")));
    };
    gainSlider_->valueFromTextFunction = [&](const String& text) {
      if (text == String::fromUTF8("–inf")) return -100.0;
      return text
          .replaceCharacters(StringRef(String::fromUTF8("–")),
                             StringRef(String::fromUTF8("-")))
          .retainCharacters("0123456789.,-+")
          .getDoubleValue();
    };
  }
  ~ChannelGain() {}

  void paint(Graphics& g) override { g.fillAll(EarColours::Area01dp); }

  void resized() override {
    auto area = getLocalBounds();
    area.removeFromTop(5).removeFromBottom(100);
    levelMeter_->setBounds(
        area.removeFromTop(220).withSizeKeepingCentre(13, 220));
    speakerLabel_->setBounds(
        area.removeFromTop(30));
  }

  EarSlider* getGainSlider() { return gainSlider_.get(); }
  LevelMeter* getLevelMeter() { return levelMeter_.get(); }

 private:
  std::unique_ptr<LevelMeter> levelMeter_;
  std::unique_ptr<Label> speakerLabel_;
  std::unique_ptr<EarSlider> gainSlider_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelGain)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
