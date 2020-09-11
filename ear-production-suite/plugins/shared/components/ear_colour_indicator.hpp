#pragma once

#include "JuceHeader.h"

#include "look_and_feel/colours.hpp"

namespace ear {
namespace plugin {
namespace ui {

class EarColourIndicator : public Component {
 public:
  EarColourIndicator(Colour colour = EarColours::Background)
      : colour_(colour) {}

  void setColour(Colour newColour) { colour_ = newColour; }
  Colour getColour() const { return colour_; }

  void paint(Graphics& g) override { g.fillAll(colour_); }

 private:
  Colour colour_;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarColourIndicator)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
