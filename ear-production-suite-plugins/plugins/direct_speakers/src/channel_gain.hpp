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
    gainSlider_->setSliderStyle(Slider::LinearVertical);
    gainSlider_->setTextBoxStyle(Slider::TextBoxAbove, false, 44, 40);
    gainSlider_->setRange(-100.f, 6.f);
    gainSlider_->setNumDecimalPlacesToDisplay(1);
    gainSlider_->setDoubleClickReturnValue(true, 0.f);
    gainSlider_->setSkewFactorFromMidPoint(-20.f);
    gainSlider_->setEnabled(false);
    gainSlider_->setAlpha(Emphasis::disabled);
  }
  ~ChannelGain() {}

  void paint(Graphics& g) override { g.fillAll(EarColours::Area01dp); }

  void resized() override {
    auto area = getLocalBounds();

    area.removeFromTop(10);
    auto meterHeight = area.getHeight() - 30;
    levelMeter_->setBounds(
        area.removeFromTop(meterHeight).withSizeKeepingCentre(13, meterHeight));
    speakerLabel_->setBounds(area);
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
