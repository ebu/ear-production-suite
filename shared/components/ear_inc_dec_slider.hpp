#pragma once

#include "JuceHeader.h"
#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"
#include "look_and_feel/slider.hpp"
#include "../helper/graphics.hpp"
#include "ear_slider.hpp"
#include "ear_button.hpp"

namespace ear {
namespace plugin {
namespace ui {

class EarIncDecSlider : public Component {
 public:
  EarIncDecSlider() {
      slider.setSliderStyle(Slider::SliderStyle::IncDecButtons);
      addAndMakeVisible(slider);

      dec_.setButtonText("-");
      dec_.onClick = [this]() {
          if(wrapAround && slider.getValue() == slider.getRange().getStart()) {
              slider.setValue(slider.getRange().getEnd());
          } else {
              slider.setValue(slider.getValue() - 1);
          }
      };
      addAndMakeVisible(dec_);

      inc_.setButtonText("+");
      inc_.onClick = [this]() {
          if(wrapAround && slider.getValue() == slider.getRange().getEnd()) {
              slider.setValue(slider.getRange().getStart());
          } else {
              slider.setValue(slider.getValue() + 1);
          }
      };
      addAndMakeVisible(inc_);

      slider.addMouseListener(this, true);
  }

  void resized() override {
      // This assumes the width is at least twice the height!
      auto area = getLocalBounds();
      auto decArea = area.removeFromLeft(area.getHeight());
      auto incArea = area.removeFromRight(area.getHeight());
      dec_.setBounds(decArea);
      slider.setBounds(area.reduced(5,0));
      inc_.setBounds(incArea);
  }

  void mouseWheelMove(const MouseEvent& event,
                      const MouseWheelDetails& wheel) override {
      slider.grabKeyboardFocus();
      int delta = 0;
      if(wheel.deltaY < 0) {
          delta = wheel.isReversed ? 1 : -1;
      } else if(wheel.deltaY > 0) {
          delta = wheel.isReversed ? -1 : 1;
      } else {
          return;
      }
      slider.setValue(slider.getValue() + delta);
  }

  EarSlider slider;
  bool wrapAround{ false };

 private:
  SliderLookAndFeel earSliderLookAndFeel_;
  EarButton inc_;
  EarButton dec_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarIncDecSlider)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
