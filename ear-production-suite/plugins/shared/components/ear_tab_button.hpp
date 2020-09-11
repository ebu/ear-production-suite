#pragma once

#include "JuceHeader.h"

#include "ear_button.hpp"

namespace ear {
namespace plugin {
namespace ui {

class EarTabButton : public Component {
 public:
  EarTabButton();
  explicit EarTabButton(const String& text);

  void setText(const String& text);
  String getText() const;

  void setSelected(bool selected);
  bool isSelected() const;

  void paint(Graphics& g) override;
  void resized() override;

  enum ColourIds {
    backgroundColourId = 0xfe011b7e,
    highlightBackgroundColourId = 0xc695320b,
    highlightColourId = 0x2fe00d76,
    textColourId = 0x1d275504,
    hoverColourId = 0xb3161960,
  };

  void mouseEnter(const MouseEvent& event) override;
  void mouseExit(const MouseEvent& event) override;
  void mouseDown(const MouseEvent& event) override;
  MouseCursor getMouseCursor() override;

  std::function<void(EarTabButton*)> onClick;
  std::function<void(EarTabButton*)> onCloseClick;

 private:
  const float paddingLeft_ = 10.f;
  const float paddingRight_ = 35.f;
  const float paddingTop_ = 5.f;
  const float paddingBottom_ = 5.f;

  String text_ = "";
  bool selected_ = false;
  bool mouseOver_ = false;
  bool mouseOverClose_ = false;

  std::unique_ptr<EarButton> removeButton_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarTabButton)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
