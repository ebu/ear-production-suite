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
   OrientationView() {
    setColour(backgroundColourId, EarColours::Transparent);
    setColour(trackColourId, EarColours::SliderTrack);
    setColour(highlightColourId, EarColours::Item01);
    setColour(angleLabelColourId, EarColours::Text.withAlpha(Emphasis::medium));
    setColour(directionLabelColourId,
              EarColours::Text.withAlpha(Emphasis::high));
    currentAzimuthValue_.addListener(this);
    currentDistanceValue_.addListener(this);
  }

  ~OrientationView() {
    currentAzimuthValue_.removeListener(this);
    currentDistanceValue_.removeListener(this);
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
    if(arcStartPos_ == arcEndPos_) g.drawLine(jointTick_, tickWidth_);

    // inner track
    const float innerDiameter = distance_ * outerDiameter_;
    g.setColour(findColour(highlightColourId));
    Path p;
    float arcEndPos = (arcEndPos_ < arcStartPos_) ? arcEndPos_ + MathConstants<float>::twoPi : arcEndPos_;
    p.addArc(getWidth() / 2.f - outerDiameter_ / 2.f,
             getHeight() / 2.f - outerDiameter_ / 2.f, outerDiameter_,
             outerDiameter_, arcStartPos_, arcEndPos, true);
    g.strokePath(p, PathStrokeType(trackWidth_));

    // thumb
    g.setColour(findColour(highlightColourId));
    float azimuthDeg = azimuth_ / 180.f * MathConstants<float>::pi;
    float xPos = -std::sin(handlePos_) * innerDiameter / 2.f;
    float yPos = std::cos(handlePos_) * innerDiameter / 2.f;
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
    if(arcStartPos_ == arcEndPos_) g.drawText(jointTickLabelText_, jointTickLabelRect_, Justification::centred);

    /*
    g.setColour(findColour(directionLabelColourId));
    g.setFont(EarFonts::Values);
    g.drawText(String("Front"), frontLabelRect_, Justification::centredBottom);
    g.drawText(String("Back"), backLabelRect_, Justification::centredTop);
    */
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

  float radsBetween(float a, float b) {
    return std::min(
      std::abs(a - b),
      std::min(
        std::abs(a - b - MathConstants<float>::twoPi),
        std::abs(a - b + MathConstants<float>::twoPi)
      )
    );
  }

  float handlePosToValue(float handlePos) {
    float posRangeMin = (arcStartPos_ > arcEndPos_) ? arcStartPos_ - MathConstants<float>::twoPi : arcStartPos_;
    float posRangeMax = arcEndPos_;
    float posRange = posRangeMax - posRangeMin;
    float valRange = arcEndVal_ - arcStartVal_;

    float curPos = (handlePos > arcEndPos_)? handlePos - MathConstants<float>::twoPi : handlePos;
    float curPosRangeProportion = (curPos - posRangeMin) / posRange;

    return (valRange * curPosRangeProportion) + arcStartVal_ ;
  }

  float valueToHandlePos(float val) {
    float posRangeMin = (arcStartPos_ > arcEndPos_) ? arcStartPos_ - MathConstants<float>::twoPi : arcStartPos_;
    float posRangeMax = arcEndPos_;
    float posRange = posRangeMax - posRangeMin;
    float valRange = arcEndVal_ - arcStartVal_;

    float curValRangeProportion = (val - arcStartVal_) / valRange;
    float handlePos = (posRange * curValRangeProportion) + posRangeMin;
    if(handlePos < 0.f) handlePos += MathConstants<float>::twoPi;
    if(handlePos > MathConstants<float>::twoPi) handlePos -= MathConstants<float>::twoPi;

    return handlePos;
  }

  void mouseDrag(const MouseEvent& event) override {
    if (isEnabled() && mouseDragActive_) {
      auto position = event.getPosition();
      const auto posRel = juce::Point<float>(
          getWidth() / 2.f - static_cast<float>(position.getX()),
          getHeight() / 2.f - static_cast<float>(position.getY()));
      float posRad = std::atan2(-posRel.getX(), posRel.getY());
      float posRadCirc = (posRad < 0.f) ? posRad + MathConstants<float>::twoPi : posRad;

      float arcRange = radsBetween(arcStartPos_, arcEndPos_);
      float arcOffset = arcStartPos_;
      if(arcStartPos_ > arcEndPos_) arcOffset = MathConstants<float>::twoPi - arcStartPos_;

      bool inBounds = false;
      if(arcStartPos_ > arcEndPos_) {
        if(posRadCirc >= arcStartPos_ && posRadCirc >= arcEndPos_) inBounds = true;
        if(posRadCirc <= arcStartPos_ && posRadCirc <= arcEndPos_) inBounds = true;
      } else if(arcStartPos_ < arcEndPos_){
        if(posRadCirc >= arcStartPos_ && posRadCirc <= arcEndPos_) inBounds = true;
      } else {
        inBounds = true;
      }

      if(inBounds) {
        handlePos_ = posRadCirc;
      } else {
        float radsToStart = radsBetween(posRadCirc, arcStartPos_);
        float radsToEnd = radsBetween(posRadCirc, arcEndPos_);
        handlePos_ = (radsToStart < radsToEnd) ? arcStartPos_ : arcEndPos_;
      }







      float newAzimuth = std::atan2(posRel.getX(), posRel.getY()) * 180.f / MathConstants<float>::pi;
      setAzimuth(newAzimuth, sendNotificationSync);
    }
  }

  Point<float> getPointOnCentredCircle(float dia, float rads) {
    return Point<float>((getWidth() / 2.f), (getHeight() / 2.f)).getPointOnCircumference(dia / 2.f, rads);
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

    if(arcStartPos_ == arcEndPos_) {
      // Full Circle
      jointTick_ = Line<float>(getPointOnCentredCircle(outerDiameter_, arcStartPos_), getPointOnCentredCircle(outerDiameter_ + tickLength_ + tickLength_, arcStartPos_));
      ccwTick_ = Line<float>(getPointOnCentredCircle(outerDiameter_, arcStartPos_ + MathConstants<float>::halfPi), getPointOnCentredCircle(outerDiameter_ + tickLength_ + tickLength_, arcStartPos_ + MathConstants<float>::halfPi));
      centreTick_ = Line<float>(getPointOnCentredCircle(outerDiameter_, arcStartPos_+ MathConstants<float>::pi), getPointOnCentredCircle(outerDiameter_ + tickLength_ + tickLength_, arcStartPos_ + MathConstants<float>::pi));
      cwTick_ = Line<float>(getPointOnCentredCircle(outerDiameter_, arcStartPos_ - MathConstants<float>::halfPi), getPointOnCentredCircle(outerDiameter_ + tickLength_ + tickLength_, arcStartPos_ - MathConstants<float>::halfPi ));

      float hyp;
      float tickVal;
      float tickPos;
      float valRange = arcEndVal_ - arcStartVal_;

      tickVal = (valRange * 0.0f) + arcStartVal_;
      tickPos = (MathConstants<float>::halfPi * 0.0f) + arcStartPos_;
      jointTickLabelText_ = String(tickVal);
      jointTickLabelRect_.setSize(EarFonts::Measures.getStringWidthFloat(jointTickLabelText_), labelHeight_);
      hyp = getRectangeCentreToPerimeterDistance(tickPos, jointTickLabelRect_.getWidth(), jointTickLabelRect_.getHeight());
      jointTickLabelRect_.setCentre(getPointOnCentredCircle(outerDiameter_ + ((tickLength_ + hyp + 5.f) * 2.f), tickPos));

      tickVal = (valRange * 0.25f) + arcStartVal_;
      tickPos = (MathConstants<float>::halfPi * 1.0f) + arcStartPos_;
      ccwTickLabelText_ = String(tickVal);
      ccwTickLabelRect_.setSize(EarFonts::Measures.getStringWidthFloat(ccwTickLabelText_), labelHeight_);
      hyp = getRectangeCentreToPerimeterDistance(tickPos, ccwTickLabelRect_.getWidth(), ccwTickLabelRect_.getHeight());
      ccwTickLabelRect_.setCentre(getPointOnCentredCircle(outerDiameter_ + ((tickLength_ + hyp + 5.f) * 2.f), tickPos));

      tickVal = (valRange * 0.5f) + arcStartVal_;
      tickPos = (MathConstants<float>::halfPi * 2.0f) + arcStartPos_;
      centreTickLabelText_ = String(tickVal);
      centreTickLabelRect_.setSize(EarFonts::Measures.getStringWidthFloat(centreTickLabelText_), labelHeight_);
      hyp = getRectangeCentreToPerimeterDistance(tickPos, centreTickLabelRect_.getWidth(), centreTickLabelRect_.getHeight());
      centreTickLabelRect_.setCentre(getPointOnCentredCircle(outerDiameter_ + ((tickLength_ + hyp + 5.f) * 2.f), tickPos));

      tickVal = (valRange * 0.75f) + arcStartVal_;
      tickPos = (MathConstants<float>::halfPi * 3.0f) + arcStartPos_;
      cwTickLabelText_ = String(tickVal);
      cwTickLabelRect_.setSize(EarFonts::Measures.getStringWidthFloat(cwTickLabelText_), labelHeight_);
      hyp = getRectangeCentreToPerimeterDistance(tickPos, cwTickLabelRect_.getWidth(), cwTickLabelRect_.getHeight());
      cwTickLabelRect_.setCentre(getPointOnCentredCircle(outerDiameter_ + ((tickLength_ + hyp + 5.f) * 2.f), tickPos));

    } else {
      // Arc
      float halfArcRange = radsBetween(arcStartPos_, arcEndPos_) / 2.f;
      ccwTick_ = Line<float>(getPointOnCentredCircle(outerDiameter_, arcStartPos_), getPointOnCentredCircle(outerDiameter_ + tickLength_ + tickLength_, arcStartPos_));
      centreTick_ = Line<float>(getPointOnCentredCircle(outerDiameter_, arcStartPos_+ halfArcRange), getPointOnCentredCircle(outerDiameter_ + tickLength_ + tickLength_, arcStartPos_ + halfArcRange));
      cwTick_ = Line<float>(getPointOnCentredCircle(outerDiameter_, arcEndPos_), getPointOnCentredCircle(outerDiameter_ + tickLength_ + tickLength_, arcEndPos_));

      float hyp;
      float tickVal;
      float tickPos;
      float valRange = arcEndVal_ - arcStartVal_;
      float posRange = radsBetween(arcStartPos_, arcEndPos_);

      tickVal = arcStartVal_;
      tickPos = arcStartPos_;
      ccwTickLabelText_ = String(tickVal);
      ccwTickLabelRect_.setSize(EarFonts::Measures.getStringWidthFloat(ccwTickLabelText_), labelHeight_);
      hyp = getRectangeCentreToPerimeterDistance(tickPos, ccwTickLabelRect_.getWidth(), ccwTickLabelRect_.getHeight());
      ccwTickLabelRect_.setCentre(getPointOnCentredCircle(outerDiameter_ + ((tickLength_ + hyp + 5.f) * 2.f), tickPos));

      tickVal = (valRange * 0.5f) + arcStartVal_;
      tickPos = (posRange * 0.5f) + arcStartPos_;
      centreTickLabelText_ = String(tickVal);
      centreTickLabelRect_.setSize(EarFonts::Measures.getStringWidthFloat(centreTickLabelText_), labelHeight_);
      hyp = getRectangeCentreToPerimeterDistance(tickPos, centreTickLabelRect_.getWidth(), centreTickLabelRect_.getHeight());
      centreTickLabelRect_.setCentre(getPointOnCentredCircle(outerDiameter_ + ((tickLength_ + hyp + 5.f) * 2.f), tickPos));

      tickVal = arcEndVal_;
      tickPos = arcEndPos_;
      cwTickLabelText_ = String(tickVal);
      cwTickLabelRect_.setSize(EarFonts::Measures.getStringWidthFloat(cwTickLabelText_), labelHeight_);
      hyp = getRectangeCentreToPerimeterDistance(tickPos, cwTickLabelRect_.getWidth(), cwTickLabelRect_.getHeight());
      cwTickLabelRect_.setCentre(getPointOnCentredCircle(outerDiameter_ + ((tickLength_ + hyp + 5.f) * 2.f), tickPos));

    }

    /*
    frontLabelRect_ = juce::Rectangle<float>(
        getWidth() / 2.f - labelWidth_ / 2.f,
        getHeight() / 2.f - outerDiameter_ / 2.f - tickLength_ -
            2.f * padding_ - EarFonts::Measures.getAscent() - labelHeight_,
        labelWidth_, labelHeight_);
    backLabelRect_ = juce::Rectangle<float>(
        getWidth() / 2.f - labelWidth_ / 2.f,
        getHeight() / 2.f + outerDiameter_ / 2.f + tickLength_ +
            2.f * padding_ + EarFonts::Measures.getAscent(),
        labelWidth_, labelHeight_);
        */
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

    virtual void pannerValueChanged(OrientationView* panner) = 0;
    virtual void pannerDragStarted(OrientationView* panner) = 0;
    virtual void pannerDragEnded(OrientationView* panner) = 0;
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
  juce::String centreLabelText_;
  juce::Rectangle<float> jointLabelRect_;
  juce::String jointLabelText_;

  /*
  juce::Rectangle<float> frontLabelRect_;
  juce::Rectangle<float> backLabelRect_;
  */

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

  const float arcStartPos_ = (float_Pi * 2.f) - 1.0;
  const float arcEndPos_ = float_Pi - 1.0;
  const float arcStartVal_ = 90.f;
  const float arcEndVal_ = -90.f;

  float handlePos_ = 0.f;


  ListenerList<Listener> listeners_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrientationView)
};  // namespace ui

}  // namespace ui
}  // namespace plugin
}  // namespace ear
