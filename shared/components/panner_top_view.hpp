#pragma once

#include "JuceHeader.h"
#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class PannerTopView : public Component,
                      public AsyncUpdater,
                      private Value::Listener {
 public:
  PannerTopView() {
    setColour(backgroundColourId, EarColours::Transparent);
    setColour(trackColourId, EarColours::SliderTrack);
    setColour(highlightColourId, EarColours::Item01);
    setColour(angleLabelColourId, EarColours::Text.withAlpha(Emphasis::medium));
    setColour(directionLabelColourId,
              EarColours::Text.withAlpha(Emphasis::high));
    currentAzimuthValue_.addListener(this);
    currentDistanceValue_.addListener(this);
  }

  ~PannerTopView() {
    currentAzimuthValue_.removeListener(this);
    currentDistanceValue_.removeListener(this);
  }

  void paint(Graphics& g) override {
    // background
    g.fillAll(findColour(backgroundColourId));

    // center cross
    Line<float> horizontalLine(
        getWidth() / 2.f - crossSize_ / 2.f, getHeight() / 2.f,
        getWidth() / 2.f + crossSize_ / 2.f, getHeight() / 2.f);
    Line<float> verticalLine(
        getWidth() / 2.f, getHeight() / 2 - crossSize_ / 2.f, getWidth() / 2.f,
        getHeight() / 2.f + crossSize_ / 2.f);
    g.setColour(findColour(trackColourId));
    g.drawLine(horizontalLine, crossWidth_);
    g.drawLine(verticalLine, crossWidth_);

    // outer track
    g.setColour(findColour(trackColourId));
    g.drawEllipse(getWidth() / 2.f - outerDiameter_ / 2.f,
                  getHeight() / 2.f - outerDiameter_ / 2.f, outerDiameter_,
                  outerDiameter_, outerTrackWidth_);
    g.drawLine(topTick_, tickWidth_);
    g.drawLine(bottomTick_, tickWidth_);
    g.drawLine(leftTick_, tickWidth_);
    g.drawLine(rightTick_, tickWidth_);

    // inner track
    const float innerDiameter = distance_ * outerDiameter_;
    g.setColour(findColour(highlightColourId));
    g.drawEllipse(getWidth() / 2.f - innerDiameter / 2.f,
                  getHeight() / 2.f - innerDiameter / 2.f, innerDiameter,
                  innerDiameter, trackWidth_);

    // thumb
    g.setColour(findColour(highlightColourId));
    float azimuthDeg = azimuth_ / 180.f * MathConstants<float>::pi;
    float xPos = std::sin(azimuthDeg) * innerDiameter / 2.f;
    float yPos = std::cos(azimuthDeg) * innerDiameter / 2.f;
    g.fillEllipse(getWidth() / 2.f - xPos - knobSize_ / 2.f,
                  getHeight() / 2.f - yPos - knobSize_ / 2.f, knobSize_,
                  knobSize_);

    // labeling
    g.setColour(findColour(angleLabelColourId));
    g.setFont(EarFontsSingleton::instance().Measures);
    g.drawText(String::fromUTF8("0"), topTickLabelRect_,
               Justification::centredBottom);
    g.drawText(String::fromUTF8("180"), bottomTickLabelRect_,
               Justification::centredTop);
    g.drawText(String::fromUTF8("+90"), leftTickLabelRect_,
               Justification::right);
    g.drawText(String::fromUTF8("–90"), rightTickLabelRect_,
               Justification::left);

    g.setColour(findColour(directionLabelColourId));
    g.setFont(EarFontsSingleton::instance().Values);
    g.drawText(String("Front"), frontLabelRect_, Justification::centredBottom);
    g.drawText(String("Back"), backLabelRect_, Justification::centredTop);
  }

  void mouseDown(const MouseEvent& event) override {
    if (isEnabled() && activeArea_.contains(getMouseXYRelative().toFloat())) {
      handleDragStart();
    }
  }

  void mouseUp(const MouseEvent& event) override { handleDragEnd(); }

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

  void mouseDoubleClick(const MouseEvent&) override {
    if (isEnabled()) {
      setDistance(distanceDefault_, sendNotificationSync);
      setAzimuth(azimuthDefault_, sendNotificationSync);
    }
  }

  void mouseDrag(const MouseEvent& event) override {
    if (isEnabled() && mouseDragActive_) {
      auto position = event.getPosition();
      const auto posRel = juce::Point<float>(
          getWidth() / 2.f - static_cast<float>(position.getX()),
          getHeight() / 2.f - static_cast<float>(position.getY()));
      if (!event.mods.isShiftDown()) {
        float newDistance =
            posRel.getDistanceFromOrigin() * 2.f / outerDiameter_;
        if (newDistance > 1.f) {
          newDistance = 1.f;
        }
        if (newDistance < 0.05f) {
          newDistance = 0;
        }
        setDistance(newDistance, sendNotificationSync);
      }
      float newAzimuth = std::atan2(posRel.getX(), posRel.getY()) * 180.f /
                         MathConstants<float>::pi;
      if (getDistance() < 0.01) {
        newAzimuth = 0.f;
      }
      setAzimuth(newAzimuth, sendNotificationSync);
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

  enum ColourIds {
    backgroundColourId = 0x00010001,
    trackColourId = 0x00010002,
    highlightColourId = 0x00010003,
    directionLabelColourId = 0x00010004,
    angleLabelColourId = 0x00010005,
  };

  void resized() override {
    activeArea_.clear();
    activeArea_.addEllipse(
        getWidth() / 2.f -
            (outerDiameter_ + outerTrackWidth_ + knobSize_) / 2.f,
        getHeight() / 2.f -
            (outerDiameter_ + outerTrackWidth_ + knobSize_) / 2.f,
        outerDiameter_ + outerTrackWidth_ + knobSize_,
        outerDiameter_ + outerTrackWidth_ + knobSize_);

    topTick_ = Line<float>(
        getWidth() / 2.f,
        getHeight() / 2.f - outerDiameter_ / 2.f - outerTrackWidth_ / 2.f,
        getWidth() / 2.f,
        getHeight() / 2.f - outerDiameter_ / 2.f - tickLength_);
    bottomTick_ = Line<float>(
        getWidth() / 2.f,
        getHeight() / 2.f + outerDiameter_ / 2.f + outerTrackWidth_ / 2.f,
        getWidth() / 2.f,
        getHeight() / 2.f + outerDiameter_ / 2.f + tickLength_);
    leftTick_ = Line<float>(
        getWidth() / 2.f - outerDiameter_ / 2.f - outerTrackWidth_ / 2.f,
        getHeight() / 2.f,
        getWidth() / 2.f - outerDiameter_ / 2.f - tickLength_,
        getHeight() / 2.f);
    rightTick_ = Line<float>(
        getWidth() / 2.f + outerDiameter_ / 2.f + outerTrackWidth_ / 2.f,
        getHeight() / 2.f,
        getWidth() / 2.f + outerDiameter_ / 2.f + tickLength_,
        getHeight() / 2.f);

    topTickLabelRect_ = juce::Rectangle<float>(
        getWidth() / 2.f - labelWidth_ / 2.f,
        getHeight() / 2.f - outerDiameter_ / 2.f - tickLength_ - labelHeight_,
        labelWidth_, labelHeight_);
    bottomTickLabelRect_ = juce::Rectangle<float>(  //
        getWidth() / 2.f - labelWidth_ / 2.f,
        getHeight() / 2.f + outerDiameter_ / 2.f + tickLength_, labelWidth_,
        labelHeight_);
    leftTickLabelRect_ = juce::Rectangle<float>(
        getWidth() / 2.f - outerDiameter_ / 2.f - tickLength_ - labelWidth_ -
            padding_,
        getHeight() / 2.f - labelHeight_ / 2.f, labelWidth_, labelHeight_);
    rightTickLabelRect_ = juce::Rectangle<float>(
        getWidth() / 2.f + outerDiameter_ / 2.f + tickLength_ + padding_,
        getHeight() / 2.f - labelHeight_ / 2.f, labelWidth_, labelHeight_);

    frontLabelRect_ = juce::Rectangle<float>(
        getWidth() / 2.f - labelWidth_ / 2.f,
        getHeight() / 2.f - outerDiameter_ / 2.f - tickLength_ -
            2.f * padding_ - EarFontsSingleton::instance().Measures.getAscent() - labelHeight_,
        labelWidth_, labelHeight_);
    backLabelRect_ = juce::Rectangle<float>(
        getWidth() / 2.f - labelWidth_ / 2.f,
        getHeight() / 2.f + outerDiameter_ / 2.f + tickLength_ +
            2.f * padding_ + EarFontsSingleton::instance().Measures.getAscent(),
        labelWidth_, labelHeight_);
  }

  Value& getAzimuthValueObject() noexcept { return currentAzimuthValue_; }
  Value& getDistanceValueObject() noexcept { return currentDistanceValue_; }

  void setAzimuth(double newValue, NotificationType notification) {
    newValue = normRangeAzimuth_.snapToLegalValue(newValue);
    if (newValue != azimuth_) {
      azimuth_ = newValue;
      if (currentAzimuthValue_ != newValue) {
        currentAzimuthValue_ = newValue;
      }
      repaint();
      triggerChangeMessage(notification);
    }
  }

  void setDistance(double newValue, NotificationType notification) {
    newValue = normRangeDistance_.snapToLegalValue(newValue);
    if (newValue != distance_) {
      distance_ = newValue;
      if (currentDistanceValue_ != newValue) {
        currentDistanceValue_ = newValue;
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
    if (value.refersToSameSourceAs(currentAzimuthValue_)) {
      setAzimuth(currentAzimuthValue_.getValue(), dontSendNotification);
    } else if (value.refersToSameSourceAs(currentDistanceValue_)) {
      setDistance(currentDistanceValue_.getValue(), dontSendNotification);
    }
  }

  float getAzimuth() { return currentAzimuthValue_.getValue(); }
  float getDistance() { return currentDistanceValue_.getValue(); }

  class Listener {
   public:
    virtual ~Listener() = default;

    virtual void pannerValueChanged(PannerTopView* panner) = 0;
    virtual void pannerDragStarted(PannerTopView* panner) = 0;
    virtual void pannerDragEnded(PannerTopView* panner) = 0;
  };

  void addListener(Listener* l) { listeners_.add(l); }
  void removeListener(Listener* l) { listeners_.remove(l); }

 private:
  bool mouseDragActive_;
  Path activeArea_;

  double azimuthDefault_ = 0.f;
  double distanceDefault_ = 1.f;
  double azimuth_ = 0.f;
  double distance_ = 1.f;

  Value currentAzimuthValue_;
  Value currentDistanceValue_;
  NormalisableRange<double> normRangeAzimuth_{-180.0, 180.0};
  NormalisableRange<double> normRangeDistance_{0.0, 1.0};

  Line<float> topTick_;
  Line<float> bottomTick_;
  Line<float> leftTick_;
  Line<float> rightTick_;
  juce::Rectangle<float> topTickLabelRect_;
  juce::Rectangle<float> bottomTickLabelRect_;
  juce::Rectangle<float> leftTickLabelRect_;
  juce::Rectangle<float> rightTickLabelRect_;
  juce::Rectangle<float> frontLabelRect_;
  juce::Rectangle<float> backLabelRect_;

  const float outerDiameter_ = 205.f;
  const float trackWidth_ = 1.f;
  const float outerTrackWidth_ = 5.f;
  const float tickLength_ = 13.5f;
  const float tickWidth_ = 1.f;
  const float labelWidth_ = 50.f;
  const float labelHeight_ = 12.f;
  const float padding_ = 2.f;
  const float knobSize_ = 19.f;
  const float crossSize_ = 28.f;
  const float crossWidth_ = 1.f;

  ListenerList<Listener> listeners_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PannerTopView)
};  // namespace ui

}  // namespace ui
}  // namespace plugin
}  // namespace ear
