#pragma once

#include "JuceHeader.h"

#include "hoa_plugin_processor.hpp"
#include "ui/hoa_frontend_backend_connector.hpp"
// TODO - remove unrequired components once UI dev complete
#include "components/ear_combo_box.hpp"
#include "components/ear_name_text_editor.hpp"
#include "helper/multi_async_updater.h"
/* Old DS Code
// These are the definitions of the UI panels
#include "value_box_channel_gain.hpp"
#include "value_box_speaker_layer.hpp"
*/

namespace ear {
namespace plugin {
namespace ui {

class HoaJuceFrontendConnector
    : public ear::plugin::ui::HoaFrontendBackendConnector,
      private AudioProcessorParameter::Listener,
      /* Old DS Code
      // Only need to inherit from this if we're using sliders for any sort of input
      Slider::Listener,
      */
      ear::plugin::ui::EarComboBox::Listener {
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

  void trackPropertiesChanged(
      const AudioProcessor::TrackProperties& properties);

  void setStatusBarLabel(std::shared_ptr<Label> label);

  void setColourComboBox(std::shared_ptr<EarComboBox> comboBox);
  void setNameTextEditor(std::shared_ptr<EarNameTextEditor> textEditor);
  void setRoutingComboBox(std::shared_ptr<EarComboBox> comboBox);
  //ME adds bit for common definition
  void setCommonDefinitionComboBox(std::shared_ptr<EarComboBox> comboBox);

  /* Old DS Code
  // These setters allow other classes (I.e, the editor) to pass pointers to current UI controls
  void setSpeakerSetupsComboBox(std::shared_ptr<EarComboBox> comboBox);
  void setUpperLayerValueBox(std::shared_ptr<ValueBoxSpeakerLayer> layer);
  void setMiddleLayerValueBox(std::shared_ptr<ValueBoxSpeakerLayer> layer);
  void setBottomLayerValueBox(std::shared_ptr<ValueBoxSpeakerLayer> layer);
  void setChannelGainsValueBox(std::shared_ptr<ValueBoxChannelGain> gains);
  */

  void setName(const std::string& name);
  void setColour(Colour colour);
  void setRouting(int routing);
  //ME add likewise for common definition
  void setCommonDefinition(int commonDefinition);

  /* Old DS Code
  // These setters are used to set the state of controls
  void setSpeakerSetup(int speakerSetupIndex);
  */

 protected:
  // ear::plugin::ui::InputFrontendBackendConnector
  void doSetStatusBarText(const std::string& text) override;

  /* Old DS Code
  // These are "listeners" - methods which fire when a particular event occurs to a control

  // Slider::Listener
  void sliderValueChanged(Slider* slider) override;
  void sliderDragStarted(Slider*) override;
  void sliderDragEnded(Slider*) override;
  */
  // EarComboBox::Listener
  void comboBoxChanged(
      ear::plugin::ui::EarComboBox* comboBoxThatHasChanged) override;

 private:

  /* Old DS Code
  // TODO - will probably need to mimic for HOA when the user changes the HOA type
  void speakerSetupChanged(int index);
  */

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
  //ME define the box for common definitions, set as int for now
  std::weak_ptr<EarComboBox> commonDefinitionComboBox_;
  int cachedCommonDefinition_;
  //ear::plugin::HoaBackend cachedCommonDefinition_;
  /* Old DS Code
  // For each parameter, we hold a pointer to the UI control which sets it, and cache the last known value
  std::weak_ptr<EarComboBox> speakerSetupsComboBox_;
  int cachedSpeakerSetupIndex_;
  ear::plugin::SpeakerSetup cachedSpeakerSetup_;
  */

  /* Old DS Code
  // Pointers to UI panels
  std::weak_ptr<ValueBoxSpeakerLayer> upperLayer_;
  std::weak_ptr<ValueBoxSpeakerLayer> middleLayer_;
  std::weak_ptr<ValueBoxSpeakerLayer> bottomLayer_;
  std::weak_ptr<ValueBoxChannelGain> channelGains_;
  */
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
