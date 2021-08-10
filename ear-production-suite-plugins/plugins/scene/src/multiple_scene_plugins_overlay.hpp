#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/look_and_feel/shadows.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class MultipleScenePluginsOverlay : public Component {
 public:
  MultipleScenePluginsOverlay() {}

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
    area.removeFromTop(getHeight() / 3);
    g.drawText(String::fromUTF8("Failed to initialize EAR Scene plugin."),
               area.removeFromTop(getHeight() / 10), Justification::centred);
    g.setFont(font::RobotoSingleton::instance().getRegular(25.f));
    g.drawText(String::fromUTF8(
                   "Please make sure this is the only instance of the EAR "
                   "Scene Plugin "),
               area.removeFromTop(getHeight() / 10), Justification::centred);
    g.drawText(String::fromUTF8("currently running on this machine."),
               area.removeFromTop(getHeight() / 20), Justification::centred);
  }

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultipleScenePluginsOverlay)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
