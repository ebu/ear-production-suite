#pragma once

#include "JuceHeader.h"

class HoaAudioProcessor;

namespace ear {
namespace plugin {

class LevelMeterCalculator;

namespace ui {

class EarButton;
class OrderBox;
class OrderDisplayBox;

class ValueBoxOrderDisplay : public Component {
 public:
  ValueBoxOrderDisplay(
      HoaAudioProcessor* p,
      std::weak_ptr<ear::plugin::LevelMeterCalculator> levelMeterCalculator);

  ~ValueBoxOrderDisplay();

  void paint(Graphics& g) override;

  void resized() override;

  void clearHoaSetup();

  void setHoaType(int hoaId);

  std::shared_ptr<ear::plugin::ui::EarButton> getResetClippingButton();

 private:
  HoaAudioProcessor* p_;
  std::weak_ptr<ear::plugin::LevelMeterCalculator> levelMeterCalculator_;

  std::unique_ptr<Label> headingLabel_;

  std::vector<std::unique_ptr<ear::plugin::ui::OrderBox>> orderBoxes_;
  std::unique_ptr<ear::plugin::ui::OrderDisplayBox> orderDisplayBox_;
  std::shared_ptr<ear::plugin::ui::EarButton> resetClippingButton_;

  const float labelWidth_ = 71.f;
  const float labelPaddingBottom_ = 0.f;
  const float sliderHeight_ = 40.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxOrderDisplay)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
