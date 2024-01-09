#pragma once

#include "JuceHeader.h"

#include "components/ear_combo_box.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class ValueBoxDataFile : public Component {
 public:
  ValueBoxDataFile();

  void paint(Graphics& g) override;
  void resized() override;

  std::shared_ptr<EarComboBox> getDataFileComboBox() { return comboBox_; }

 private:
  const float labelWidth_ = 80.f;
  const float rowHeight_ = 30.f;
  const float margin_ = 10.f;

  Label label_;
  std::shared_ptr<EarComboBox> comboBox_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxDataFile)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
