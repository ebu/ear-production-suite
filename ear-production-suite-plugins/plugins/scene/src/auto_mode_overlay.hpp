#pragma once

#include "JuceHeader.h"

#include "../../shared/components/look_and_feel/colours.hpp"
#include "../../shared/components/look_and_feel/fonts.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class AutoModeOverlay : public Component {
 public:
  AutoModeOverlay() {}

  void paint(Graphics& g) override {
    auto area = getLocalBounds();
    g.setColour(Colours::black.withAlpha(Emphasis::medium));
    g.fillRect(area);
    g.setColour(EarColours::Text.withAlpha(Emphasis::high));
    g.setFont(font::RobotoSingleton::instance().getMedium(35.f));
    area.removeFromTop(getHeight() / 4);
    g.drawText(String::fromUTF8("EAR Scene ›Auto Mode‹ is on."),
               area.removeFromTop(getHeight() / 10), Justification::centred);
    g.setFont(font::RobotoSingleton::instance().getRegular(25.f));
    g.drawText(
        String::fromUTF8("In ›Auto Mode‹ all new Items will automatically be "
                         "added to a default Programme."),
        area.removeFromTop(getHeight() / 10), Justification::centred);
    g.setFont(font::RobotoSingleton::instance().getMedium(25.f));
    g.drawText(String::fromUTF8("Click anywhere to disable it."),
               area.removeFromTop(getHeight() / 10), Justification::centred);
  }

  void mouseDown(const MouseEvent& event) override {}
  void mouseUp(const MouseEvent& event) override { setVisible(false); }

  MouseCursor getMouseCursor() override {
    return MouseCursor::PointingHandCursor;
  }

  void visibilityChanged() override {
    Component::BailOutChecker checker(this);
    listeners_.callChecked(checker, [this](Listener& l) {
      l.autoModeChanged(this, this->isVisible());
    });
    if (checker.shouldBailOut()) {
      return;
    }
  }

  class Listener {
   public:
    virtual ~Listener() = default;
    virtual void autoModeChanged(AutoModeOverlay* overlay, bool state) = 0;
  };

  void addListener(Listener* l) { listeners_.add(l); }
  void removeListener(Listener* l) { listeners_.remove(l); }

 private:
  ListenerList<Listener> listeners_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutoModeOverlay)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
