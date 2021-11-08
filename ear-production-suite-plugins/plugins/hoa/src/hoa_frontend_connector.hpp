#pragma once

#include "JuceHeader.h"

#include "hoa_plugin_processor.hpp"
#include "components/ear_combo_box.hpp"
#include "helper/multi_async_updater.h"

namespace ear {
namespace plugin {
namespace ui {

class LevelDisplayBox;
class HoaFrontendBackendConnector;
class EarNameTextEditor;
class ValueBoxOrderDisplay;

class HoaJuceFrontendConnector
    : public ear::plugin::ui::HoaFrontendBackendConnector,
      private AudioProcessorParameter::Listener,
      ear::plugin::ui::EarComboBox::Listener{
 public:
  /**
   * Note: Make sure that all AudioProcessorParameters are already
   * added to the HoaAudioProcessor at the time the ctor
   * is called.
   */
  HoaJuceFrontendConnector(HoaAudioProcessor* processor);
  ~HoaJuceFrontendConnector();

  void parameterValueChanged(int parameterIndex, float newValue) override;
  void parameterGestureChanged(int parameterIndex,
                               bool gestureIsStarting) override{};
  void trackPropertiesChanged(const AudioProcessor::TrackProperties& properties);

  void setStatusBarLabel(std::shared_ptr<Label> label);
  void setColourComboBox(std::shared_ptr<EarComboBox> comboBox);
  void setNameTextEditor(std::shared_ptr<EarNameTextEditor> textEditor);
  void setRoutingComboBox(std::shared_ptr<EarComboBox> comboBox);
  void setHoaTypeComboBox(std::shared_ptr<EarComboBox> comboBox);
  void setOrderDisplayValueBox(std::shared_ptr<ValueBoxOrderDisplay> gains);

  void setName(const std::string& name);
  void setColour(Colour colour);
  void setRouting(int routing);
  void setHoaType(int hoaType);

 protected:

  void doSetStatusBarText(const std::string& text) override;

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
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
