#pragma once

#include <memory>

#include "JuceHeader.h"

#include "../../shared/components/ear_combo_box.hpp"
#include "../../shared/components/ear_tabbed_component.hpp"
#include "../../shared/components/onboarding.hpp"
#include "../../shared/components/overlay.hpp"
#include "../../shared/components/ear_header.hpp"
#include "items_container.hpp"
#include "auto_mode_overlay.hpp"
#include "multiple_scene_plugins_overlay.hpp"
#include "programmes_container.hpp"
#include "scene_plugin_processor.hpp"

class SceneAudioProcessorEditor
    : public AudioProcessorEditor,
      private ear::plugin::ui::Onboarding::Listener {
 public:
  SceneAudioProcessorEditor(SceneAudioProcessor*);
  ~SceneAudioProcessorEditor();

  void paint(Graphics&) override;
  void resized() override;

 private:
  SceneAudioProcessor* p_;
  std::unique_ptr<ear::plugin::ui::EarHeader> header_;
  std::unique_ptr<ear::plugin::ui::EarButton> onBoardingButton_;
  std::unique_ptr<ear::plugin::ui::Overlay> onBoardingOverlay_;
  std::unique_ptr<ear::plugin::ui::Onboarding> onBoardingContent_;

  std::shared_ptr<ear::plugin::ui::Overlay> itemsOverlay_;
  std::shared_ptr<ear::plugin::ui::ItemsContainer> itemsContainer_;
  std::shared_ptr<ear::plugin::ui::ProgrammesContainer> programmesContainer_;

  std::shared_ptr<ear::plugin::ui::AutoModeOverlay> autoModeOverlay_;
  std::shared_ptr<ear::plugin::ui::MultipleScenePluginsOverlay>
      multipleScenePluginsOverlay_;

  std::unique_ptr<InterProcessLock> propertiesFileLock_;
  std::unique_ptr<PropertiesFile> propertiesFile_;

  // --- Onboarding::Listener
  void endButtonClicked(ear::plugin::ui::Onboarding* onboarding) override;
  void moreButtonClicked(ear::plugin::ui::Onboarding* onboarding) override;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SceneAudioProcessorEditor)
};
