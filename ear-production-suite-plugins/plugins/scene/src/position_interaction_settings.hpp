#pragma once
#include "JuceHeader.h"

#include "components/ear_slider.hpp"
#include "components/ear_slider_range.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class PositionInteractionSettings : public Component {
 public:
  PositionInteractionSettings()
      : azimuthLabel(std::make_unique<Label>()),
        azimuthSliderRange(std::make_unique<EarSliderRange>()),
        azimuthMinLabel(std::make_unique<Label>()),
        azimuthMinSlider(std::make_unique<EarSlider>()),
        azimuthMaxLabel(std::make_unique<Label>()),
        azimuthMaxSlider(std::make_unique<EarSlider>()),
        elevationLabel(std::make_unique<Label>()),
        elevationSliderRange(std::make_unique<EarSliderRange>()),
        elevationMinLabel(std::make_unique<Label>()),
        elevationMinSlider(std::make_unique<EarSlider>()),
        elevationMaxLabel(std::make_unique<Label>()),
        elevationMaxSlider(std::make_unique<EarSlider>()),
        distanceLabel(std::make_unique<Label>()),
        distanceSliderRange(std::make_unique<EarSliderRange>()),
        distanceMinLabel(std::make_unique<Label>()),
        distanceMinSlider(std::make_unique<EarSlider>()),
        distanceMaxLabel(std::make_unique<Label>()),
        distanceMaxSlider(std::make_unique<EarSlider>()) {
    azimuthLabel->setFont(EarFonts::Label);
    azimuthLabel->setColour(Label::textColourId, EarColours::Label);
    azimuthLabel->setText("Azimuth",
                          juce::NotificationType::dontSendNotification);
    azimuthLabel->setJustificationType(Justification::bottomRight);
    addAndMakeVisible(azimuthLabel.get());
    elevationLabel->setFont(EarFonts::Label);
    elevationLabel->setColour(Label::textColourId, EarColours::Label);
    elevationLabel->setText("Elevation",
                            juce::NotificationType::dontSendNotification);
    elevationLabel->setJustificationType(Justification::bottomRight);
    addAndMakeVisible(elevationLabel.get());
//    distanceLabel->setFont(EarFonts::Label);
//    distanceLabel->setColour(Label::textColourId, EarColours::Label);
//    distanceLabel->setText("Distance",
//                           juce::NotificationType::dontSendNotification);
//    distanceLabel->setJustificationType(Justification::bottomRight);
//    addAndMakeVisible(distanceLabel.get());

    azimuthSliderRange->setTicks(std::vector<Tick>{
        {String::fromUTF8("–180"), -180.f, Justification::left},
        {String::fromUTF8("0"), 0.f, Justification::centred},
        {String::fromUTF8("+180"), 180.f, Justification::right}});
    azimuthSliderRange->setRange(-180.f, 180.f);
    azimuthSliderRange->setLowerRange(-180.f, 0.f);
    azimuthSliderRange->setUpperRange(0.f, 180.f);
    azimuthSliderRange->setLowerDoubleClickReturnValue(-10.f);
    azimuthSliderRange->setUpperDoubleClickReturnValue(10.f);
    addAndMakeVisible(azimuthSliderRange.get());

    azimuthMinLabel->setFont(EarFonts::MinMaxLabel);
    azimuthMinLabel->setColour(Label::textColourId, EarColours::Label);
    azimuthMinLabel->setText("Min",
                             juce::NotificationType::dontSendNotification);
    azimuthMinLabel->setJustificationType(Justification::right);
    addAndMakeVisible(azimuthMinLabel.get());

    azimuthMinSlider->setSliderStyle(Slider::IncDecButtons);
    azimuthMinSlider->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
    azimuthMinSlider->setRange(-180.f, 0.f, 0.f);
    azimuthMinSlider->setNumDecimalPlacesToDisplay(1);
    azimuthMinSlider->setDoubleClickReturnValue(true, 0.f);
    addAndMakeVisible(azimuthMinSlider.get());

    azimuthMaxLabel->setFont(EarFonts::MinMaxLabel);
    azimuthMaxLabel->setColour(Label::textColourId, EarColours::Label);
    azimuthMaxLabel->setText("Max",
                             juce::NotificationType::dontSendNotification);
    azimuthMaxLabel->setJustificationType(Justification::right);
    addAndMakeVisible(azimuthMaxLabel.get());

    azimuthMaxSlider->setSliderStyle(Slider::IncDecButtons);
    azimuthMaxSlider->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
    azimuthMaxSlider->setRange(0.f, 180.f, 0.f);
    azimuthMaxSlider->setNumDecimalPlacesToDisplay(1);
    azimuthMaxSlider->setDoubleClickReturnValue(true, 0.f);
    addAndMakeVisible(azimuthMaxSlider.get());

    azimuthSliderRange->getLowerValueObject().referTo(
        azimuthMinSlider->getValueObject());
    azimuthSliderRange->getUpperValueObject().referTo(
        azimuthMaxSlider->getValueObject());

    elevationSliderRange->setTicks(std::vector<Tick>{
        {String::fromUTF8("–90"), -90.f, Justification::left},
        {String::fromUTF8("0"), 0.f, Justification::centred},
        {String::fromUTF8("+90"), 90.f, Justification::right}});
    elevationSliderRange->setRange(-90.f, 90.f);
    elevationSliderRange->setLowerRange(-90.f, 0.f);
    elevationSliderRange->setUpperRange(0.f, 90.f);
    elevationSliderRange->setLowerDoubleClickReturnValue(-10.f);
    elevationSliderRange->setUpperDoubleClickReturnValue(10.f);
    addAndMakeVisible(elevationSliderRange.get());

    elevationMinLabel->setFont(EarFonts::MinMaxLabel);
    elevationMinLabel->setColour(Label::textColourId, EarColours::Label);
    elevationMinLabel->setText("Min",
                               juce::NotificationType::dontSendNotification);
    elevationMinLabel->setJustificationType(Justification::right);
    addAndMakeVisible(elevationMinLabel.get());

    elevationMinSlider->setSliderStyle(Slider::IncDecButtons);
    elevationMinSlider->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
    elevationMinSlider->setRange(-90.f, 0.f, 0.f);
    elevationMinSlider->setNumDecimalPlacesToDisplay(1);
    elevationMinSlider->setDoubleClickReturnValue(true, 0.f);
    addAndMakeVisible(elevationMinSlider.get());

    elevationMaxLabel->setFont(EarFonts::MinMaxLabel);
    elevationMaxLabel->setColour(Label::textColourId, EarColours::Label);
    elevationMaxLabel->setText("Max",
                               juce::NotificationType::dontSendNotification);
    elevationMaxLabel->setJustificationType(Justification::right);
    addAndMakeVisible(elevationMaxLabel.get());

    elevationMaxSlider->setSliderStyle(Slider::IncDecButtons);
    elevationMaxSlider->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
    elevationMaxSlider->setRange(0.f, 90.f, 0.f);
    elevationMaxSlider->setNumDecimalPlacesToDisplay(1);
    elevationMaxSlider->setDoubleClickReturnValue(true, 0.f);
    addAndMakeVisible(elevationMaxSlider.get());

    elevationSliderRange->getLowerValueObject().referTo(
        elevationMinSlider->getValueObject());
    elevationSliderRange->getUpperValueObject().referTo(
        elevationMaxSlider->getValueObject());

//    distanceSliderRange->setTicks(std::vector<Tick>{
//        {String::fromUTF8("0"), 0.f, Justification::left},
//        {String::fromUTF8("0.5"), 0.5f, Justification::centred},
//        {String::fromUTF8("1"), 1.f, Justification::right}});
//    distanceSliderRange->setRange(0.f, 1.f);
//    distanceSliderRange->setLowerRange(0.f, 0.f);
//    distanceSliderRange->setUpperRange(0.f, 1.f);
//    distanceSliderRange->setLowerDoubleClickReturnValue(0.f);
//    distanceSliderRange->setUpperDoubleClickReturnValue(0.f);
//    distanceSliderRange->setLowerValue(0.f, dontSendNotification);
//    distanceSliderRange->setUpperValue(0.f, dontSendNotification);
//    addAndMakeVisible(distanceSliderRange.get());
//    distanceMinLabel->setFont(EarFonts::MinMaxLabel);
//    distanceMinLabel->setColour(Label::textColourId, EarColours::Label);
//    distanceMinLabel->setText("Min",
//                              juce::NotificationType::dontSendNotification);
//    distanceMinLabel->setJustificationType(Justification::right);
//    addAndMakeVisible(distanceMinLabel.get());
//
//    distanceMinSlider->setSliderStyle(Slider::IncDecButtons);
//    distanceMinSlider->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
//    distanceMinSlider->setRange(0.f, 0.f, 0.f);
//    distanceMinSlider->setNumDecimalPlacesToDisplay(1);
//    distanceMinSlider->setDoubleClickReturnValue(true, 0.f);
//    addAndMakeVisible(distanceMinSlider.get());
//
//    distanceMaxLabel->setFont(EarFonts::MinMaxLabel);
//    distanceMaxLabel->setColour(Label::textColourId, EarColours::Label);
//    distanceMaxLabel->setText("Max",
//                              juce::NotificationType::dontSendNotification);
//    distanceMaxLabel->setJustificationType(Justification::right);
//    addAndMakeVisible(distanceMaxLabel.get());
//
//    distanceMaxSlider->setSliderStyle(Slider::IncDecButtons);
//    distanceMaxSlider->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
//    distanceMaxSlider->setRange(0.f, 1.f, 0.f);
//    distanceMaxSlider->setNumDecimalPlacesToDisplay(1);
//    distanceMaxSlider->setDoubleClickReturnValue(true, 0.f);
//    addAndMakeVisible(distanceMaxSlider.get());
//
//    distanceSliderRange->getLowerValueObject().referTo(
//        distanceMinSlider->getValueObject());
//    distanceSliderRange->getUpperValueObject().referTo(
//        distanceMaxSlider->getValueObject());
  }
  ~PositionInteractionSettings() {}

  void resized() override {
    auto area = getLocalBounds();
    area.reduce(5, 5);

    // azimuth
    auto azimuthArea = area.removeFromTop(sliderHeight_);
    azimuthLabel->setBounds(azimuthArea.removeFromLeft(labelWidth_)
                                .withTrimmedBottom(labelPaddingBottom_));
    auto azimuthMinMaxArea = azimuthArea.removeFromRight(80);

    auto azimuthMinArea =
        azimuthMinMaxArea.removeFromTop(sliderHeight_ / 2).withTrimmedBottom(1);
    azimuthMinLabel->setBounds(azimuthMinArea.removeFromLeft(30));
    azimuthMinSlider->setBounds(azimuthMinArea);

    auto azimuthMaxArea = azimuthMinMaxArea.withTrimmedTop(1);
    azimuthMaxLabel->setBounds(azimuthMaxArea.removeFromLeft(30));
    azimuthMaxSlider->setBounds(azimuthMaxArea);

    azimuthSliderRange->setBounds(azimuthArea);

    area.removeFromTop(marginSmall_);

    // elevation
    auto elevationArea = area.removeFromTop(sliderHeight_);
    elevationLabel->setBounds(elevationArea.removeFromLeft(labelWidth_)
                                  .withTrimmedBottom(labelPaddingBottom_));
    auto elevationMinMaxArea = elevationArea.removeFromRight(80);

    auto elevationMinArea = elevationMinMaxArea.removeFromTop(sliderHeight_ / 2)
                                .withTrimmedBottom(1);
    elevationMinLabel->setBounds(elevationMinArea.removeFromLeft(30));
    elevationMinSlider->setBounds(elevationMinArea);

    auto elevationMaxArea = elevationMinMaxArea.withTrimmedTop(1);
    elevationMaxLabel->setBounds(elevationMaxArea.removeFromLeft(30));
    elevationMaxSlider->setBounds(elevationMaxArea);

    elevationSliderRange->setBounds(elevationArea);

    area.removeFromTop(marginSmall_);

    // distance
//    auto distanceArea = area.removeFromTop(sliderHeight_);
//    distanceLabel->setBounds(distanceArea.removeFromLeft(labelWidth_)
//                                 .withTrimmedBottom(labelPaddingBottom_));
//    auto distanceMinMaxArea = distanceArea.removeFromRight(80);
//
//    auto distanceMinArea = distanceMinMaxArea.removeFromTop(sliderHeight_ / 2)
//                               .withTrimmedBottom(1);
//    distanceMinLabel->setBounds(distanceMinArea.removeFromLeft(30));
//    distanceMinSlider->setBounds(distanceMinArea);
//
//    auto distanceMaxArea = distanceMinMaxArea.withTrimmedTop(1);
//    distanceMaxLabel->setBounds(distanceMaxArea.removeFromLeft(30));
//    distanceMaxSlider->setBounds(distanceMaxArea);
//
//    distanceSliderRange->setBounds(distanceArea);
  }

  std::unique_ptr<Label> azimuthLabel;
  std::unique_ptr<EarSliderRange> azimuthSliderRange;
  std::unique_ptr<Label> azimuthMinLabel;
  std::unique_ptr<EarSlider> azimuthMinSlider;
  std::unique_ptr<Label> azimuthMaxLabel;
  std::unique_ptr<EarSlider> azimuthMaxSlider;

  std::unique_ptr<Label> elevationLabel;
  std::unique_ptr<EarSliderRange> elevationSliderRange;
  std::unique_ptr<Label> elevationMinLabel;
  std::unique_ptr<EarSlider> elevationMinSlider;
  std::unique_ptr<Label> elevationMaxLabel;
  std::unique_ptr<EarSlider> elevationMaxSlider;

  std::unique_ptr<Label> distanceLabel;
  std::unique_ptr<EarSliderRange> distanceSliderRange;
  std::unique_ptr<Label> distanceMinLabel;
  std::unique_ptr<EarSlider> distanceMinSlider;
  std::unique_ptr<Label> distanceMaxLabel;
  std::unique_ptr<EarSlider> distanceMaxSlider;

 private:
  const float labelWidth_ = 71.f;
  const float labelPaddingBottom_ = 0.f;
  const float sliderHeight_ = 40.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionInteractionSettings)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
