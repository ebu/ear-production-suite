#pragma once

#include "JuceHeader.h"

namespace ear {
namespace plugin {
namespace ui {

inline Path createRoundedRectangle(float x, float y, float width, float height,
                                   float cornerSize) {
  float diameter = 2.f * cornerSize;
  Path p;
  bool shiftHorizontally = false;
  if (width < 0) {
    width = -width;
    shiftHorizontally = true;
  }
  bool shiftVertically = false;
  if (height < 0) {
    height = -height;
    shiftVertically = true;
  }
  p.addArc(x, y, diameter, diameter,
           MathConstants<float>::pi + MathConstants<float>::halfPi,
           MathConstants<float>::twoPi, true);
  p.lineTo(x + width - diameter, y);
  p.addArc(x + width - diameter, y, diameter, diameter, 0.f,
           MathConstants<float>::halfPi);
  p.lineTo(x + width, y + height - diameter);
  p.addArc(x + width - diameter, y + height - diameter, diameter, diameter,
           MathConstants<float>::halfPi, MathConstants<float>::pi);
  p.lineTo(x + diameter, y + height);
  p.addArc(x, y + height - diameter, diameter, diameter,
           MathConstants<float>::pi,
           MathConstants<float>::pi + MathConstants<float>::halfPi);
  p.closeSubPath();
  if (shiftHorizontally) {
    p.applyTransform(AffineTransform::translation(-width, 0));
  }
  if (shiftVertically) {
    p.applyTransform(AffineTransform::translation(0, -height));
  }
  return p;
}

inline void drawRoundedRectangle(Graphics& g, float x, float y, float width,
                                 float height, float cornerSize,
                                 float lineThickness = 1.f) {
  g.strokePath(createRoundedRectangle(x, y, width, height, cornerSize),
               PathStrokeType(lineThickness));
}

inline void fillRoundedRectangle(Graphics& g, float x, float y, float width,
                                 float height, float cornerSize) {
  g.fillPath(createRoundedRectangle(x, y, width, height, cornerSize));
}

inline void drawRoundedRectangle(Graphics& g, const juce::Rectangle<float> rect,
                                 float cornerSize, float lineThickness = 1.f) {
  g.strokePath(createRoundedRectangle(rect.getX(), rect.getY(), rect.getWidth(),
                                      rect.getHeight(), cornerSize),
               PathStrokeType(lineThickness));
}

inline void fillRoundedRectangle(Graphics& g, const juce::Rectangle<float> rect,
                                 float cornerSize) {
  g.fillPath(createRoundedRectangle(rect.getX(), rect.getY(), rect.getWidth(),
                                    rect.getHeight(), cornerSize));
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
