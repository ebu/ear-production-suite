#pragma once

#include "JuceHeader.h"

#include "direct_speakers_plugin_processor.hpp"
#include "ui/direct_speakers_frontend_backend_connector.hpp"
#include "components/ear_combo_box.hpp"
#include "components/ear_name_text_editor.hpp"
#include "helper/multi_async_updater.h"
#include "channel_meter_layout.hpp"
#include "value_box_speaker_layer.hpp"

namespace ear {
namespace plugin {
namespace ui {

class DirectSpeakersJuceFrontendConnector
    : public ear::plugin::ui::DirectSpeakersFrontendBackendConnector,
      private AudioProcessorParameter::Listener,
      Slider::Listener,
      TextEditor::Listener,
      Button::Listener,
      ear::plugin::ui::EarComboBox::Listener {
 public:
  /**
   * Note: Make sure that all AudioProcessorParameters are already
   * added to the DirectSpeakersAudioProcessor at the time the ctor
   * is called.
   */
  DirectSpeakersJuceFrontendConnector(DirectSpeakersAudioProcessor* processor);
  ~DirectSpeakersJuceFrontendConnector();

  void parameterValueChanged(int parameterIndex, float newValue) override;
  void parameterGestureChanged(int parameterIndex,
                               bool gestureIsStarting) override{};

  void trackPropertiesChanged(
      const AudioProcessor::TrackProperties& properties);

  void setStatusBarLabel(std::shared_ptr<Label> label);
  void setColourComboBox(std::shared_ptr<EarComboBox> comboBox);
  void setNameTextEditor(std::shared_ptr<EarNameTextEditor> textEditor);
  void setUseTrackNameCheckbox(std::shared_ptr<ToggleButton> useTrackNameCheckbox);
  void setRoutingComboBox(std::shared_ptr<EarComboBox> comboBox);
  void setSpeakerSetupsComboBox(std::shared_ptr<EarComboBox> comboBox);
  void setUpperLayerValueBox(std::shared_ptr<ValueBoxSpeakerLayer> layer);
  void setMiddleLayerValueBox(std::shared_ptr<ValueBoxSpeakerLayer> layer);
  void setBottomLayerValueBox(std::shared_ptr<ValueBoxSpeakerLayer> layer);

  void setName(const std::string& name);
  void setColour(Colour colour);
  void setUseTrackName(bool value);
  void setRouting(int routing);
  void setSpeakerSetup(int speakerSetupIndex);
  void setChannelGainsValueBox(std::shared_ptr<ChannelMeterLayout> gains);

  std::string getActiveName();

 protected:
  // ear::plugin::ui::InputFrontendBackendConnector
  void doSetStatusBarText(const std::string& text) override;

  // Slider::Listener
  void sliderValueChanged(Slider* slider) override;
  void sliderDragStarted(Slider*) override;
  void sliderDragEnded(Slider*) override;

  // EarComboBox::Listener
  void comboBoxChanged(
      ear::plugin::ui::EarComboBox* comboBoxThatHasChanged) override;

  // Button::Listener
  void buttonClicked(Button*) override;

  //TextEditor::Listener
  void textEditorTextChanged(TextEditor&) override;

 private:
  void speakerSetupChanged(int index);

  DirectSpeakersAudioProcessor* p_;

  MultiAsyncUpdater updater_;

  std::map<int, RangedAudioParameter*> parameters_;

  std::weak_ptr<Label> statusBarLabel_;
  std::string cachedStatusBarText_;
  std::weak_ptr<EarNameTextEditor> nameTextEditor_;
  std::weak_ptr<ToggleButton> useTrackNameCheckbox_;
  std::string cachedName_{};
  std::string lastKnownTrackName_{};
  std::string lastKnownCustomName_{};
  bool cachedUseTrackName_{ true };
  std::weak_ptr<EarComboBox> colourComboBox_;
  Colour cachedColour_{ juce::Colours::transparentBlack };
  std::weak_ptr<EarComboBox> routingComboBox_;
  int cachedRouting_{ -1 };
  std::weak_ptr<EarComboBox> speakerSetupsComboBox_;
  int cachedSpeakerSetupIndex_{ -1 };
  ear::plugin::SpeakerSetup cachedSpeakerSetup_;

  std::weak_ptr<ValueBoxSpeakerLayer> upperLayer_;
  std::weak_ptr<ValueBoxSpeakerLayer> middleLayer_;
  std::weak_ptr<ValueBoxSpeakerLayer> bottomLayer_;
  std::weak_ptr<ChannelMeterLayout> channelGains_;
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
