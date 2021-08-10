#pragma once
#include "JuceHeader.h"

#include "components/ear_slider.hpp"
#include "components/ear_slider_range.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class GainInteractionSettings : public Component {
 public:
  GainInteractionSettings()
      : gainLabel(std::make_unique<Label>()),
        gainSliderRange(std::make_unique<EarSliderRange>()),
        gainMinLabel(std::make_unique<Label>()),
        gainMinSlider(std::make_unique<EarSlider>()),
        gainMaxLabel(std::make_unique<Label>()),
        gainMaxSlider(std::make_unique<EarSlider>()) {
    gainLabel->setFont(EarFonts::Label);
    gainLabel->setColour(Label::textColourId, EarColours::Label);
    gainLabel->setText("Gain", dontSendNotification);
    gainLabel->setJustificationType(Justification::bottomRight);
    addAndMakeVisible(gainLabel.get());

    gainSliderRange->setTicks(std::vector<Tick>{
        {String::fromUTF8("–inf"), -100.f, Justification::left},
        {String::fromUTF8("–60"), -60.f, Justification::centred},
        {String::fromUTF8("–40"), -40.f, Justification::centred},
        {String::fromUTF8("–20"), -20.f, Justification::centred},
        {String::fromUTF8("–12"), -12.f, Justification::centred},
        {String::fromUTF8("–6"), -6.f, Justification::centred},
        {String::fromUTF8("0"), 0.f, Justification::centred},
        {String::fromUTF8("+6"), 6.f, Justification::centred},
        {String::fromUTF8("+12"), 12.f, Justification::centred},
        {String::fromUTF8("+20"), 20.f, Justification::right}});
    gainSliderRange->setRange(-100.f, 20.f);
    gainSliderRange->setLowerRange(-100.f, 0.f);
    gainSliderRange->setUpperRange(0.f, 20.f);
    gainSliderRange->setLowerDoubleClickReturnValue(0.f);
    gainSliderRange->setUpperDoubleClickReturnValue(0.f);
    gainSliderRange->setSkewFactorFromMidPoint(-20.f);
    addAndMakeVisible(gainSliderRange.get());

    gainMinLabel->setFont(EarFonts::MinMaxLabel);
    gainMinLabel->setColour(Label::textColourId, EarColours::Label);
    gainMinLabel->setText("Min", juce::NotificationType::dontSendNotification);
    gainMinLabel->setJustificationType(Justification::right);
    addAndMakeVisible(gainMinLabel.get());

    gainMinSlider->setSliderStyle(Slider::IncDecButtons);
    gainMinSlider->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
    gainMinSlider->setRange(-100.f, 0.f, 0.f);
    gainMinSlider->setNumDecimalPlacesToDisplay(1);
    gainMinSlider->setDoubleClickReturnValue(true, 0.f);
    gainMinSlider->setValueOrigin(-100.f);
    gainMinSlider->textFromValueFunction = [&](double val) {
      if (val <= -100.0) return String::fromUTF8("–inf");
      return String(val, 1).replaceCharacters(StringRef(String::fromUTF8("-")),
                                              StringRef(String::fromUTF8("–")));
    };
    gainMinSlider->valueFromTextFunction = [&](const String& text) {
      if (text == String::fromUTF8("–inf")) return -100.0;
      return text
          .replaceCharacters(StringRef(String::fromUTF8("–")),
                             StringRef(String::fromUTF8("-")))
          .retainCharacters("0123456789.,-+")
          .getDoubleValue();
    };
    addAndMakeVisible(gainMinSlider.get());

    gainMaxLabel->setFont(EarFonts::MinMaxLabel);
    gainMaxLabel->setColour(Label::textColourId, EarColours::Label);
    gainMaxLabel->setText("Max", juce::NotificationType::dontSendNotification);
    gainMaxLabel->setJustificationType(Justification::right);
    addAndMakeVisible(gainMaxLabel.get());

    gainMaxSlider->setSliderStyle(Slider::IncDecButtons);
    gainMaxSlider->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
    gainMaxSlider->setRange(0.f, 20.f, 0.f);
    gainMaxSlider->setNumDecimalPlacesToDisplay(1);
    gainMaxSlider->setDoubleClickReturnValue(true, 0.f);
    gainMaxSlider->setValueOrigin(-100.f);
    gainMaxSlider->textFromValueFunction = [&](double val) {
      if (val <= -100.0) return String::fromUTF8("–inf");
      return String(val, 1).replaceCharacters(StringRef(String::fromUTF8("-")),
                                              StringRef(String::fromUTF8("–")));
    };
    gainMaxSlider->valueFromTextFunction = [&](const String& text) {
      if (text == String::fromUTF8("–inf")) return -100.0;
      return text
          .replaceCharacters(StringRef(String::fromUTF8("–")),
                             StringRef(String::fromUTF8("-")))
          .retainCharacters("0123456789.,-+")
          .getDoubleValue();
    };

    addAndMakeVisible(gainMaxSlider.get());

    gainSliderRange->getLowerValueObject().referTo(
        gainMinSlider->getValueObject());
    gainSliderRange->getUpperValueObject().referTo(
        gainMaxSlider->getValueObject());
  }
  ~GainInteractionSettings() {}

  void resized() override {
    auto area = getLocalBounds();
    area.reduce(5, 5);
    auto gainArea = area.removeFromTop(sliderHeight_);
    gainLabel->setBounds(gainArea.removeFromLeft(labelWidth_)
                             .withTrimmedBottom(labelPaddingBottom_));
    auto gainMinMaxArea = gainArea.removeFromRight(80);

    auto gainMinArea =
        gainMinMaxArea.removeFromTop(sliderHeight_ / 2).withTrimmedBottom(1);
    gainMinLabel->setBounds(gainMinArea.removeFromLeft(30));
    gainMinSlider->setBounds(gainMinArea);

    auto gainMaxArea = gainMinMaxArea.withTrimmedTop(1);
    gainMaxLabel->setBounds(gainMaxArea.removeFromLeft(30));
    gainMaxSlider->setBounds(gainMaxArea);

    gainSliderRange->setBounds(gainArea);
  }

  std::unique_ptr<Label> gainLabel;
  std::unique_ptr<EarSliderRange> gainSliderRange;
  std::unique_ptr<Label> gainMinLabel;
  std::unique_ptr<EarSlider> gainMinSlider;
  std::unique_ptr<Label> gainMaxLabel;
  std::unique_ptr<EarSlider> gainMaxSlider;

 private:
  const float labelWidth_ = 71.f;
  const float labelPaddingBottom_ = 0.f;
  const float sliderHeight_ = 40.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainInteractionSettings)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
