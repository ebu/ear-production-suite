#pragma once

#include <string>
#include "JuceHeader.h"
#include "helper/nng_wrappers.h"
#include "store_metadata.hpp"
#include "components/read_only_audio_parameter_int.hpp"
#include "components/level_meter_calculator.hpp"
#include "backend_setup_timer.hpp"
#include "ui_event_dispatcher.hpp"
#include "communication/metadata_thread.hpp"
#include "auto_mode_controller.hpp"
#include <daw_channel_count.h>

namespace ear {
namespace plugin {
namespace ui {
class JuceSceneFrontendConnector;
}
class SceneBackend;
class PendingStore;
class RestoredPendingStore;
}  // namespace plugin
}  // namespace ear

class SceneAudioProcessor : public AudioProcessor, public VST3ClientExtensions {
 public:
  SceneAudioProcessor();
  ~SceneAudioProcessor();

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

  void processBlock(AudioBuffer<float>&, MidiBuffer&) override;

  AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  const String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const String getProgramName(int index) override;
  void changeProgramName(int index, const String& newName) override;

  void getStateInformation(MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;
  void incomingMessage(std::shared_ptr<NngMsg> msg);

  std::weak_ptr<ear::plugin::LevelMeterCalculator> getLevelMeter() {
    return levelMeter_;
  };

  void setupBackend();

  void setIHostApplication(Steinberg::FUnknown* unknown) override;

  ear::plugin::Metadata& metadata();

 private:
  void doSampleRateChecks();
  void sendAdmMetadata();
  void recvAdmMetadata(std::string admStr, std::vector<PluginToAdmMap> pluginToAdmMaps);
  void startExport();
  void stopExport();
  ear::plugin::MetadataThread metadataThread_;
  std::unique_ptr<ear::plugin::SceneBackend> backend_;
  ear::plugin::Metadata metadata_;
  std::shared_ptr<ear::plugin::PendingStore> pendingStore_;
  std::shared_ptr<ear::plugin::RestoredPendingStore> restoredStore_;
  std::shared_ptr<ear::plugin::AutoModeController> autoModeController_;

  ReadOnlyAudioParameterInt* commandPort;
  ReadOnlyAudioParameterInt* samplesPort;
  CommandReceiver* commandSocket;
  SamplesSender* samplesSocket;
  bool sendSamplesToExtension{false};
  uint32_t samplerate_{0};
  int numDawChannels_{MAX_DAW_CHANNELS};

  std::shared_ptr<ear::plugin::LevelMeterCalculator> levelMeter_;
  std::unique_ptr<ear::plugin::BackendSetupTimer> backendSetupTimer_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SceneAudioProcessor)
};
