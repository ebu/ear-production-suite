#pragma once

#include "JuceHeader.h"
#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class OrientationView : public Component,
                      public AsyncUpdater,
                      private Value::Listener {
 public:
  OrientationView(float startRad, float endRad, float startVal, float endVal, float defaultVal, juce::String centreLabel, juce::String jointLabel) {
    setColour(backgroundColourId, EarColours::Transparent);
    setColour(trackColourId, EarColours::SliderTrack);
    setColour(highlightColourId, EarColours::Item01);
    setColour(angleLabelColourId, EarColours::Text.withAlpha(Emphasis::medium));
    setColour(directionLabelColourId,
              EarColours::Text.withAlpha(Emphasis::high));

    double fullCircWithTolerance = MathConstants<double>::twoPi - (MathConstants<double>::pi / 18000.0);
    fullCircle = (startRad == endRad || radsBetween(startRad, endRad) >= fullCircWithTolerance);

    if(fullCircle) {
      arcStartPos_ = unwrapRads(startRad);
      arcEndPos_ = arcStartPos_ + MathConstants<double>::twoPi;

    } else {
      arcStartPos_ = unwrapRads(startRad);
      arcEndPos_ = unwrapRads(endRad);
      while(arcEndPos_ < arcStartPos_) {
        arcEndPos_ += MathConstants<double>::twoPi;
      }
    }

    arcStartVal_ = startVal;
    arcEndVal_ = endVal;
    normRange_ = NormalisableRange<double> {std::min(startVal, endVal), std::max(startVal, endVal)};
    valueDefault_ = defaultVal;
    setValue(valueDefault_, dontSendNotification);
    centreLabelText_ = centreLabel;
    jointLabelText_ = jointLabel;

    currentValue_.addListener(this);
  }

  ~OrientationView() {
    currentValue_.removeListener(this);
  }

  void paint(Graphics& g) override {
    // background
    //g.fillAll(juce::Colours::grey);
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

    // ticks
    g.setColour(findColour(trackColourId));
    g.drawLine(ccwTick_, tickWidth_);
    g.drawLine(centreTick_, tickWidth_);
    g.drawLine(cwTick_, tickWidth_);
    if(fullCircle) g.drawLine(jointTick_, tickWidth_);

    // highlighted arc
    g.setColour(findColour(highlightColourId));
    Path p;
    float arcEndPos = (arcEndPos_ < arcStartPos_) ? arcEndPos_ + MathConstants<float>::twoPi : arcEndPos_;
    p.addArc(getWidth() / 2.f - outerDiameter_ / 2.f,
             getHeight() / 2.f - outerDiameter_ / 2.f, outerDiameter_,
             outerDiameter_, arcStartPos_, arcEndPos, true);
    g.strokePath(p, PathStrokeType(trackWidth_));

    // thumb
    g.setColour(findColour(highlightColourId));
    float xPos = -std::sin(handlePos_) * outerDiameter_ / 2.f;
    float yPos = std::cos(handlePos_) * outerDiameter_ / 2.f;
    g.fillEllipse(getWidth() / 2.f - xPos - knobSize_ / 2.f,
                  getHeight() / 2.f - yPos - knobSize_ / 2.f, knobSize_,
                  knobSize_);

    // directional line
    g.setColour(findColour(trackColourId));
    auto tps = getPointOnCentredCircle(outerDiameter_, handlePos_);
    auto tpe = getPointOnCentredCircle(outerDiameter_, handlePos_ + MathConstants<float>::pi);
    g.drawLine(tps.getX(), tps.getY(), tpe.getX(), tpe.getY());

    // labeling
    g.setColour(findColour(angleLabelColourId));
    g.setFont(EarFonts::Measures);
    g.drawText(ccwTickLabelText_, ccwTickLabelRect_, Justification::centred);
    g.drawText(centreTickLabelText_, centreTickLabelRect_, Justification::centred);
    g.drawText(cwTickLabelText_, cwTickLabelRect_, Justification::centred);
    if(fullCircle) g.drawText(jointTickLabelText_, jointTickLabelRect_, Justification::centred);

    g.setColour(findColour(directionLabelColourId));
    g.setFont(EarFonts::Values);
    if(centreLabelText_.isNotEmpty()) g.drawText(centreLabelText_, centreLabelRect_, Justification::centred);
    if(fullCircle && jointLabelText_.isNotEmpty()) g.drawText(jointLabelText_, jointLabelRect_, Justification::centred);
  }

  void resized() override {
    activeArea_.clear();
    activeArea_.addEllipse(
        getWidth() / 2.f -
            (outerDiameter_ + outerTrackWidth_ + knobSize_) / 2.f,
        getHeight() / 2.f -
            (outerDiameter_ + outerTrackWidth_ + knobSize_) / 2.f,
        outerDiameter_ + outerTrackWidth_ + knobSize_,
        outerDiameter_ + outerTrackWidth_ + knobSize_);

    if(fullCircle) {
      // Full Circle
      jointTick_ = Line<float>(getPointOnCentredCircle(outerDiameter_, arcStartPos_), getPointOnCentredCircle(outerDiameter_ + tickLength_ + tickLength_, arcStartPos_));
      ccwTick_ = Line<float>(getPointOnCentredCircle(outerDiameter_, arcStartPos_ + MathConstants<float>::halfPi), getPointOnCentredCircle(outerDiameter_ + tickLength_ + tickLength_, arcStartPos_ + MathConstants<float>::halfPi));
      centreTick_ = Line<float>(getPointOnCentredCircle(outerDiameter_, arcStartPos_+ MathConstants<float>::pi), getPointOnCentredCircle(outerDiameter_ + tickLength_ + tickLength_, arcStartPos_ + MathConstants<float>::pi));
      cwTick_ = Line<float>(getPointOnCentredCircle(outerDiameter_, arcStartPos_ - MathConstants<float>::halfPi), getPointOnCentredCircle(outerDiameter_ + tickLength_ + tickLength_, arcStartPos_ - MathConstants<float>::halfPi ));

      float tickVal;
      float tickPos;
      float valRange = arcEndVal_ - arcStartVal_;

      tickVal = (arcStartVal_ < 0.f && arcEndVal_ >= 0.0)? arcEndVal_ : arcStartVal_;
      tickPos = (MathConstants<float>::halfPi * 0.0f) + arcStartPos_;
      jointTickLabelText_ = String(tickVal);
      setLabelsSizeAndPosition(tickPos, jointTickLabelRect_, jointTickLabelText_, jointLabelRect_, jointLabelText_);

      tickVal = (valRange * 0.25f) + arcStartVal_;
      tickPos = (MathConstants<float>::halfPi * 1.0f) + arcStartPos_;
      ccwTickLabelText_ = String(tickVal);
      setLabelsSizeAndPosition(tickPos, ccwTickLabelRect_, ccwTickLabelText_, juce::Rectangle<float>{}, juce::String{});

      tickVal = (valRange * 0.5f) + arcStartVal_;
      tickPos = (MathConstants<float>::halfPi * 2.0f) + arcStartPos_;
      centreTickLabelText_ = String(tickVal);
      setLabelsSizeAndPosition(tickPos, centreTickLabelRect_, centreTickLabelText_, centreLabelRect_, centreLabelText_);

      tickVal = (valRange * 0.75f) + arcStartVal_;
      tickPos = (MathConstants<float>::halfPi * 3.0f) + arcStartPos_;
      cwTickLabelText_ = String(tickVal);
      setLabelsSizeAndPosition(tickPos, cwTickLabelRect_, cwTickLabelText_, juce::Rectangle<float>{}, juce::String{});

    } else {
      // Arc
      float halfArcRange = radsBetween(arcStartPos_, arcEndPos_) / 2.f;
      ccwTick_ = Line<float>(getPointOnCentredCircle(outerDiameter_, arcStartPos_), getPointOnCentredCircle(outerDiameter_ + tickLength_ + tickLength_, arcStartPos_));
      centreTick_ = Line<float>(getPointOnCentredCircle(outerDiameter_, arcStartPos_+ halfArcRange), getPointOnCentredCircle(outerDiameter_ + tickLength_ + tickLength_, arcStartPos_ + halfArcRange));
      cwTick_ = Line<float>(getPointOnCentredCircle(outerDiameter_, arcEndPos_), getPointOnCentredCircle(outerDiameter_ + tickLength_ + tickLength_, arcEndPos_));

      float tickVal;
      float tickPos;
      float valRange = arcEndVal_ - arcStartVal_;
      float posRange = radsBetween(arcStartPos_, arcEndPos_);

      tickVal = arcStartVal_;
      tickPos = arcStartPos_;
      ccwTickLabelText_ = String(tickVal);
      setLabelsSizeAndPosition(tickPos, ccwTickLabelRect_, ccwTickLabelText_, juce::Rectangle<float>{}, juce::String{});

      tickVal = (valRange * 0.5f) + arcStartVal_;
      tickPos = (posRange * 0.5f) + arcStartPos_;
      centreTickLabelText_ = String(tickVal);
      setLabelsSizeAndPosition(tickPos, centreTickLabelRect_, centreTickLabelText_, centreLabelRect_, centreLabelText_);

      tickVal = arcEndVal_;
      tickPos = arcEndPos_;
      cwTickLabelText_ = String(tickVal);
      setLabelsSizeAndPosition(tickPos, cwTickLabelRect_, cwTickLabelText_, juce::Rectangle<float>{}, juce::String{});

    }
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
      setValue(valueDefault_, sendNotificationSync);
    }
  }

  void mouseDrag(const MouseEvent& event) override {
    if (isEnabled() && mouseDragActive_) {
      auto position = event.getPosition();
      const auto posRel = juce::Point<float>(
          getWidth() / 2.f - static_cast<float>(position.getX()),
          getHeight() / 2.f - static_cast<float>(position.getY()));
      double posRad = std::atan2(-posRel.getX(), posRel.getY());

      handlePos_ = unwrapRadsToBounds(posRad, arcStartPos_, arcEndPos_, true);

      double newValue = handlePosToValue(handlePos_);
      setValue(newValue, sendNotificationSync);
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
    if (value.refersToSameSourceAs(currentValue_)) {
      setValue(currentValue_.getValue(), dontSendNotification);
    }
  }

  enum ColourIds {
    backgroundColourId = 0x00010001,
    trackColourId = 0x00010002,
    highlightColourId = 0x00010003,
    directionLabelColourId = 0x00010004,
    angleLabelColourId = 0x00010005,
  };

  class Listener {
   public:
    virtual ~Listener() = default;

    virtual void pannerValueChanged(OrientationView* panner) = 0;
    virtual void pannerDragStarted(OrientationView* panner) = 0;
    virtual void pannerDragEnded(OrientationView* panner) = 0;
  };

  void addListener(Listener* l) { listeners_.add(l); }
  void removeListener(Listener* l) { listeners_.remove(l); }

private:

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

  Point<float> getPointOnRect(float angle, float w, float h) {
    Point<float> intersection;

    float xRad = h / 2.f;
    float yRad = w / 2.f;

    float tangent = tan(angle);
    float y = xRad * tangent;

      if (fabs(y) <= yRad) {

        if (angle < (float_Pi / 2.f) || angle > 3.f * (float_Pi / 2.f)) {
          intersection.x = xRad;
          intersection.y = y;
        } else {
          intersection.x = -xRad;
          intersection.y = -y;
        }
      } else {
        float x = yRad / tangent;

          if (angle < float_Pi) {
            intersection.x = x;
            intersection.y = yRad;
          } else {
            intersection.x = -x;
            intersection.y = -yRad;
          }
      }

      return intersection;
  }

  float getRectangeCentreToPerimeterDistance(float angle, float w, float h) {
    Point<float> intersection = getPointOnRect(angle, w, h);
    return sqrt((intersection.x * intersection.x) + (intersection.y * intersection.y));
  }

  Point<float> getPointOnCentredCircle(float dia, float rads) {
    return Point<float>((getWidth() / 2.f), (getHeight() / 2.f)).getPointOnCircumference(dia / 2.f, rads);
  }

  double radsBetween(double a, double b) {
    return std::abs(a - b);
  }

  double circRadsBetween(double a, double b) {
    return std::min(
      std::abs(a - b),
      std::min(
        std::abs(a - b - MathConstants<double>::twoPi),
        std::abs(a - b + MathConstants<double>::twoPi)
      )
    );
  }

  double unwrapRads(double rads) {
    while(rads < 0.f) {
      rads += MathConstants<double>::twoPi;
    }
    while(rads > MathConstants<double>::twoPi) {
      rads -= MathConstants<double>::twoPi;
    }
    return rads;
  }

  double unwrapRadsToBounds(double rads, double boundA, double boundB, bool clampToBounds = false) {
    double minRad = std::min(boundA, boundB);
    double maxRad = std::max(boundA, boundB);

    while(rads < minRad) {
      rads += MathConstants<double>::twoPi;
    }
    while(rads > maxRad) {
      rads -= MathConstants<double>::twoPi;
    }

    if(rads >= minRad) {
      // Now in bounds
      return rads;
    }

    // Not fitting in bounds (rads is now < minRad)
    double lowerVal = rads;
    double upperVal = rads + MathConstants<double>::twoPi;
    double lowerValDist = radsBetween(lowerVal, minRad);
    double upperValDist = radsBetween(upperVal, maxRad);

    if(clampToBounds) {
      return (lowerValDist < upperValDist) ? minRad : maxRad;
    }

    return (lowerValDist < upperValDist) ? lowerVal : upperVal;
  }

  double handlePosToValue(double handlePos) {
    double posRange = arcEndPos_ - arcStartPos_;
    double valRange = arcEndVal_ - arcStartVal_;

    handlePos = unwrapRadsToBounds(handlePos, arcStartPos_, arcEndPos_);
    double curPosRangeProportion = (handlePos - arcStartPos_) / posRange;

    double ret = (valRange * curPosRangeProportion) + arcStartVal_;
    return ret;
  }

  double valueToHandlePos(double val) {
    double posRange = arcEndPos_ - arcStartPos_;
    double valRange = arcEndVal_ - arcStartVal_;

    double curValRangeProportion = (val - arcStartVal_) / valRange;
    double ret = (posRange * curValRangeProportion) + arcStartPos_;
    return ret;
  }

  void setLabelsSizeAndPosition(float atRad, juce::Rectangle<float> &valLabelRect, juce::String &valLabelStr, juce::Rectangle<float> &txtLabelRect, juce::String &txtLabelStr) {
    atRad = unwrapRads(atRad);

    float labelWidth = EarFonts::Measures.getStringWidthFloat(valLabelStr);
    if(txtLabelStr.isNotEmpty()) {
      labelWidth = std::max(labelWidth, EarFonts::Values.getStringWidthFloat(txtLabelStr));
    }

    valLabelRect.setSize(labelWidth, labelHeight_);
    txtLabelRect.setSize(labelWidth, labelHeight_);
    float hyp = getRectangeCentreToPerimeterDistance(atRad, valLabelRect.getWidth(), valLabelRect.getHeight());
    valLabelRect.setCentre(getPointOnCentredCircle(outerDiameter_ + ((tickLength_ + hyp + labelSeperation_) * 2.f), atRad));

    if(atRad > (MathConstants<float>::pi * 0.51f) && atRad < (MathConstants<float>::pi * 1.49f)) {
      // txtLabels in bottom half sit under value label
      txtLabelRect.setCentre(valLabelRect.getCentreX(), valLabelRect.getCentreY() + labelSeperation_ + labelHeight_);
    } else {
      txtLabelRect.setCentre(valLabelRect.getCentreX(), valLabelRect.getCentreY() - labelSeperation_ - labelHeight_);
    }
  }

  Value& getValueObject() noexcept { return currentValue_; }

  float getValue() { return currentValue_.getValue(); }

  void setValue(double newValue, NotificationType notification) {
    newValue = normRange_.snapToLegalValue(newValue);
    if (currentValue_ != newValue) {
      currentValue_ = newValue;
      handlePos_ = valueToHandlePos(newValue);
      repaint();
      triggerChangeMessage(notification);
    }
  }


 private:
  bool mouseDragActive_;
  Path activeArea_;

  double valueDefault_ = 0.f;
  Value currentValue_;
  NormalisableRange<double> normRange_;

  Line<float> ccwTick_;
  juce::Rectangle<float> ccwTickLabelRect_;
  juce::String ccwTickLabelText_;
  Line<float> centreTick_;
  juce::Rectangle<float> centreTickLabelRect_;
  juce::String centreTickLabelText_;
  Line<float> cwTick_;
  juce::Rectangle<float> cwTickLabelRect_;
  juce::String cwTickLabelText_;
  Line<float> jointTick_;
  juce::Rectangle<float> jointTickLabelRect_;
  juce::String jointTickLabelText_;

  juce::Rectangle<float> centreLabelRect_;
  juce::String centreLabelText_{ "Forward" };
  juce::Rectangle<float> jointLabelRect_;
  juce::String jointLabelText_;

  const float outerDiameter_ = 100.f;
  const float trackWidth_ = 1.f;
  const float outerTrackWidth_ = 3.f;
  const float tickLength_ = 7.f;
  const float tickWidth_ = 1.f;
  const float labelHeight_ = 12.f;
  const float labelSeperation_ = 2.f;
  const float knobSize_ = 10.f;
  const float crossSize_ = 14.f;
  const float crossWidth_ = 1.f;

  double arcStartPos_ = 0.0;
  double arcEndPos_ = 0.0;
  double arcStartVal_ = -180.0;
  double arcEndVal_ = 180.0;
  bool fullCircle = false;

  double handlePos_ = 0.0;


  ListenerList<Listener> listeners_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrientationView)
};  // namespace ui

}  // namespace ui
}  // namespace plugin
}  // namespace ear
