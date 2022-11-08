#pragma once

#include "JuceHeader.h"

#include "components/ear_slider.hpp"
#include "components/level_meter.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/look_and_feel/slider.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ValueBoxGain : public Component {
 public:
  ValueBoxGain()
      : levelMeter_(std::make_shared<LevelMeter>()),
        gainLabel_(std::make_unique<Label>()),
        gainSlider_(std::make_shared<EarSlider>()) {
    setColour(backgroundColourId, EarColours::Area01dp);
    addAndMakeVisible(levelMeter_.get());

    gainLabel_->setFont(EarFontsSingleton::instance().Label);
    gainLabel_->setColour(Label::textColourId, EarColours::Label);
    gainLabel_->setText("Gain", dontSendNotification);
    gainLabel_->setJustificationType(Justification::bottomRight);
    addAndMakeVisible(gainLabel_.get());

    gainSlider_->setTicks(std::vector<Tick>{
        {String::fromUTF8("–inf"), -100.f, Justification::left},
        {String::fromUTF8("–40"), -40.f, Justification::centred},
        {String::fromUTF8("–20"), -20.f, Justification::centred},
        {String::fromUTF8("–6"), -6.f, Justification::centred},
        {String::fromUTF8("0"), 0.f, Justification::centred},
        {String::fromUTF8("+6"), 6.f, Justification::right}});
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
    gainSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
    gainSlider_->setRange(-100.f, 6.f);
    gainSlider_->setNumDecimalPlacesToDisplay(1);
    gainSlider_->setDoubleClickReturnValue(true, 0.f);
    gainSlider_->setSkewFactorFromMidPoint(-20.f);
    addAndMakeVisible(gainSlider_.get());
  }

  ~ValueBoxGain() {}

  void paint(Graphics& g) override {
    // background
    g.fillAll(findColour(backgroundColourId));
  }

  enum ColourIds {
    backgroundColourId = 0x00010001,
  };

  void resized() override {
    auto area = getLocalBounds();
    area.reduce(10, 5);
    levelMeter_->setBounds(area.removeFromTop(33).reduced(0, 10));

    float sliderWidth = area.getWidth() - labelWidth_ - marginBig_;

    auto gainArea = area.removeFromTop(sliderHeight_);
    auto gainLabelArea =
        gainArea.withWidth(labelWidth_).withTrimmedBottom(labelPaddingBottom_);
    auto gainSliderArea = gainArea.withTrimmedLeft(labelWidth_);
    gainLabel_->setBounds(gainLabelArea);
    gainSlider_->setBounds(gainSliderArea);
  }

  std::shared_ptr<EarSlider> getGainSlider() { return gainSlider_; }
  std::shared_ptr<LevelMeter> getLevelMeter() { return levelMeter_; }

 private:
  std::shared_ptr<LevelMeter> levelMeter_;
  std::unique_ptr<Label> gainLabel_;
  std::shared_ptr<EarSlider> gainSlider_;

  const float labelWidth_ = 71.f;
  const float labelPaddingBottom_ = 0.f;
  const float sliderHeight_ = 40.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxGain)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
