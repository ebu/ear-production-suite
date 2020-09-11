#pragma once

#include "JuceHeader.h"

namespace ear {
namespace plugin {
namespace ui {

class EarSliderRange : public Component,
                       public AsyncUpdater,
                       private Value::Listener {
 public:
  EarSliderRange() {
    setColour(backgroundColourId, EarColours::Transparent);
    setColour(trackColourId, Colour(0x29a5a5a5));
    setColour(highlightColourId, EarColours::Text.withAlpha(Emphasis::medium));
    setColour(thumbColourId, EarColours::Text);
    lowerMouseDragActive_ = false;
    upperMouseDragActive_ = false;

    currentLowerValue_.addListener(this);
    currentUpperValue_.addListener(this);

    setWantsKeyboardFocus(true);
    setMouseClickGrabsKeyboardFocus(true);
  }

  ~EarSliderRange() { currentLowerValue_.removeListener(this); }

  void setRange(double newMin, double newMax, double newInt = 0.0) {
    normRange_ = NormalisableRange<double>(
        newMin, newMax, newInt, normRange_.skew, normRange_.symmetricSkew);
    lowerNormRange_ = NormalisableRange<double>(
        newMin, newMax, newInt, normRange_.skew, normRange_.symmetricSkew);
    upperNormRange_ = NormalisableRange<double>(
        newMin, newMax, newInt, normRange_.skew, normRange_.symmetricSkew);
  }
  void setLowerRange(double newMin, double newMax, double newInt = 0.0) {
    lowerNormRange_ = NormalisableRange<double>(
        newMin, newMax, newInt, normRange_.skew, normRange_.symmetricSkew);
  }
  void setUpperRange(double newMin, double newMax, double newInt = 0.0) {
    upperNormRange_ = NormalisableRange<double>(
        newMin, newMax, newInt, normRange_.skew, normRange_.symmetricSkew);
  }

  void setSkewFactorFromMidPoint(double skewFactor) {
    normRange_.setSkewForCentre(skewFactor);
    lowerNormRange_.setSkewForCentre(skewFactor);
    upperNormRange_.setSkewForCentre(skewFactor);
  }

  void setLowerDoubleClickReturnValue(double valueToSetOnDoubleClick) {
    lowerDefault_ = valueToSetOnDoubleClick;
  }
  void setUpperDoubleClickReturnValue(double valueToSetOnDoubleClick) {
    upperDefault_ = valueToSetOnDoubleClick;
  }
  void setTicks(std::vector<Tick> ticks) { ticks_ = ticks; }

  double valueToProportionOfLength(double value) {
    return normRange_.convertTo0to1(value);
  }

  double proportionOfLengthToValue(double proportion) {
    float normalisedValue = (proportion - paddingLeft_) / trackLength();
    return normRange_.convertFrom0to1(normalisedValue);
  }

  float getMinValue() { return lowerNormRange_.getRange().getStart(); }
  float getMaxValue() { return lowerNormRange_.getRange().getEnd(); }

  float trackLength() { return getWidth() - paddingLeft_ - paddingRight_; }

  float getXPositionOfValue(float value) {
    return paddingLeft_ + trackLength() * valueToProportionOfLength(value);
  }

  bool isInLowerArea(juce::Point<int> position) {
    int threshold =
        getXPositionOfValue(lowerValue_) +
        (getXPositionOfValue(upperValue_) - getXPositionOfValue(lowerValue_)) /
            2;
    return position.getX() < threshold;
  }
  bool isInUpperArea(juce::Point<int> position) {
    return !isInLowerArea(position);
  }

  void paint(Graphics& g) override {
    g.fillAll(findColour(backgroundColourId));

    // draw track
    g.setColour(findColour(trackColourId));
    fillRoundedRectangle(g, getXPositionOfValue(getMinValue()), trackPosY_,
                         trackLength(), trackWidth_, 1.f);

    // draw highlighted track
    g.setColour(findColour(highlightColourId));
    g.fillRect(
        getXPositionOfValue(lowerValue_), trackPosY_,
        getXPositionOfValue(upperValue_) - getXPositionOfValue(lowerValue_),
        trackWidth_);

    // draw ticks + labels
    g.setColour(findColour(highlightColourId));
    g.setFont(EarFonts::Measures);
    for (const auto tick : ticks_) {
      float tickPosX = getXPositionOfValue(tick.value);
      g.drawLine(tickPosX, tickPosStartY_, tickPosX, tickPosEndY_, tickWidth_);
      auto justification = Justification::centred;
      juce::Rectangle<float> labelRect(tickPosX - tickLabelWidth_ / 2.f,
                                       tickLabelPosY_, tickLabelWidth_,
                                       tickLabelHeight_);
      if (tick.justification == Justification::left) {
        labelRect.translate(tickLabelWidth_ / 2.f, 0.f);
      } else if (tick.justification == Justification::right) {
        labelRect.translate(-tickLabelWidth_ / 2.f, 0.f);
      }
      g.drawText(tick.text, labelRect, tick.justification);
    }
    // draw thumbs
    g.setColour(findColour(thumbColourId));
    Path lowerHandlePath;
    lowerHandlePath.addArc(getXPositionOfValue(lowerValue_) - thumbRadius_,
                           trackPosY_ + trackWidth_ / 2.f - thumbRadius_,
                           2 * thumbRadius_, 2 * thumbRadius_,
                           MathConstants<float>::pi + 0.02f,
                           MathConstants<float>::twoPi - 0.02f, true);
    lowerHandlePath.closeSubPath();
    g.fillPath(lowerHandlePath);
    // g.fillEllipse(getXPositionOfValue(lowerValue_) - thumbRadius_,
    //               getHeight() - 2.f * thumbRadius_, 2.0f * thumbRadius_,
    //               2.0f * thumbRadius_);
    Path upperHandlePath;
    upperHandlePath.addArc(getXPositionOfValue(upperValue_) - thumbRadius_,
                           trackPosY_ + trackWidth_ / 2.f - thumbRadius_,
                           2 * thumbRadius_, 2 * thumbRadius_, 0.02f,
                           MathConstants<float>::pi - 0.02f, true);
    upperHandlePath.closeSubPath();
    g.fillPath(upperHandlePath);
    // g.fillEllipse(getXPositionOfValue(upperValue_) - thumbRadius_,
    //               getHeight() - 2.f * thumbRadius_, 2.0f * thumbRadius_,
    //               2.0f * thumbRadius_);
  }

  void mouseDoubleClick(const MouseEvent&) override {
    if (isEnabled()) {
      setLowerValue(lowerDefault_, sendNotificationSync);
      setUpperValue(upperDefault_, sendNotificationSync);
      repaint();
    }
  }

  void mouseDown(const MouseEvent& event) override {
    if (isEnabled() && isInLowerArea(event.getPosition())) {
      lowerHandleDragStart();
    }
    if (isEnabled() && isInUpperArea(event.getPosition())) {
      upperHandleDragStart();
    }
  }

  void mouseUp(const MouseEvent& event) override {
    if (lowerMouseDragActive_) {
      lowerHandleDragEnd();
    }
    if (upperMouseDragActive_) {
      upperHandleDragEnd();
    }
  }

  void mouseDrag(const MouseEvent& event) override {
    if (isEnabled() && lowerMouseDragActive_) {
      float newValue = proportionOfLengthToValue(event.getPosition().getX());
      if (newValue > upperValue_) {
        newValue = upperValue_;
      }
      setLowerValue(newValue, sendNotificationSync);
    }
    if (isEnabled() && upperMouseDragActive_) {
      float newValue = proportionOfLengthToValue(event.getPosition().getX());
      if (newValue < lowerValue_) {
        newValue = lowerValue_;
      }
      setUpperValue(newValue, sendNotificationSync);
    }
  }

  void lowerHandleDragStart() {
    lowerMouseDragActive_ = true;
    Component::BailOutChecker checker(this);
    listeners_.callChecked(
        checker, [&](Listener& l) { l.sliderRangeLowerDragStarted(this); });
    if (checker.shouldBailOut()) {
      return;
    }
  }

  void upperHandleDragStart() {
    upperMouseDragActive_ = true;
    Component::BailOutChecker checker(this);
    listeners_.callChecked(
        checker, [&](Listener& l) { l.sliderRangeUpperDragStarted(this); });
    if (checker.shouldBailOut()) {
      return;
    }
  }

  void lowerHandleDragEnd() {
    lowerMouseDragActive_ = false;
    Component::BailOutChecker checker(this);
    listeners_.callChecked(
        checker, [this](Listener& l) { l.sliderRangeLowerDragEnded(this); });
    if (checker.shouldBailOut()) {
      return;
    }
  }

  void upperHandleDragEnd() {
    upperMouseDragActive_ = false;
    Component::BailOutChecker checker(this);
    listeners_.callChecked(
        checker, [this](Listener& l) { l.sliderRangeUpperDragEnded(this); });
    if (checker.shouldBailOut()) {
      return;
    }
  }

  MouseCursor getMouseCursor() override {
    if (isEnabled()) {
      if (lowerMouseDragActive_ || upperMouseDragActive_) {
        return MouseCursor::DraggingHandCursor;
      } else {
        return MouseCursor::PointingHandCursor;
      }
    }
    return MouseCursor::NormalCursor;
  }

  enum ColourIds {
    backgroundColourId = 0x33b43b4b,
    trackColourId = 0x2a1b9a44,
    highlightColourId = 0x37bc9418,
    thumbColourId = 0x0d372173,
  };

  void resized() override {
    trackPosY_ = getHeight() - thumbRadius_ - trackWidth_ / 2.f;
    tickPosStartY_ = trackPosY_ - tickMargin_ - tickLength_;
    tickPosEndY_ = trackPosY_ - tickMargin_;
    tickLabelPosY_ = tickPosStartY_ - tickMargin_ - tickLabelHeight_;
  }

  Value& getLowerValueObject() noexcept { return currentLowerValue_; }
  Value& getUpperValueObject() noexcept { return currentUpperValue_; }

  void setLowerValue(double newValue, NotificationType notification) {
    newValue = normRange_.snapToLegalValue(newValue);
    newValue = lowerNormRange_.snapToLegalValue(newValue);
    if (newValue != lowerValue_) {
      lowerValue_ = newValue;
      if (currentLowerValue_ != newValue) {
        currentLowerValue_ = newValue;
      }
      repaint();
      triggerChangeMessage(notification);
    }
  }

  void setUpperValue(double newValue, NotificationType notification) {
    newValue = normRange_.snapToLegalValue(newValue);
    newValue = upperNormRange_.snapToLegalValue(newValue);
    if (newValue != upperValue_) {
      upperValue_ = newValue;
      if (currentUpperValue_ != newValue) {
        currentUpperValue_ = newValue;
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
    listeners_.callChecked(
        checker, [this](Listener& l) { l.sliderRangeValueChanged(this); });
    if (checker.shouldBailOut()) {
      return;
    }
  }

  void valueChanged(Value& value) override {
    if (value.refersToSameSourceAs(currentLowerValue_)) {
      setLowerValue(currentLowerValue_.getValue(), dontSendNotification);
    }
    if (value.refersToSameSourceAs(currentUpperValue_)) {
      setUpperValue(currentUpperValue_.getValue(), dontSendNotification);
    }
  }

  float getLowerValue() { return currentLowerValue_.getValue(); }
  float getUpperValue() { return currentUpperValue_.getValue(); }

  class Listener {
   public:
    virtual ~Listener() = default;

    virtual void sliderRangeValueChanged(EarSliderRange* range) = 0;
    virtual void sliderRangeLowerDragStarted(EarSliderRange* range) {}
    virtual void sliderRangeUpperDragStarted(EarSliderRange* range) {}
    virtual void sliderRangeLowerDragEnded(EarSliderRange* range) {}
    virtual void sliderRangeUpperDragEnded(EarSliderRange* range) {}
  };

  void addListener(Listener* l) { listeners_.add(l); }
  void removeListener(Listener* l) { listeners_.remove(l); }

 private:
  bool isDoubleClickEnabled_;
  float lowerDefault_ = 0.f;
  float upperDefault_ = 0.f;
  float lowerValue_ = 0.f;
  float upperValue_ = 0.f;

  Value currentLowerValue_;
  Value currentUpperValue_;
  NormalisableRange<double> normRange_{0.0, 1.0};
  NormalisableRange<double> lowerNormRange_{0.0, 1.0};
  NormalisableRange<double> upperNormRange_{0.0, 1.0};

  bool lowerMouseDragActive_;
  bool upperMouseDragActive_;

  std::vector<Tick> ticks_;

  float trackPosY_;
  float tickPosStartY_;
  float tickPosEndY_;
  float tickLabelPosY_;

  const float paddingLeft_ = 8.f;
  const float paddingRight_ = 10.f;
  const float trackWidth_ = 5.f;
  const float thumbRadius_ = 7.5f;

  float tickLength_ = 7.f;
  float tickMargin_ = 3.f;
  float tickWidth_ = 0.5f;
  float tickLabelWidth_ = 50.f;
  float tickLabelHeight_ = 12.f;

  ListenerList<Listener> listeners_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarSliderRange)
};  // namespace ui

}  // namespace ui
}  // namespace plugin
}  // namespace ear
