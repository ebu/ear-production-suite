#pragma once

#include "JuceHeader.h"

#include "components/ear_button.hpp"
#include "components/level_meter.hpp"
#include "components/onboarding.hpp"
#include "components/overlay.hpp"
#include "components/ear_header.hpp"
#include "monitoring_plugin_processor.hpp"
#include "speaker_meter.hpp"
#include "speaker_meter_box.hpp"

class EarMonitoringAudioProcessorEditor
    : public AudioProcessorEditor,
      private ear::plugin::ui::Onboarding::Listener {
 public:
  EarMonitoringAudioProcessorEditor(EarMonitoringAudioProcessor*);
  ~EarMonitoringAudioProcessorEditor();

  void paint(Graphics&) override;
  void resized() override;

 private:
  EarMonitoringAudioProcessor* p_;

  std::unique_ptr<ear::plugin::ui::EarHeader> header_;
  std::unique_ptr<ear::plugin::ui::EarButton> onBoardingButton_;
  std::unique_ptr<ear::plugin::ui::Overlay> onBoardingOverlay_;
  std::unique_ptr<ear::plugin::ui::Onboarding> onBoardingContent_;

  std::unique_ptr<ear::plugin::ui::SpeakerMeterBox> speakerMeterBoxTop_;
  std::unique_ptr<ear::plugin::ui::SpeakerMeterBox> speakerMeterBoxBottom_;

  std::vector<std::unique_ptr<ear::plugin::ui::SpeakerMeter>> speakerMeters_;

  std::unique_ptr<InterProcessLock> propertiesFileLock_;
  std::unique_ptr<PropertiesFile> propertiesFile_;

  Label versionLabel;

  // --- Onboarding::Listener
  void endButtonClicked(ear::plugin::ui::Onboarding* onboarding) override;
  void moreButtonClicked(ear::plugin::ui::Onboarding* onboarding) override;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
      EarMonitoringAudioProcessorEditor)
};
