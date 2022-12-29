#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/look_and_feel/slider.hpp"
#include "components/speaker_layer.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ValueBoxSpeakerLayer : public Component {
 public:
  ValueBoxSpeakerLayer(const String& headerText)
      : headingLabel_(std::make_unique<Label>()),
        layer_(std::make_unique<SpeakerLayer>()) {
    headingLabel_->setFont(EarFontsSingleton::instance().Label);
    headingLabel_->setColour(Label::textColourId, EarColours::Heading);
    headingLabel_->setText(headerText,
                           juce::NotificationType::dontSendNotification);
    headingLabel_->setJustificationType(Justification::bottomLeft);
    addAndMakeVisible(headingLabel_.get());
    addAndMakeVisible(layer_.get());
  }

  ~ValueBoxSpeakerLayer() {}

  void clearSpeakerSetup() { layer_->clearSpeakerSetup(); }
  void setSpeakerSetup(ear::plugin::SpeakerSetup speakerSetup) {
    layer_->setSpeakerSetup(speakerSetup);
  }

  void setHighlightColour(Colour colour) {
    layer_->setColour(SpeakerLayer::highlightColourId, colour);
  }

  void setLayer(ear::plugin::Layer layer) { layer_->setLayer(layer); }

  void paint(Graphics& g) override { g.fillAll(EarColours::Area01dp); }

  void resized() override {
    auto area = getLocalBounds();
    area.reduce(10, 5);
    headingLabel_->setBounds(area.removeFromTop(30));
    // TODO: No trimmed bottom when boxes were smaller (before metadata removal)
    layer_->setBounds(area.withTrimmedLeft(70).withTrimmedBottom(20));
  }

 private:
  std::unique_ptr<Label> headingLabel_;
  std::unique_ptr<SpeakerLayer> layer_;

  const float labelWidth_ = 71.f;
  const float labelPaddingBottom_ = 0.f;
  const float rowHeight_ = 40.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxSpeakerLayer)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
