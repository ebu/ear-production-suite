#pragma once

#include "JuceHeader.h"

#include "../../shared/components/ear_colour_indicator.hpp"
#include "../../shared/components/level_meter.hpp"
#include "../../shared/components/look_and_feel/colours.hpp"
#include "../../shared/components/look_and_feel/fonts.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class EarDropIndicator : public Component {
 public:
  EarDropIndicator() {
    setColour(backgroundColourId, EarColours::Transparent);
    setColour(indicatorColourId, EarColours::Primary);
  }

  enum ColourIds {
    backgroundColourId = 0xdf0d3efd,
    indicatorColourId = 0x4f12ca61,
  };

  void paint(Graphics& g) override {
    g.fillAll(findColour(backgroundColourId));
    g.setColour(findColour(indicatorColourId));
    g.drawLine(0, getHeight() / 2.f, getWidth(), getHeight() / 2.f, 2.f);
  }

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarDropIndicator)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
