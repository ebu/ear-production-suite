#pragma once

#include "JuceHeader.h"

namespace ear {
namespace plugin {
namespace ui {

class EarBoundsConstrainer : public ComponentBoundsConstrainer {
 public:
  void checkBounds(juce::Rectangle<int>& bounds,
                   const juce::Rectangle<int>& previousBounds,
                   const juce::Rectangle<int>& limits, bool isStretchingTop,
                   bool isStretchingLeft, bool isStretchingBottom,
                   bool isStretchingRight) override {
    bounds = bounds.constrainedWithin(limits);
  }
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
