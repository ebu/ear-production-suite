#pragma once

#include "elements_container.hpp"

#include "helper/move.hpp"
#include <cassert>

namespace ear {
namespace plugin {
namespace ui {

ElementsContainer::ElementsContainer()
    : viewport(std::make_unique<Viewport>()) {
  list = std::make_unique<ElementViewList>(this);
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

void ElementsContainer::removeElementUiInteraction(ElementView * element)
{
  auto it = std::find_if(
    elements.begin(), elements.end(),
    [element](auto candidate) { return candidate.get() == element; });
  auto index = std::distance(elements.begin(), it);
  removeElement(index);
  Component::BailOutChecker checker(this);
  listeners_.callChecked(checker, [this, index](Listener& l) {
    l.removeElementClicked(this->list.get(), index);
  });
  if (checker.shouldBailOut()) {
    return;
  }
}

void ElementsContainer::moveElementUiInteraction(int oldIndex, int newIndex)
{
  moveElement(oldIndex, newIndex);
  Component::BailOutChecker checker(this);
  listeners_.callChecked(checker, [this, oldIndex, newIndex](Listener& l) {
    l.elementMoved(this->list.get(), oldIndex, newIndex);
  });
  if (checker.shouldBailOut()) {
    return;
  }
}

void ElementsContainer::removeElement(int index) {
  assert(index >= 0 && index < elements.size());
  list->removeChildComponent(elements[index].get());
  elements.erase(elements.begin() + index);
  list->resized();
}

void ElementsContainer::moveElement(int oldIndex, int newIndex) {
  assert(oldIndex >= 0 && oldIndex < elements.size());
  assert(newIndex >= 0 && newIndex < elements.size());
  if(oldIndex != newIndex) {
    move(elements.begin(), oldIndex, newIndex);
  }
}

void ElementsContainer::addElement(std::shared_ptr<ElementView> element) {
  element->getRemoveButton()->onClick = [this, element]() {
    removeElementUiInteraction(element.get());
  };
  elements.push_back(element);
  list->addAndMakeVisible(element.get());
  resized();
}

void ElementsContainer::addListener(Listener* l) { listeners_.add(l); }

void ElementsContainer::removeListener(Listener* l) { listeners_.remove(l); }

}  // namespace ui
}  // namespace plugin
}  // namespace ear
