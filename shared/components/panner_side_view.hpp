#pragma once

#include "JuceHeader.h"

#include <algorithm>

namespace ear {
namespace plugin {
namespace ui {

class PannerSideView : public Component,
                       public AsyncUpdater,
                       private Value::Listener {
 public:
  PannerSideView() {
    setColour(backgroundColourId, EarColours::Transparent);
    setColour(trackColourId, EarColours::SliderTrack);
    setColour(highlightColourId, EarColours::Item01);
    setColour(angleLabelColourId, EarColours::Text.withAlpha(Emphasis::medium));
    setColour(directionLabelColourId,
              EarColours::Text.withAlpha(Emphasis::high));
    currentElevationValue_.addListener(this);
    mouseDragActive_ = false;
  }

  ~PannerSideView() { currentElevationValue_.removeListener(this); }

  void paint(Graphics& g) override {
    // background
    g.fillAll(findColour(backgroundColourId));

    // outer track
    g.setColour(findColour(trackColourId));
    g.drawLine(trackLine_, outerTrackWidth_);
    g.drawLine(topTick_, tickWidth_);
    g.drawLine(centreTick_, tickWidth_);
    g.drawLine(bottomTick_, tickWidth_);

    // tick labels
    g.setColour(findColour(angleLabelColourId));
    g.setFont(EarFonts::Measures);
    g.drawText(String("+90"), topTickLabelRect_, Justification::right);
    g.drawText(String("0"), centreTickLabelRect_, Justification::right);
    g.drawText(String("-90"), bottomTickLabelRect_, Justification::right);

    // inner track
    const float yPos =
        getHeight() / 2.f - (elevation_ / 90.f * trackLength_ / 2.f);

    g.setColour(findColour(highlightColourId));
    g.drawLine(getWidth() - knobSize_ / 2.f, getHeight() / 2.f,
               getWidth() - knobSize_ / 2.f, yPos);
    // thumb
    g.setColour(findColour(highlightColourId));
    g.fillEllipse(getWidth() - knobSize_, yPos - knobSize_ / 2.f, knobSize_,
                  knobSize_);

    // labeling
    g.setColour(findColour(directionLabelColourId));
    g.setFont(EarFonts::Values);
    g.drawText(String("Top"), topLabelRect_, Justification::centredBottom);
    g.drawText(String("Bottom"), bottomLabelRect_, Justification::centredTop);
  }

  void mouseDoubleClick(const MouseEvent&) override {
    if (isEnabled()) {
      setElevation(elevationDefault_, sendNotificationSync);
      repaint();
    }
  }

  void mouseDown(const MouseEvent& event) override {
    if (isEnabled() && activeArea_.contains(getMouseXYRelative().toFloat())) {
      handleDragStart();
    }
  }

  void mouseUp(const MouseEvent& event) override { handleDragEnd(); }

  void mouseDrag(const MouseEvent& event) override {
    if (isEnabled() && mouseDragActive_) {
      auto position = event.getPosition();
      const auto posRel = juce::Point<float>(
          getWidth() - knobSize_ / 2.f - static_cast<float>(position.getX()),
          getHeight() / 2.f - static_cast<float>(position.getY()));
      float newElevation = 90.f * posRel.getY() / (trackLength_ / 2.f);
      newElevation = std::max(-90.f, std::min(newElevation, 90.f));
      setElevation(newElevation, sendNotificationSync);
    }
  }

  void handleDragStart() {
    mouseDragActive_ = true;
    Component::BailOutChecker checker(this);
    listeners_.callChecked(checker,
                           [&](Listener& l) { l.pannerDragStarted(this); });
    if (checker.shouldBailOut()) {
      return;
    }
  }

  void handleDragEnd() {
    mouseDragActive_ = false;
    Component::BailOutChecker checker(this);
    listeners_.callChecked(checker,
                           [this](Listener& l) { l.pannerDragEnded(this); });

    if (checker.shouldBailOut()) {
      return;
    }
  }

  MouseCursor getMouseCursor() override {
    if (isEnabled()) {
      if (mouseDragActive_) {
        return MouseCursor::DraggingHandCursor;
      } else if (activeArea_.contains(getMouseXYRelative().toFloat())) {
        return MouseCursor::PointingHandCursor;
      }
    }
    return MouseCursor::NormalCursor;
  }

  enum ColourIds {
    backgroundColourId = 0x00010001,
    trackColourId = 0x00010002,
    highlightColourId = 0x00010003,
    directionLabelColourId = 0x00010004,
    angleLabelColourId = 0x00010005,
  };

  void resized() override {
    activeArea_ = juce::Rectangle<float>(
        0.f,
        getHeight() / 2.f - trackLength_ / 2.f - outerTrackWidth_ / 2.f -
            knobSize_ / 2.f,
        getWidth(), trackLength_ + knobSize_ + outerTrackWidth_);

    trackLength_ = trackLength_ + outerTrackWidth_;
    trackLine_ = Line<float>(
        getWidth() - knobSize_ / 2.f,
        getHeight() / 2.f - trackLength_ / 2.f - outerTrackWidth_ / 2.f,
        getWidth() - knobSize_ / 2.f,
        getHeight() / 2.f + trackLength_ / 2.f + outerTrackWidth_ / 2.f);
    topTick_ =
        Line<float>(getWidth() - knobSize_ / 2.f - outerTrackWidth_ / 2.f,
                    getHeight() / 2.f - trackLength_ / 2.f -
                        outerTrackWidth_ / 2.f + tickWidth_ / 2.f,
                    getWidth() - knobSize_ / 2.f - tickLength_,
                    getHeight() / 2.f - trackLength_ / 2.f -
                        outerTrackWidth_ / 2.f + tickWidth_ / 2.f);
    centreTick_ = Line<float>(
        getWidth() - knobSize_ / 2.f - outerTrackWidth_ / 2.f,
        getHeight() / 2.f, getWidth() - knobSize_ / 2.f - tickLength_,
        getHeight() / 2.f);
    bottomTick_ =
        Line<float>(getWidth() - knobSize_ / 2.f - outerTrackWidth_ / 2.f,
                    getHeight() / 2.f + trackLength_ / 2.f +
                        outerTrackWidth_ / 2.f - tickWidth_ / 2.f,
                    getWidth() - knobSize_ / 2.f - tickLength_,
                    getHeight() / 2.f + trackLength_ / 2.f +
                        outerTrackWidth_ / 2.f - tickWidth_ / 2.f);

    topTickLabelRect_ = juce::Rectangle<float>(
        getWidth() - knobSize_ / 2.f - tickLength_ - tickLabelWidth_ - padding_,
        getHeight() / 2.f - trackLength_ / 2.f - outerTrackWidth_ / 2.f +
            tickWidth_ / 2.f - tickLabelHeight_ / 2.f,
        tickLabelWidth_, tickLabelHeight_);
    centreTickLabelRect_ = juce::Rectangle<float>(
        getWidth() - knobSize_ / 2.f - tickLength_ - tickLabelWidth_ - padding_,
        getHeight() / 2.f - tickLabelHeight_ / 2.f, tickLabelWidth_,
        tickLabelHeight_);
    bottomTickLabelRect_ = juce::Rectangle<float>(
        getWidth() - knobSize_ / 2.f - tickLength_ - tickLabelWidth_ - padding_,
        getHeight() / 2.f + trackLength_ / 2.f + outerTrackWidth_ / 2.f -
            tickWidth_ / 2.f - tickLabelHeight_ / 2.f,
        tickLabelWidth_, tickLabelHeight_);

    topLabelRect_ = juce::Rectangle<float>(
        getWidth() / 2.f - labelWidth_ / 2.f,
        getHeight() / 2.f - trackLength_ / 2.f - tickLength_ - 2.f * padding_ -
            angleLabelFontsize_ - labelHeight_,
        labelWidth_, labelHeight_);
    bottomLabelRect_ = juce::Rectangle<float>(
        getWidth() / 2.f - labelWidth_ / 2.f,
        getHeight() / 2.f + trackLength_ / 2.f + tickLength_ + 2.f * padding_ +
            angleLabelFontsize_,
        labelWidth_, labelHeight_);
  }

  Value& getElevationValueObject() noexcept { return currentElevationValue_; }

  void setElevation(double newValue, NotificationType notification) {
    newValue = normRangeElevation_.snapToLegalValue(newValue);
    if (newValue != elevation_) {
      elevation_ = newValue;
      if (currentElevationValue_ != newValue) {
        currentElevationValue_ = newValue;
      }
      repaint();
      triggerChangeMessage(notification);
    }
  }

  void triggerChangeMessage(NotificationType notification) {
    if (notification != dontSendNotification) {
      if (notification == sendNotificationSync)
        handleAsyncUpdate();
      else
        triggerAsyncUpdate();
    }
  }

  void handleAsyncUpdate() override {
    cancelPendingUpdate();
    Component::BailOutChecker checker(this);
    listeners_.callChecked(checker,
                           [this](Listener& l) { l.pannerValueChanged(this); });
    if (checker.shouldBailOut()) {
      return;
    }
  }

  void valueChanged(Value& value) override {
    if (value.refersToSameSourceAs(currentElevationValue_)) {
      setElevation(currentElevationValue_.getValue(), dontSendNotification);
    }
  }

  float getElevation() { return currentElevationValue_.getValue(); }

  class Listener {
   public:
    virtual ~Listener() = default;

    virtual void pannerValueChanged(PannerSideView* panner) = 0;
    virtual void pannerDragStarted(PannerSideView* panner) = 0;
    virtual void pannerDragEnded(PannerSideView* panner) = 0;
  };

  void addListener(Listener* l) { listeners_.add(l); }
  void removeListener(Listener* l) { listeners_.remove(l); }

 private:
  float elevationDefault_ = 0.f;
  float elevation_ = 0.f;

  Value currentElevationValue_;
  NormalisableRange<double> normRangeElevation_{-90.0, 90.0};

  juce::Rectangle<float> activeArea_;
  bool mouseDragActive_;

  Line<float> trackLine_;
  Line<float> topTick_;
  Line<float> centreTick_;
  Line<float> bottomTick_;
  juce::Rectangle<float> topTickLabelRect_;
  juce::Rectangle<float> centreTickLabelRect_;
  juce::Rectangle<float> bottomTickLabelRect_;
  juce::Rectangle<float> topLabelRect_;
  juce::Rectangle<float> bottomLabelRect_;

  float trackLength_ = 205.f;
  float trackWidth_ = 1.f;
  float outerTrackWidth_ = 5.f;
  float tickLength_ = 13.5f;
  float tickWidth_ = 1.f;
  float tickLabelWidth_ = 50.f;
  float tickLabelHeight_ = 12.f;
  float labelWidth_ = 50.f;
  float labelHeight_ = 12.f;
  float angleLabelFontsize_ = 10.f;
  float directionLabelFontsize_ = 12.f;
  float padding_ = 2.f;
  float knobSize_ = 19.f;
  float crossSize_ = 28.f;
  float crossWidth_ = 1.f;

  ListenerList<Listener> listeners_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PannerSideView)
};  // namespace ui

}  // namespace ui
}  // namespace plugin
}  // namespace ear
