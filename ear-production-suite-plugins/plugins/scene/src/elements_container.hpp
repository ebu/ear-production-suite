#pragma once

#include <memory>

#include "JuceHeader.h"

#include "element_view.hpp"
#include "element_view_list.hpp"
#include "communication/common_types.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ElementsContainer : public Component {
 public:
  ElementsContainer();

  void paint(Graphics& g) override;

  void resized() override;

  void removeElementUiInteraction(ElementView* element);
  void moveElementUiInteraction(int oldIndex, int newIndex);

  void addElement(std::shared_ptr<ElementView> element);
  void removeElement(int index);
  void moveElement(int oldIndex, int newIndex);

  class Listener {
   public:
    virtual ~Listener() = default;

    virtual void elementMoved(ElementViewList* list, int oldIndex,
                              int newIndex) = 0;
    virtual void removeElementClicked(ElementViewList* list, ElementView* view) = 0;
  };

  void addListener(Listener* l);
  void removeListener(Listener* l);

  std::unique_ptr<ElementViewList> list;
  std::unique_ptr<Viewport> viewport;
  std::vector<std::shared_ptr<ElementView>> elements;

 private:
  ListenerList<Listener> listeners_;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElementsContainer)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
