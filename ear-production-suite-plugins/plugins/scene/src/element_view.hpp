#pragma once

#include "JuceHeader.h"

#include "components/ear_button.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class ElementView : public Component {
 public:
  ElementView();
  void paint(Graphics& g) override;

  void mouseDown(const MouseEvent& event) override;

  virtual int getDesiredHeight() = 0;

  void mouseEnter(const MouseEvent& event) override;
  void mouseExit(const MouseEvent& event) override;
  void mouseUp(const MouseEvent& event) override;

  void resized() override;

  EarButton* getHandleButton();
  EarButton* getRemoveButton();

 protected:
  std::unique_ptr<EarButton> handleButton_;
  std::unique_ptr<EarButton> removeButton_;

  const float marginBig_ = 20;
  const float marginSmall_ = 10;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElementView)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
