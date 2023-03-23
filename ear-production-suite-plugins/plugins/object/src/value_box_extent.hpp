#pragma once

#include "JuceHeader.h"

#include "components/ear_button.hpp"
#include "components/ear_slider.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ValueBoxExtent : public Component {
 public:
  ValueBoxExtent()
      : extentLabel_(std::make_unique<Label>()),
        linkSizeButton_(std::make_shared<EarButton>()),
        sizeLabel_(std::make_shared<Label>()),
        sizeSlider_(std::make_shared<EarSlider>()),
        widthLabel_(std::make_shared<Label>()),
        widthSlider_(std::make_shared<EarSlider>()),
        heightLabel_(std::make_shared<Label>()),
        heightSlider_(std::make_shared<EarSlider>()),
        depthLabel_(std::make_shared<Label>()),
        depthSlider_(std::make_shared<EarSlider>()),
        diffuseLabel_(std::make_unique<Label>()),
        diffuseSlider_(std::make_shared<EarSlider>()),
        divergenceButton_(std::make_shared<EarButton>()),
        factorLabel_(std::make_shared<Label>()),
        factorSlider_(std::make_shared<EarSlider>()),
        rangeLabel_(std::make_shared<Label>()),
        rangeSlider_(std::make_shared<EarSlider>()) {
    setColour(backgroundColourId, EarColours::Area01dp);

    extentLabel_->setFont(EarFontsSingleton::instance().Heading);
    extentLabel_->setColour(Label::textColourId, EarColours::Heading);
    extentLabel_->setText("Extent",
                          juce::NotificationType::dontSendNotification);
    extentLabel_->setJustificationType(Justification::bottomLeft);
    addAndMakeVisible(extentLabel_.get());

    linkSizeButton_->setOffStateIcon(
        std::unique_ptr<Drawable>(Drawable::createFromImageData(
            binary_data::link_icon_svg, binary_data::link_icon_svgSize)));
    linkSizeButton_->setOnStateIcon(std::unique_ptr<Drawable>(
        Drawable::createFromImageData(binary_data::link_off_icon_svg,
                                      binary_data::link_off_icon_svgSize)));
    addAndMakeVisible(linkSizeButton_.get());

    sizeLabel_->setFont(EarFontsSingleton::instance().Label);
    sizeLabel_->setColour(Label::textColourId, EarColours::Label);
    sizeLabel_->setText("Size", juce::NotificationType::dontSendNotification);
    sizeLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(sizeLabel_.get());
    sizeSlider_->setTicks(std::vector<Tick>{
        {String::fromUTF8("0"), 0.f, Justification::left},
        {String::fromUTF8("0.5"), 0.5f, Justification::centred},
        {String::fromUTF8("1"), 1.f, Justification::right}});
    sizeSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 44, rowHeight_);
    sizeSlider_->setRange(0.f, 1.f);
    sizeSlider_->setDoubleClickReturnValue(true, 0.f);
    sizeSlider_->setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(sizeSlider_.get());

    widthLabel_->setFont(EarFontsSingleton::instance().Label);
    widthLabel_->setColour(Label::textColourId, EarColours::Label);
    widthLabel_->setText("Width", juce::NotificationType::dontSendNotification);
    widthLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(widthLabel_.get());
    widthSlider_->setSliderStyle(Slider::IncDecButtons);
    widthSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
    widthSlider_->setRange(0.f, 1.f, 0.01f);
    widthSlider_->setNumDecimalPlacesToDisplay(2);
    widthSlider_->setDoubleClickReturnValue(true, 0.f);
    addAndMakeVisible(widthSlider_.get());

    heightLabel_->setFont(EarFontsSingleton::instance().Label);
    heightLabel_->setColour(Label::textColourId, EarColours::Label);
    heightLabel_->setText("Height",
                          juce::NotificationType::dontSendNotification);
    heightLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(heightLabel_.get());
    heightSlider_->setSliderStyle(Slider::IncDecButtons);
    heightSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
    heightSlider_->setRange(0.f, 1.f, 0.01f);
    heightSlider_->setNumDecimalPlacesToDisplay(2);
    heightSlider_->setDoubleClickReturnValue(true, 0.f);
    heightSlider_->getValueObject().referTo(sizeValue);
    addAndMakeVisible(heightSlider_.get());
    depthLabel_->setFont(EarFontsSingleton::instance().Label);
    depthLabel_->setColour(Label::textColourId, EarColours::Label);
    depthLabel_->setText("Depth", juce::NotificationType::dontSendNotification);
    depthLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(depthLabel_.get());
    depthSlider_->setSliderStyle(Slider::IncDecButtons);
    depthSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
    depthSlider_->setRange(0.f, 1.f, 0.01f);
    depthSlider_->setNumDecimalPlacesToDisplay(2);
    depthSlider_->setDoubleClickReturnValue(true, 0.f);
    addAndMakeVisible(depthSlider_.get());

    sizeLabel_->setAlpha(1.f);
    sizeSlider_->setAlpha(1.f);
    sizeSlider_->setEnabled(true);
    widthLabel_->setAlpha(Emphasis::disabled);
    widthSlider_->setAlpha(Emphasis::disabled);
    widthSlider_->setEnabled(false);
    heightLabel_->setAlpha(Emphasis::disabled);
    heightSlider_->setAlpha(Emphasis::disabled);
    heightSlider_->setEnabled(false);
    depthLabel_->setAlpha(Emphasis::disabled);
    depthSlider_->setAlpha(Emphasis::disabled);
    depthSlider_->setEnabled(false);

    widthSlider_->setGrabFocusOnTextChange(false);
    heightSlider_->setGrabFocusOnTextChange(false);
    depthSlider_->setGrabFocusOnTextChange(false);

    diffuseLabel_->setFont(EarFontsSingleton::instance().Label);
    diffuseLabel_->setColour(Label::textColourId, EarColours::Label);
    diffuseLabel_->setText("Diffuse",
                           juce::NotificationType::dontSendNotification);
    diffuseLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(diffuseLabel_.get());
    diffuseSlider_->setTicks(std::vector<Tick>{
        {String::fromUTF8("0"), 0.f, Justification::left},
        {String::fromUTF8("0.5"), 0.5f, Justification::centred},
        {String::fromUTF8("1"), 1.f, Justification::right}});
    diffuseSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 44,
                                    rowHeight_);
    diffuseSlider_->setRange(0.f, 1.f);
    diffuseSlider_->setNumDecimalPlacesToDisplay(2);
    diffuseSlider_->setDoubleClickReturnValue(true, 0.f);
    addAndMakeVisible(diffuseSlider_.get());

    divergenceButton_->setButtonText("Divergence");
    divergenceButton_->setShape(EarButton::Shape::Toggle);
    divergenceButton_->setAlpha(Emphasis::disabled);
    divergenceButton_->setEnabled(false);
    addAndMakeVisible(divergenceButton_.get());

    factorLabel_->setFont(EarFontsSingleton::instance().Label);
    factorLabel_->setColour(Label::textColourId, EarColours::Label);
    factorLabel_->setText("Factor",
                          juce::NotificationType::dontSendNotification);
    factorLabel_->setJustificationType(Justification::right);
    factorSlider_->setTicks(
        std::vector<Tick>{{String::fromUTF8("0"), 0.f, Justification::left},
                          {String::fromUTF8("1"), 1.f, Justification::right}});
    factorSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 44, rowHeight_);
    factorSlider_->setRange(0.f, 1.f);
    factorSlider_->setNumDecimalPlacesToDisplay(2);
    factorSlider_->setDoubleClickReturnValue(true, 0.f);
    factorSlider_->setEnabled(false);
    addAndMakeVisible(factorLabel_.get());
    addAndMakeVisible(factorSlider_.get());

    rangeLabel_->setFont(EarFontsSingleton::instance().Label);
    rangeLabel_->setColour(Label::textColourId, EarColours::Label);
    rangeLabel_->setText("Range", juce::NotificationType::dontSendNotification);
    rangeLabel_->setJustificationType(Justification::right);
    rangeSlider_->setTicks(std::vector<Tick>{
        {String::fromUTF8("0"), 0.f, Justification::left},
        {String::fromUTF8("90"), 90.f, Justification::centred},
        {String::fromUTF8("180"), 180.f, Justification::right}});
    rangeSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 44, rowHeight_);
    rangeSlider_->setRange(0.f, 180.f);
    rangeSlider_->setNumDecimalPlacesToDisplay(1);
    rangeSlider_->setDoubleClickReturnValue(true, 0.f);
    rangeSlider_->setEnabled(false);
    addAndMakeVisible(rangeLabel_.get());
    addAndMakeVisible(rangeSlider_.get());
  }

  ~ValueBoxExtent() {}

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
    extentLabel_->setBounds(area.removeFromTop(30));

    area.removeFromTop(2.f * marginBig_);

    float sliderWidth = area.getWidth() - labelWidth_ - marginBig_;

    auto sizeArea = area.removeFromTop(rowHeight_);
    auto linkButtonArea = sizeArea.withWidth(rowHeight_);
    auto sizeLabelArea =
        sizeArea.withWidth(labelWidth_).withTrimmedTop(labelPaddingTop_);
    auto sizeSliderArea = sizeArea.withTrimmedLeft(labelWidth_ + marginBig_);
    linkSizeButton_->setBounds(sizeLabelArea.removeFromLeft(28).expanded(
        0, (28 - (rowHeight_ - labelPaddingTop_)) / 2));
    sizeLabel_->setBounds(sizeLabelArea);
    sizeSlider_->setBounds(sizeSliderArea);

    area.removeFromTop(marginBig_);

    auto sizeDetailedArea = area.removeFromTop(narrowRowHeight_);

    depthSlider_->setBounds(sizeDetailedArea.removeFromRight(44.f));
    depthLabel_->setBounds(sizeDetailedArea.removeFromRight(55.f));
    heightSlider_->setBounds(sizeDetailedArea.removeFromRight(44.f));
    heightLabel_->setBounds(sizeDetailedArea.removeFromRight(55.f));
    widthSlider_->setBounds(sizeDetailedArea.removeFromRight(44.f));
    widthLabel_->setBounds(sizeDetailedArea.removeFromRight(55.f));

    area.removeFromTop(3.f * marginBig_);

    auto diffuseArea = area.removeFromTop(rowHeight_);
    auto diffuseLabelArea =
        diffuseArea.withWidth(labelWidth_).withTrimmedTop(labelPaddingTop_);
    auto diffuseSliderArea =
        diffuseArea.withTrimmedLeft(labelWidth_ + marginBig_);
    diffuseLabel_->setBounds(diffuseLabelArea);
    diffuseSlider_->setBounds(diffuseSliderArea);

    area.removeFromTop(3.f * marginBig_);

    auto divergenceArea = area.removeFromTop(narrowRowHeight_);
    divergenceButton_->setBounds(divergenceArea);

    auto factorArea = area.removeFromTop(rowHeight_);
    auto factorLabelArea =
        factorArea.withWidth(labelWidth_).withTrimmedTop(labelPaddingTop_);
    auto factorSliderArea =
        factorArea.withTrimmedLeft(labelWidth_ + marginBig_);
    factorLabel_->setBounds(factorLabelArea);
    factorSlider_->setBounds(factorSliderArea);

    auto rangeArea = area.removeFromTop(rowHeight_);
    auto rangeLabelArea =
        rangeArea.withWidth(labelWidth_).withTrimmedTop(labelPaddingTop_);
    auto rangeSliderArea = rangeArea.withTrimmedLeft(labelWidth_ + marginBig_);
    rangeLabel_->setBounds(rangeLabelArea);
    rangeSlider_->setBounds(rangeSliderArea);
  }

  std::shared_ptr<EarButton> getLinkSizeButton() { return linkSizeButton_; }
  std::shared_ptr<Label> getSizeLabel() { return sizeLabel_; }
  std::shared_ptr<EarSlider> getSizeSlider() { return sizeSlider_; }
  std::shared_ptr<Label> getWidthLabel() { return widthLabel_; }
  std::shared_ptr<EarSlider> getWidthSlider() { return widthSlider_; }
  std::shared_ptr<Label> getHeightLabel() { return heightLabel_; }
  std::shared_ptr<EarSlider> getHeightSlider() { return heightSlider_; }
  std::shared_ptr<Label> getDepthLabel() { return depthLabel_; }
  std::shared_ptr<EarSlider> getDepthSlider() { return depthSlider_; }
  std::shared_ptr<EarSlider> getDiffuseSlider() { return diffuseSlider_; }
  std::shared_ptr<EarButton> getDivergenceButton() { return divergenceButton_; }
  std::shared_ptr<Label> getFactorLabel() { return factorLabel_; }
  std::shared_ptr<EarSlider> getFactorSlider() { return factorSlider_; }
  std::shared_ptr<Label> getRangeLabel() { return rangeLabel_; }
  std::shared_ptr<EarSlider> getRangeSlider() { return rangeSlider_; }

 private:
  std::unique_ptr<Label> extentLabel_;
  std::shared_ptr<EarButton> linkSizeButton_;
  std::shared_ptr<Label> sizeLabel_;
  std::shared_ptr<EarSlider> sizeSlider_;
  std::shared_ptr<Label> widthLabel_;
  std::shared_ptr<EarSlider> widthSlider_;
  std::shared_ptr<Label> heightLabel_;
  std::shared_ptr<EarSlider> heightSlider_;
  std::shared_ptr<Label> depthLabel_;
  std::shared_ptr<EarSlider> depthSlider_;

  std::unique_ptr<Label> diffuseLabel_;
  std::shared_ptr<EarSlider> diffuseSlider_;
  std::shared_ptr<EarButton> divergenceButton_;
  std::shared_ptr<Label> factorLabel_;
  std::shared_ptr<EarSlider> factorSlider_;
  std::shared_ptr<Label> rangeLabel_;
  std::shared_ptr<EarSlider> rangeSlider_;

  Value sizeValue;

  const float labelWidth_ = 71.f;
  const float labelPaddingTop_ = 20.f;
  const float rowHeight_ = 40.f;
  const float narrowRowHeight_ = 24.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxExtent)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
