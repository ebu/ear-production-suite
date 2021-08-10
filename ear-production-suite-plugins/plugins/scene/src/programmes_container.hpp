#pragma once

#include <memory>

#include "JuceHeader.h"

#include "components/ear_tabbed_component.hpp"
#include "programme_view.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ProgrammesContainer : public Component {
 public:
  ProgrammesContainer() : tabs(std::make_shared<EarTabbedComponent>()) {
    addAndMakeVisible(tabs.get());
  }

  void resized() override { tabs->setBounds(getLocalBounds()); }

  std::shared_ptr<EarTabbedComponent> tabs;
  std::vector<std::shared_ptr<ProgrammeView>> programmes;

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgrammesContainer)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
