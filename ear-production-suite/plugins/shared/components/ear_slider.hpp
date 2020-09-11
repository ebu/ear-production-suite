#pragma once

#include "JuceHeader.h"
#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"
#include "look_and_feel/slider.hpp"
#include "../helper/graphics.hpp"
#include "ear_slider_label.hpp"

namespace ear {
namespace plugin {
namespace ui {

class EarSlider : public Slider {
 public:
  EarSlider() : Slider() { setLookAndFeel(&earSliderLookAndFeel_); }

  void setTicks(std::vector<Tick> ticks) {
    earSliderLookAndFeel_.setTicks(ticks);
  }
  void setUnit(const String& unit) { earSliderLookAndFeel_.setUnit(unit); }
  void setValueOrigin(float valueOrigin) {
    earSliderLookAndFeel_.setValueOrigin(valueOrigin);
  }
  void setGrabFocusOnTextChange(bool grabFocus) {
    earSliderLookAndFeel_.setGrabFocusOnTextChange(grabFocus);
  }

  void setLabel(EarSliderLabel* label) { label_ = label; }

  void addListener(Listener* l) {
    Slider::addListener(l);
    if (label_) {
      label_->addListener(l);
    }
  }
  void removeListener(Listener* l) {
    Slider::removeListener(l);
    if (label_) {
      label_->removeListener(l);
    }
  }

 private:
  SliderLookAndFeel earSliderLookAndFeel_;
  EarSliderLabel* label_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarSlider)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
