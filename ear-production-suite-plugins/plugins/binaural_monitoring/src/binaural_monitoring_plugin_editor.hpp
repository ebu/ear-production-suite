#pragma once

#include "JuceHeader.h"

#include "components/ear_button.hpp"
#include "components/level_meter.hpp"
#include "components/onboarding.hpp"
#include "components/overlay.hpp"
#include "components/ear_header.hpp"
#include "binaural_monitoring_plugin_processor.hpp"
#include "headphone_channel_meter.hpp"
#include "headphone_channel_meter_box.hpp"
#include "value_box_orientation.hpp"
#include "value_box_osc.hpp"
#include "error_overlay.hpp"

class EarBinauralMonitoringAudioProcessorEditor
    : public AudioProcessorEditor,
      private ear::plugin::ui::Onboarding::Listener {
 public:
  EarBinauralMonitoringAudioProcessorEditor(
      EarBinauralMonitoringAudioProcessor*);
  ~EarBinauralMonitoringAudioProcessorEditor();

  void paint(Graphics&) override;
  void resized() override;

  std::unique_ptr<ear::plugin::ui::ValueBoxOrientation> orientationValueBox;
  std::unique_ptr<ear::plugin::ui::ValueBoxOsc> oscValueBox;

 private:
  EarBinauralMonitoringAudioProcessor* p_;

  std::unique_ptr<ear::plugin::ui::EarHeader> header_;
  std::unique_ptr<ear::plugin::ui::EarButton> onBoardingButton_;
  std::unique_ptr<ear::plugin::ui::Overlay> onBoardingOverlay_;
  std::unique_ptr<ear::plugin::ui::Onboarding> onBoardingContent_;
  std::shared_ptr<ear::plugin::ui::BinauralRendererErrorOverlay> errorOverlay_;

  std::unique_ptr<ear::plugin::ui::HeadphoneChannelMeterBox> headphoneMeterBox_;
  std::vector<std::unique_ptr<ear::plugin::ui::HeadphoneChannelMeter>>
      headphoneMeters_;

  std::unique_ptr<InterProcessLock> propertiesFileLock_;
  std::unique_ptr<PropertiesFile> propertiesFile_;

  Label versionLabel;
  Label statusLabel;

  // --- Onboarding::Listener
  void endButtonClicked(ear::plugin::ui::Onboarding* onboarding) override;
  void moreButtonClicked(ear::plugin::ui::Onboarding* onboarding) override;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
      EarBinauralMonitoringAudioProcessorEditor)
};
