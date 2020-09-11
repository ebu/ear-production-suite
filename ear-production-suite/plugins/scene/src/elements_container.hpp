#pragma once

#include <memory>

#include "JuceHeader.h"

#include "element_view.hpp"
#include "element_view_list.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ElementsContainer : public Component {
 public:
  ElementsContainer()
      : viewport(std::make_unique<Viewport>()),
        list(std::make_unique<ElementViewList>()) {
    viewport->setViewedComponent(list.get(), false);
    viewport->setScrollBarsShown(true, false);
    viewport->getVerticalScrollBar().setColour(ScrollBar::thumbColourId,
                                               EarColours::Area04dp);
    addAndMakeVisible(viewport.get());
  }

  void paint(Graphics& g) override {
    g.fillAll(EarColours::ComboBoxPopupBackground);
  }

  void resized() override {
    auto area = getLocalBounds();
    viewport->setBounds(area.reduced(2, 2));
    list->setBounds(area.reduced(2, 2));
  }

  std::unique_ptr<Viewport> viewport;
  std::unique_ptr<ElementViewList> list;
  std::vector<std::shared_ptr<ElementView>> elements;

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElementsContainer)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
