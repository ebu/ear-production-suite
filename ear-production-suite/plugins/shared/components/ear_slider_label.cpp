#include "ear_slider_label.hpp"

#include "ear_inverted_slider.hpp"
#include "look_and_feel/colours.hpp"

using namespace ear::plugin::ui;

EarSliderLabel::EarSliderLabel(Slider& s) : Label(), parentSlider_(s) {
  setColour(EarSliderLabel::unitTextColourId, EarColours::Text.withAlpha(0.6f));
}
EarSliderLabel::~EarSliderLabel() {}

void EarSliderLabel::textWasChanged() {
  // XXX: never really worked how it should and does not work at all on windows
  //      -> disabled for now
  // if (grabFocus_) {
  //   grabKeyboardFocus();
  // }
}

void EarSliderLabel::setGrabFocusOnTextChange(bool grabFocus) {
  grabFocus_ = grabFocus;
}
bool EarSliderLabel::getGrabFocusOnTextChange() { return grabFocus_; }

void EarSliderLabel::mouseDown(const MouseEvent& event) {
  dragStartValue_ = parentSlider_.getValue();
  dragStartPosY_ = event.getPosition().getY();
  Component::BailOutChecker checker(&parentSlider_);
  listeners_.callChecked(checker, [&](Slider::Listener& l) {
    l.sliderDragStarted(&(this->parentSlider_));
  });
  if (checker.shouldBailOut()) {
    return;
  }
}

void EarSliderLabel::mouseDrag(const MouseEvent& event) {
  if (isEnabled()) {
    int posDifference = dragStartPosY_ - event.getPosition().getY();
    if (dynamic_cast<EarInvertedSlider*>(&parentSlider_) != nullptr) {
      posDifference *= -1;
    }
    parentSlider_.setValue(dragStartValue_ + posDifference * dragFactor_ *
                                                 (parentSlider_.getMaximum() -
                                                  parentSlider_.getMinimum()),
                           juce::NotificationType::dontSendNotification);
    Component::BailOutChecker checker(&parentSlider_);
    listeners_.callChecked(checker, [this](Slider::Listener& l) {
      l.sliderValueChanged(&(this->parentSlider_));
    });
    if (checker.shouldBailOut()) {
      return;
    }
  }
};

void EarSliderLabel::mouseUp(const MouseEvent& event) {
  Label::mouseUp(event);
  Component::BailOutChecker checker(&parentSlider_);
  listeners_.callChecked(checker, [this](Slider::Listener& l) {
    l.sliderDragEnded(&(this->parentSlider_));
  });

  if (checker.shouldBailOut()) {
    return;
  }
}

void EarSliderLabel::setUnit(const String& unit) { unit_ = unit; }
String EarSliderLabel::getUnit() { return unit_; }

void EarSliderLabel::setUnitFont(const Font& newFont) { unitFont_ = newFont; }
Font EarSliderLabel::getUnitFont() { return unitFont_; }

MouseCursor EarSliderLabel::getMouseCursor() {
  if (isEnabled()) {
    return MouseCursor::UpDownResizeCursor;
  }
  return MouseCursor::NormalCursor;
}

void EarSliderLabel::paint(Graphics& g) {
  g.fillAll(findColour(Label::backgroundColourId));

  if (unit_.isNotEmpty()) {
    g.setFont(getUnitFont());
    g.setColour(findColour(unitTextColourId));
    g.drawText(unit_, getLocalBounds().withTrimmedTop(20),
               Justification::centred);
  }

  if (!isBeingEdited()) {
    g.setColour(findColour(Label::textColourId));
    g.setFont(getFont());

    auto textArea = getBorderSize().subtractedFrom(getLocalBounds());

    g.drawFittedText(
        getText(), textArea, getJustificationType(),
        jmax(1, (int)(textArea.getHeight() / getFont().getHeight())),
        getMinimumHorizontalScale());
  }
  if (hasKeyboardFocus(true)) {
    g.setColour(findColour(Label::outlineColourId));
    g.drawRect(getLocalBounds(), 1);
  }
}
