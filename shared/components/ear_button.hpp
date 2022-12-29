#pragma once

#include "JuceHeader.h"

#include "../helper/graphics.hpp"
#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"
#include "look_and_feel/shadows.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class EarButton : public Button {
 public:
  EarButton() : EarButton(String()) {}

  explicit EarButton(const String& name)
      : Button(name),
        font_(EarFontsSingleton::instance().Items),
        justification_(Justification::centred),
        iconAlignment_(IconAlignment::centred),
        shape_(Shape::Rounded) {
    setColour(textColourId, EarColours::Text);
    setColour(highlightColourId, EarColours::Primary);
    setColour(backgroundColourId, EarColours::Area01dp);
    setColour(hoverColourId, EarColours::Area04dp);
  }

  enum IconAlignment { left, centred, right };

  enum ColourIds {
    backgroundColourId = 0x1020100,
    textColourId = 0x10201001,
    highlightColourId = 0x1020102,
    hoverColourId = 0x1020103,
  };

  void setFont(const Font& newFont) {
    if (font_ != newFont) {
      font_ = newFont;
      repaint();
    }
  }

  Font getFont() const noexcept { return font_; }

  enum Shape { Rounded, Rectangular, Circular, Toggle };

  void setShape(Shape newShape) {
    if (shape_ != newShape) {
      shape_ = newShape;
      if (newShape == Shape::Toggle) {
        setJustification(Justification::left);
      }
      repaint();
    }
  }

  Shape getShape() const noexcept { return shape_; }

  void setJustification(Justification justification) {
    justification_ = justification;
  }

  void setIconAlignment(IconAlignment iconAlignment) {
    iconAlignment_ = iconAlignment;
  }

  void paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted,
                   bool shouldDrawButtonAsDown) override {
    auto area = getLocalBounds().toFloat();
    if (getToggleState()) {
      g.setColour(findColour(highlightColourId));
    } else {
      g.setColour(findColour(backgroundColourId));
    }
    switch (shape_) {
      case Shape::Rounded:
        fillRoundedRectangle(g, area, 4.f);
        break;
      case Shape::Rectangular:
        g.fillRect(area);
        break;
      case Shape::Circular: {
        float circleSize = std::min(getWidth(), getHeight());
        g.fillEllipse(area.withSizeKeepingCentre(circleSize, circleSize));
      } break;
      case Shape::Toggle: {
        const float thumbRadius_ = 7.f;
        const float trackWidth_ = 24.f;
        const float trackHeight_ = 12.f;
        const float margin_ = 5.f;
        const float yCentre = getHeight() / 2.f;
        Path trackPath;
        trackPath.addArc(0.f, 0.f, trackHeight_, trackHeight_,
                         MathConstants<float>::pi, MathConstants<float>::twoPi,
                         true);

        trackPath.lineTo(trackWidth_ - trackHeight_, 0.f);
        trackPath.addArc(trackWidth_ - trackHeight_, 0.f, trackHeight_,
                         trackHeight_, 0.f, MathConstants<float>::pi);
        trackPath.closeSubPath();
        trackPath.applyTransform(AffineTransform::translation(
            margin_, yCentre - trackHeight_ / 2.f));

        if (getToggleState()) {
          g.setColour(findColour(highlightColourId));
        } else {
          g.setColour(findColour(textColourId).withAlpha(Emphasis::medium));
        }
        g.fillPath(trackPath);

        Path thumbPath;
        if (getToggleState()) {
          thumbPath.addEllipse(margin_ + trackWidth_ - trackHeight_,
                               yCentre - thumbRadius_, 2.f * thumbRadius_,
                               2.f * thumbRadius_);
        } else {
          thumbPath.addEllipse(margin_ + trackHeight_ / 2.f - thumbRadius_,
                               yCentre - thumbRadius_, 2.f * thumbRadius_,
                               2.f * thumbRadius_);
        }
        Shadows::thumb.drawForPath(g, thumbPath);
        g.setColour(findColour(textColourId));
        g.fillPath(thumbPath);
      }
    }

    auto buttonTextArea = getLocalBounds().toFloat();
    juce::Rectangle<float> iconArea;
    if (offStateIcon_ || onStateIcon_ || hoverStateIcon_) {
      switch (iconAlignment_) {
        case IconAlignment::left:
          iconArea = buttonTextArea.removeFromLeft(getHeight());
          break;
        case IconAlignment::centred:
          buttonTextArea = area;
          iconArea =
              area.withSizeKeepingCentre(area.getHeight(), area.getHeight());
          break;
        case IconAlignment::right:
          iconArea = buttonTextArea.removeFromRight(getHeight());
          break;
      }
    }
    if (shouldDrawButtonAsHighlighted) {
      g.setColour(findColour(hoverColourId));
      switch (shape_) {
        case Shape::Rounded:
          fillRoundedRectangle(g, area, 4.f);
          break;
        case Shape::Rectangular:
          g.fillRect(area);
          break;
        case Shape::Circular: {
          float circleSize = std::min(getWidth(), getHeight());
          g.fillEllipse(area.withSizeKeepingCentre(circleSize, circleSize));
        } break;
        case Shape::Toggle:
          break;
      }
    }

    if (shouldDrawButtonAsHighlighted && hoverStateIcon_) {
      hoverStateIcon_->drawWithin(
          g, iconArea,
          RectanglePlacement::centred | RectanglePlacement::doNotResize,
          hoverStateIconAlpha_);
    } else if (getToggleState() && onStateIcon_) {
      onStateIcon_->drawWithin(
          g, iconArea,
          RectanglePlacement::centred | RectanglePlacement::doNotResize,
          onStateIconAlpha_);
    } else if (!getToggleState() && offStateIcon_) {
      offStateIcon_->drawWithin(
          g, iconArea,
          RectanglePlacement::centred | RectanglePlacement::doNotResize,
          offStateIconAlpha_);
    }

    g.setColour(findColour(textColourId));
    g.setFont(getFont());
    switch (shape_) {
      case Shape::Toggle:
        g.drawText(getButtonText(), buttonTextArea.withTrimmedLeft(40.f),
                   justification_);
        break;
      default:
        g.drawText(getButtonText(), buttonTextArea, justification_);
        break;
    }
  }

  MouseCursor getMouseCursor() override {
    if (isEnabled()) {
      return MouseCursor::PointingHandCursor;
    }
    return MouseCursor::NormalCursor;
  }

  void setOnStateIcon(std::unique_ptr<Drawable> onStateIcon,
                      float alpha = 1.f) {
    onStateIcon_ = std::move(onStateIcon);
    onStateIconAlpha_ = alpha;
  }

  void setOffStateIcon(std::unique_ptr<Drawable> offStateIcon,
                       float alpha = 1.f) {
    offStateIcon_ = std::move(offStateIcon);
    offStateIconAlpha_ = alpha;
  }

  void setHoverStateIcon(std::unique_ptr<Drawable> hoverStateIcon,
                         float alpha = 1.f) {
    hoverStateIcon_ = std::move(hoverStateIcon);
    hoverStateIconAlpha_ = alpha;
  }

  void colourChanged() override { repaint(); }

 private:
  Font font_;
  Shape shape_;
  Justification justification_;
  IconAlignment iconAlignment_;

  std::unique_ptr<Drawable> onStateIcon_;
  float onStateIconAlpha_;
  std::unique_ptr<Drawable> offStateIcon_;
  float offStateIconAlpha_;
  std::unique_ptr<Drawable> hoverStateIcon_;
  float hoverStateIconAlpha_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarButton)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
