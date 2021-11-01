#pragma once

#include "JuceHeader.h"

#include "hoa_plugin_processor.hpp"
#include "ui/hoa_frontend_backend_connector.hpp"
// TODO - remove unrequired components once UI dev complete
#include "components/ear_combo_box.hpp"
#include "components/ear_name_text_editor.hpp"
#include "helper/multi_async_updater.h"
#include "value_box_order_display.hpp"//probs needs adding to this target file path
#include "level_display_box.hpp"

namespace ear {
namespace plugin {
namespace ui {
class LevelDisplayBox;

class HoaJuceFrontendConnector
    : public ear::plugin::ui::HoaFrontendBackendConnector,
      private AudioProcessorParameter::Listener,
      ear::plugin::ui::EarComboBox::Listener/*,
          ear::plugin::ui::LevelDisplayBox::Listener*/ {
 public:
  /**
   * Note: Make sure that all AudioProcessorParameters are already
   * added to the HoaAudioProcessor at the time the ctor
   * is called.
   */
  HoaJuceFrontendConnector(HoaAudioProcessor* processor);
  ~HoaJuceFrontendConnector();

  void setOrderDisplayValueBox(std::shared_ptr<ValueBoxOrderDisplay> gains);

  void parameterValueChanged(int parameterIndex, float newValue) override;
  void parameterGestureChanged(int parameterIndex,
                               bool gestureIsStarting) override{};

  void trackPropertiesChanged(const AudioProcessor::TrackProperties& properties);

  void setStatusBarLabel(std::shared_ptr<Label> label);

  void setColourComboBox(std::shared_ptr<EarComboBox> comboBox);
  void setNameTextEditor(std::shared_ptr<EarNameTextEditor> textEditor);
  void setRoutingComboBox(std::shared_ptr<EarComboBox> comboBox);
  void setHoaTypeComboBox(std::shared_ptr<EarComboBox> comboBox);


  void setName(const std::string& name);
  void setColour(Colour colour);
  void setRouting(int routing);
  void setHoaType(int hoaType);


 protected:
  // ear::plugin::ui::InputFrontendBackendConnector
  void doSetStatusBarText(const std::string& text) override;

  // EarComboBox::Listener
  void comboBoxChanged(
      ear::plugin::ui::EarComboBox* comboBoxThatHasChanged) override;

 private:

  HoaAudioProcessor* p_;

  MultiAsyncUpdater updater_;

  std::map<int, RangedAudioParameter*> parameters_;

  std::weak_ptr<Label> statusBarLabel_;
  std::string cachedStatusBarText_;

  std::weak_ptr<EarNameTextEditor> nameTextEditor_;
  std::string cachedName_;
  std::weak_ptr<EarComboBox> colourComboBox_;
  Colour cachedColour_;
  std::weak_ptr<EarComboBox> routingComboBox_;
  int cachedRouting_;
  std::weak_ptr<EarComboBox> hoaTypeComboBox_;
  int cachedHoaType_;

  std::weak_ptr<ValueBoxOrderDisplay> orderDisplay_;
  //std::shared_ptr<LevelDisplayBox> displayOrderBox_;
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
