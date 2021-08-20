#pragma once

#include "direct_speakers_plugin_processor.hpp"

#include "JuceHeader.h"

#include "direct_speakers_component.hpp"

class DirectSpeakersAudioProcessorEditor : public AudioProcessorEditor {
 public:
  DirectSpeakersAudioProcessorEditor(DirectSpeakersAudioProcessor*);
  ~DirectSpeakersAudioProcessorEditor();

  void paint(Graphics&) override;
  void resized() override;

 private:
  DirectSpeakersAudioProcessor* p_;

  const int desiredWidth{ 750 };
  const int desiredHeight{ 930 };

  std::unique_ptr<ear::plugin::ui::DirectSpeakersComponent> content_;
  std::unique_ptr<Viewport> viewport_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
      DirectSpeakersAudioProcessorEditor)
};
