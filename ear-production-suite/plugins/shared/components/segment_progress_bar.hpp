
#pragma once

#include "JuceHeader.h"
#include "look_and_feel/colours.hpp"

namespace ear {
namespace plugin {
namespace ui {

class SegmentProgressBar : public Component {
 public:
  SegmentProgressBar(int size = 0) : size_(size) {
    setColour(backgroundColourId, EarColours::Transparent);
    setColour(trackColourId, EarColours::SliderTrack);
    setColour(highlightColorId, EarColours::Text);
  }

  ~SegmentProgressBar() {}

  void setSelected(int newValue) {
    if (value_ != newValue) {
      value_ = newValue;
      value_ = value_ >= size_ ? size_ - 1 : value_;
      repaint();
    }
  }

  void setSize(int newSize) {
    if (size_ != newSize) {
      size_ = newSize;
      repaint();
    }
  }

  void paint(Graphics& g) override {
    g.fillAll(findColour(backgroundColourId));
    for (int i = 0; i < size_; ++i) {
      if (i == value_) {
        g.setColour(findColour(highlightColorId));
      } else {
        g.setColour(findColour(trackColourId));
      }

      g.fillRect(element_.withCentre(juce::Point<float>{
          (i + 0.5f) * (static_cast<float>(getWidth()) / size_),
          getHeight() / 2.f}));
    }
  }

  enum ColourIds {
    backgroundColourId = 0x00010001,
    trackColourId = 0x00010002,
    highlightColorId = 0x00010003
  };

  void resized() override {
    element_ = juce::Rectangle<float>{
        0.f, 0.f, static_cast<float>(getWidth()) / size_ - padding_, 2.73f};
  }

 private:
  int value_;
  int size_;
  juce::Rectangle<float> element_;
  const float padding_ = 5.5f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SegmentProgressBar)
};  // namespace ui

}  // namespace ui
}  // namespace plugin
}  // namespace ear
