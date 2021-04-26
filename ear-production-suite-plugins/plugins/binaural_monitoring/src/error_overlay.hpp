#pragma once

#include "JuceHeader.h"

#include "../../shared/components/look_and_feel/colours.hpp"
#include "../../shared/components/look_and_feel/fonts.hpp"
#include "../../shared/components/look_and_feel/shadows.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class BinauralRendererErrorOverlay : public Component {
 public:
   BinauralRendererErrorOverlay() {}

  void paint(Graphics& g) override {
    auto area = getLocalBounds();

    g.fillAll(EarColours::Background);
    Shadows::elevation01dp.drawForRectangle(g, area.reduced(100, 100));
    g.setColour(EarColours::Background.overlaidWith(EarColours::Area01dp));
    g.fillRect(area.reduced(100, 100));
    g.setColour(EarColours::Error.withAlpha(Emphasis::high));
    g.drawRect(area.reduced(100, 100), 2.f);

    g.setColour(EarColours::Text.withAlpha(Emphasis::high));
    g.setFont(font::RobotoSingleton::instance().getMedium(35.f));
    g.drawText(String::fromUTF8("Failed to initialize Binaural Renderer."),
               area, Justification::centred);
  }

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BinauralRendererErrorOverlay)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
