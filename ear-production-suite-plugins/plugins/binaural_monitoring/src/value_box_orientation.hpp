#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/orientation.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ValueBoxOrientation : public Component {
public:
   ValueBoxOrientation()
      : headingLabel_(std::make_unique<Label>()),
        yawLabel_(std::make_unique<Label>()),
        pitchLabel_(std::make_unique<Label>()),
        rollLabel_(std::make_unique<Label>()) {
    setColour(backgroundColourId, EarColours::Area01dp);

    headingLabel_->setFont(EarFontsSingleton::instance().Heading);
    headingLabel_->setColour(Label::textColourId, EarColours::Heading);
    headingLabel_->setText("Listener Orientation",
                           juce::NotificationType::dontSendNotification);
    headingLabel_->setJustificationType(Justification::bottomLeft);
    addAndMakeVisible(headingLabel_.get());

    yawLabel_->setFont(EarFontsSingleton::instance().Label);
    yawLabel_->setColour(Label::textColourId, EarColours::Label);
    yawLabel_->setText("Yaw", juce::NotificationType::dontSendNotification);
    yawLabel_->setJustificationType(Justification::topLeft);
    addAndMakeVisible(yawLabel_.get());

    pitchLabel_->setFont(EarFontsSingleton::instance().Label);
    pitchLabel_->setColour(Label::textColourId, EarColours::Label);
    pitchLabel_->setText("Pitch", juce::NotificationType::dontSendNotification);
    pitchLabel_->setJustificationType(Justification::topLeft);
    addAndMakeVisible(pitchLabel_.get());

    rollLabel_->setFont(EarFontsSingleton::instance().Label);
    rollLabel_->setColour(Label::textColourId, EarColours::Label);
    rollLabel_->setText("Roll", juce::NotificationType::dontSendNotification);
    rollLabel_->setJustificationType(Justification::topLeft);
    addAndMakeVisible(rollLabel_.get());

    yawControl_ = std::make_shared<OrientationView>(
      -MathConstants<float>::pi, MathConstants<float>::pi,
      -180.f, 180.f, -180.f, 180.f, 0.f,
      String("Front"), String("Back") );
    addAndMakeVisible(yawControl_.get());

    pitchControl_ = std::make_shared<OrientationView>(
      0.f, MathConstants<float>::pi,
      90.f, -90.f, -180.f, 180.f, 0.f,
      String("Horizon"), String() );
    addAndMakeVisible(pitchControl_.get());

    rollControl_ = std::make_shared<OrientationView>(
      -MathConstants<float>::halfPi, MathConstants<float>::halfPi,
      -90.f, 90.f, -180.f, 180.f, 0.f,
      String("Upright"), String() );
    addAndMakeVisible(rollControl_.get());

  }

  ~ValueBoxOrientation() {}

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

    area.removeFromLeft(controlSeperation_);
    auto yawArea = area.removeFromLeft(200);
    area.removeFromLeft(controlSeperation_);
    auto pitchArea = area.removeFromLeft(200);
    area.removeFromLeft(controlSeperation_);
    auto rollArea = area.removeFromLeft(200);

    yawLabel_->setBounds(yawArea.removeFromTop(20));
    yawControl_->setBounds(yawArea.withHeight(200).withWidth(200));

    pitchLabel_->setBounds(pitchArea.removeFromTop(20));
    pitchControl_->setBounds(pitchArea.withHeight(200).withWidth(200));

    rollLabel_->setBounds(rollArea.removeFromTop(20));
    rollControl_->setBounds(rollArea.withHeight(200).withWidth(200));
  }

  std::shared_ptr<OrientationView> getYawControl() { return yawControl_; }
  std::shared_ptr<OrientationView> getPitchControl() { return pitchControl_; }
  std::shared_ptr<OrientationView> getRollControl() { return rollControl_; }

private:
  std::unique_ptr<Label> headingLabel_;

  std::unique_ptr<Label> yawLabel_;
  std::shared_ptr<OrientationView> yawControl_;
  std::unique_ptr<Label> pitchLabel_;
  std::shared_ptr<OrientationView> pitchControl_;
  std::unique_ptr<Label> rollLabel_;
  std::shared_ptr<OrientationView> rollControl_;

  const float labelWidth_ = 71.f;
  const float labelPaddingBottom_ = 0.f;
  const float rowHeight_ = 40.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;
  const float controlSeperation_ = 20.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxOrientation)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
