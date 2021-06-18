#pragma once

#include "hoa_plugin_processor.hpp"

#include "JuceHeader.h"

#include "hoa_component.hpp"

class HoaAudioProcessorEditor : public AudioProcessorEditor {
 public:
  HoaAudioProcessorEditor(HoaAudioProcessor*);
  ~HoaAudioProcessorEditor();

  void paint(Graphics&) override;
  void resized() override;

 private:
  HoaAudioProcessor* p_;

  std::unique_ptr<ear::plugin::ui::HoaComponent> content_;
  std::unique_ptr<Viewport> viewport_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HoaAudioProcessorEditor)
};
