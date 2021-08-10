#pragma once

#include "JuceHeader.h"

#include "../../shared/components/ear_slider.hpp"
#include "../../shared/components/ear_inverted_slider.hpp"
#include "../../shared/components/look_and_feel/colours.hpp"
#include "../../shared/components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ValueBoxPanning : public Component {
 public:
  ValueBoxPanning()
      : headingLabel_(std::make_unique<Label>()),
        azLabel_(std::make_unique<Label>()),
        azSlider_(std::make_shared<EarInvertedSlider>()),
        elLabel_(std::make_unique<Label>()),
        elSlider_(std::make_shared<EarSlider>()),
        distLabel_(std::make_unique<Label>()),
        distSlider_(std::make_shared<EarSlider>()) {
    setColour(backgroundColourId, EarColours::Area01dp);

    headingLabel_->setFont(EarFonts::Heading);
    headingLabel_->setColour(Label::textColourId, EarColours::Heading);
    headingLabel_->setText("Panning",
                           juce::NotificationType::dontSendNotification);
    headingLabel_->setJustificationType(Justification::bottomLeft);
    addAndMakeVisible(headingLabel_.get());

    azLabel_->setFont(EarFonts::Label);
    azLabel_->setColour(Label::textColourId, EarColours::Label);
    azLabel_->setText("Azimuth", juce::NotificationType::dontSendNotification);
    azLabel_->setJustificationType(Justification::bottomRight);
    addAndMakeVisible(azLabel_.get());
    elLabel_->setFont(EarFonts::Label);
    elLabel_->setColour(Label::textColourId, EarColours::Label);
    elLabel_->setText("Elevation",
                      juce::NotificationType::dontSendNotification);
    elLabel_->setJustificationType(Justification::bottomRight);
    addAndMakeVisible(elLabel_.get());
    distLabel_->setFont(EarFonts::Label);
    distLabel_->setColour(Label::textColourId, EarColours::Label);
    distLabel_->setText("Distance",
                        juce::NotificationType::dontSendNotification);
    distLabel_->setJustificationType(Justification::bottomRight);
    addAndMakeVisible(distLabel_.get());

    azSlider_->setTicks(std::vector<Tick>{
        {String::fromUTF8("–180"), -180.f, Justification::right},
        {String::fromUTF8("0"), 0.f, Justification::centred},
        {String::fromUTF8("+180"), 180.f, Justification::left}});
    azSlider_->setUnit("Degree");
    azSlider_->textFromValueFunction = [&](double val) {
      return String(val, 1).replaceCharacters(StringRef(String::fromUTF8("-")),
                                              StringRef(String::fromUTF8("–")));
    };
    azSlider_->valueFromTextFunction = [&](const String& text) {
      return text
          .replaceCharacters(StringRef(String::fromUTF8("–")),
                             StringRef(String::fromUTF8("-")))
          .retainCharacters("0123456789.,-+")
          .getDoubleValue();
    };
    azSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
    azSlider_->setRange(-180.f, 180.f);
    azSlider_->setNumDecimalPlacesToDisplay(1);
    azSlider_->setDoubleClickReturnValue(true, 0.f);
    addAndMakeVisible(azSlider_.get());

    elSlider_->setTicks(std::vector<Tick>{
        {String::fromUTF8("–90"), -90.f, Justification::left},
        {String::fromUTF8("0"), 0.f, Justification::centred},
        {String::fromUTF8("+90"), 90.f, Justification::right}});
    elSlider_->setUnit("Degree");
    elSlider_->textFromValueFunction = [&](double val) {
      return String(val, 1).replaceCharacters(StringRef(String::fromUTF8("-")),
                                              StringRef(String::fromUTF8("–")));
    };
    elSlider_->valueFromTextFunction = [&](const String& text) {
      return text
          .replaceCharacters(StringRef(String::fromUTF8("–")),
                             StringRef(String::fromUTF8("-")))
          .retainCharacters("0123456789.,-+")
          .getDoubleValue();
    };
    elSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
    elSlider_->setRange(-90.f, 90.f);
    elSlider_->setNumDecimalPlacesToDisplay(0);
    elSlider_->setDoubleClickReturnValue(true, 0.f);
    addAndMakeVisible(elSlider_.get());

    distSlider_->setTicks(std::vector<Tick>{
        {String::fromUTF8("0"), 0.f, Justification::left},
        {String::fromUTF8("0.5"), 0.5f, Justification::centred},
        {String::fromUTF8("1"), 1.f, Justification::right}});
    distSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
    distSlider_->setRange(0.f, 1.f);
    distSlider_->setNumDecimalPlacesToDisplay(2);
    distSlider_->setDoubleClickReturnValue(true, 1.f);
    distSlider_->setValue(1.f);
    addAndMakeVisible(distSlider_.get());
  }

  ~ValueBoxPanning() {}

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
    headingLabel_->setBounds(area.removeFromTop(30));

    area.removeFromTop(2.f * marginBig_);

    float sliderWidth = area.getWidth() - labelWidth_ - marginBig_;

    auto azArea = area.removeFromTop(rowHeight_);
    auto azLabelArea =
        azArea.withWidth(labelWidth_).withTrimmedBottom(labelPaddingBottom_);
    auto azSliderArea = azArea.withTrimmedLeft(labelWidth_ + marginBig_);
    azLabel_->setBounds(azLabelArea);
    azSlider_->setBounds(azSliderArea);

    area.removeFromTop(marginBig_);

    auto elArea = area.removeFromTop(rowHeight_);
    auto elLabelArea =
        elArea.withWidth(labelWidth_).withTrimmedBottom(labelPaddingBottom_);
    auto elSliderArea = elArea.withTrimmedLeft(labelWidth_ + marginBig_);
    elLabel_->setBounds(elLabelArea);
    elSlider_->setBounds(elSliderArea);

    area.removeFromTop(marginBig_);

    auto distArea = area.removeFromTop(rowHeight_);
    auto distLabelArea =
        distArea.withWidth(labelWidth_).withTrimmedBottom(labelPaddingBottom_);
    auto distSliderArea = distArea.withTrimmedLeft(labelWidth_ + marginBig_);
    distLabel_->setBounds(distLabelArea);
    distSlider_->setBounds(distSliderArea);
  }

  std::shared_ptr<EarInvertedSlider> getAzimuthSlider() { return azSlider_; }
  std::shared_ptr<EarSlider> getElevationSlider() { return elSlider_; }
  std::shared_ptr<EarSlider> getDistanceSlider() { return distSlider_; }

 private:
  std::unique_ptr<Label> headingLabel_;
  std::unique_ptr<Label> azLabel_;
  std::shared_ptr<EarInvertedSlider> azSlider_;
  std::unique_ptr<Label> elLabel_;
  std::shared_ptr<EarSlider> elSlider_;
  std::unique_ptr<Label> distLabel_;
  std::shared_ptr<EarSlider> distSlider_;

  const float labelWidth_ = 71.f;
  const float labelPaddingBottom_ = 0.f;
  const float rowHeight_ = 40.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxPanning)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
