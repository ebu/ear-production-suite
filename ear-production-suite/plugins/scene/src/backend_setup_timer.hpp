#pragma once

#include "JuceHeader.h"

class SceneAudioProcessor;

namespace ear {
namespace plugin {

class BackendSetupTimer : public Timer {
 public:
  BackendSetupTimer(SceneAudioProcessor* processor) : processor_(processor) {};
  ~BackendSetupTimer() { stopTimer(); };

  void timerCallback() override;

 private:
  SceneAudioProcessor* processor_;
};

}  // namespace plugin
}  // namespace ear
