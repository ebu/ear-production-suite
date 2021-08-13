#pragma once

#include "JuceHeader.h"

#include <map>

namespace ear {
namespace plugin {
namespace ui {

struct Tick {
  String text;
  float value;
  Justification justification;
};

class SliderLookAndFeel : public LookAndFeel_V4 {
 public:
  SliderLookAndFeel();
  ~SliderLookAndFeel() override;

  void drawLinearSlider(Graphics&, int x, int y, int width, int height,
                        float sliderPos, float minSliderPos, float maxSliderPos,
                        const Slider::SliderStyle, Slider&) override;

  void drawLinearSliderBackground(Graphics&, int x, int y, int width,
                                  int height, float sliderPos,
                                  float minSliderPos, float maxSliderPos,
                                  const Slider::SliderStyle style,
                                  Slider&) override;

  void drawLinearSliderThumb(Graphics&, int x, int y, int width, int height,
                             float sliderPos, float minSliderPos,
                             float maxSliderPos, const Slider::SliderStyle,
                             Slider&) override;

  Slider::SliderLayout getSliderLayout(Slider&) override;

  Label* createSliderTextBox(Slider&) override;

  // void drawLabel(Graphics& g, Label& label) override;

  void drawTextEditorOutline(Graphics& g, int width, int height,
                             TextEditor& textEditor) override;
  // void drawLabel(Graphics& g, Label& label) override;
  // int getSliderThumbRadius(Slider&) override;
  //
  //
  // int Slider::getSliderThumbRadius(Slider&) override;
  //
  //
  // Button* Slider::createSliderButton(Slider&, bool isIncrement) override;
  //
  // ImageEffectFilter* Slider::getSliderEffect(Slider&) override;
  //
  // Font getSliderPopupFont(Slider&) override;
  // int getSliderPopupPlacement(Slider&) override;
  //

  void setTicks(std::vector<Tick> ticks);
  void setUnit(const String& unit);
  void setValueOrigin(float valueOrigin);
  void setGrabFocusOnTextChange(bool grabFocus);

 private:
  std::vector<Tick> ticks_;
  String unit_ = "";
  float valueOrigin_ = 0.f;
  bool grabFocus_ = true;

  float trackWidth_ = 5.f;
  float sliderRadius_ = 7.5f;
  float tickLength_ = 7.f;
  float tickMargin_ = 3.f;
  float tickWidth_ = 0.5f;
  float tickLabelWidth_ = 50.f;
  float tickLabelHeight_ = 12.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderLookAndFeel)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
