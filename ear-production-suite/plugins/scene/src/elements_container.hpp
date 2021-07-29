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
  ElementsContainer();

  void paint(Graphics& g) override;

  void resized() override;

  std::unique_ptr<Viewport> viewport;
  std::unique_ptr<ElementViewList> list;
  std::vector<std::shared_ptr<ElementView>> elements;

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElementsContainer)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
