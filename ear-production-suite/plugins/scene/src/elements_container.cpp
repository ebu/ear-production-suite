#pragma once

#include "elements_container.hpp"

namespace ear {
namespace plugin {
namespace ui {

ElementsContainer::ElementsContainer()
    : viewport(std::make_unique<Viewport>()),
      list(std::make_unique<ElementViewList>()) {
  viewport->setViewedComponent(list.get(), false);
  viewport->setScrollBarsShown(true, false);
  viewport->getVerticalScrollBar().setColour(ScrollBar::thumbColourId,
                                              EarColours::Area04dp);
  addAndMakeVisible(viewport.get());
}

void ElementsContainer::paint(Graphics& g) {
  g.fillAll(EarColours::ComboBoxPopupBackground);
}

void ElementsContainer::resized() {
  auto area = getLocalBounds();
  viewport->setBounds(area.reduced(2, 2));
  list->setBounds(area.reduced(2, 2));
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
