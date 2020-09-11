#pragma once

#include "JuceHeader.h"

#include "ear_slider.hpp"

namespace ear {
namespace plugin {
namespace ui {

/** A normal slider, but inverted in the sense that it goes
 * from maximum to minimum value
 */
class EarInvertedSlider : public EarSlider {
 public:
  EarInvertedSlider() : EarSlider() {}
  ~EarInvertedSlider() {}

  double proportionOfLengthToValue(double proportion) {
    return juce::Slider::proportionOfLengthToValue(1.0f - proportion);
  }
  double valueToProportionOfLength(double value) {
    return 1.0f - (juce::Slider::valueToProportionOfLength(value));
  }
};
}  // namespace ui
}  // namespace plugin
}  // namespace ear
