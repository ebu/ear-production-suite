#pragma once

#include <map>
#include "JuceHeader.h"
#include "helper/nng_wrappers.h"
#include "programme_store.pb.h"
#include "programme_store.hpp"
#include "components/read_only_audio_parameter_int.hpp"
#include "components/level_meter_calculator.hpp"
#include "backend_setup_timer.hpp"

namespace ear {
namespace plugin {
namespace ui {
class JuceSceneFrontendConnector;
}
class SceneBackend;
}  // namespace plugin
}  // namespace ear

class SceneAudioProcessor : public AudioProcessor {
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

  ear::plugin::ui::JuceSceneFrontendConnector* getFrontendConnector();
  ear::plugin::ProgrammeStore& getProgrammeStore() {
    return programmeStore_;
  }
  std::mutex& getProgrammeStoreMutex() { return programmeStoreMutex_; }
//  ear::plugin::proto::ProgrammeStore getProgrammeStoreCopy() {
//    return programmeStore_;
//  }

  void incomingMessage(std::shared_ptr<NngMsg> msg);

  std::weak_ptr<ear::plugin::LevelMeterCalculator> getLevelMeter() {
    return levelMeter_;
  };

  std::multimap<int, ear::plugin::proto::ProgrammeElement*>&
  getPendingElements() {
    return pendingElements_;
  }

  void setStoreFromPending() { programmeStore_.set(pendingStore_); }

  void setupBackend();

 private:
  void doSampleRateChecks();
  void sendAdmMetadata();
  void recvAdmMetadata(std::string admStr, std::vector<uint32_t> mappings);
  std::mutex programmeStoreMutex_;
  std::shared_ptr<ear::plugin::ui::JuceSceneFrontendConnector> connector_;
  std::unique_ptr<ear::plugin::SceneBackend> backend_;

  ear::plugin::ProgrammeStore programmeStore_;
  ear::plugin::proto::ProgrammeStore pendingStore_;
  std::multimap<int, ear::plugin::proto::ProgrammeElement*> pendingElements_;

  ear::plugin::ui::ReadOnlyAudioParameterInt* commandPort;
  ear::plugin::ui::ReadOnlyAudioParameterInt* samplesPort;
  CommandReceiver* commandSocket;
  SamplesSender* samplesSocket;
  bool sendSamplesToExtension{false};
  uint32_t samplerate_{0};

  std::shared_ptr<ear::plugin::LevelMeterCalculator> levelMeter_;
  std::unique_ptr<ear::plugin::BackendSetupTimer> backendSetupTimer_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SceneAudioProcessor)
};
