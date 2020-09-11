#pragma once

#include "object_plugin_processor.hpp"

#include "JuceHeader.h"
#include "object_component.hpp"

#include <memory>

class ObjectAudioProcessorEditor : public AudioProcessorEditor {
 public:
  ObjectAudioProcessorEditor(ObjectsAudioProcessor*);
  ~ObjectAudioProcessorEditor();

  void paint(Graphics&) override;
  void resized() override;

 private:
  ObjectsAudioProcessor* p_;

  std::unique_ptr<ear::plugin::ui::ObjectsComponent> content_;
  std::unique_ptr<Viewport> viewport_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ObjectAudioProcessorEditor)
};
