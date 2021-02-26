#pragma once

#include "JuceHeader.h"

#include <ear/ear.hpp>
#include <memory>

#include "components/level_meter_calculator.hpp"

// TODO: Temp
#define SPEAKER_LAYOUT_NAME "Binaural"
#define SPEAKER_LAYOUT "0+2+0"

namespace ear {
namespace plugin {
class MonitoringBackend;
class MonitoringAudioProcessor;

}  // namespace plugin
}  // namespace ear

struct ProcessorConfig {
  int inputChannels;
  int outputChannels;
  int blockSize;
  ear::Layout layout;
};

inline bool operator==(ProcessorConfig const& lhs, ProcessorConfig const rhs) {
  auto eq = lhs.inputChannels == rhs.inputChannels &&
            lhs.outputChannels == rhs.outputChannels &&
            lhs.blockSize == rhs.blockSize &&
            lhs.layout.name() == rhs.layout.name();
  return eq;
}
inline bool operator!=(ProcessorConfig const& lhs, ProcessorConfig const rhs) {
  return !(lhs == rhs);
}

class EarMonitoringAudioProcessor : public AudioProcessor {
 public:
  EarMonitoringAudioProcessor();
  ~EarMonitoringAudioProcessor();

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

  void speakerSetupChanged(std::string layout);

  std::weak_ptr<ear::plugin::LevelMeterCalculator> getLevelMeter() {
    return levelMeter_;
  };

 private:
  BusesProperties _getBusProperties();
  void configureProcessor(const ProcessorConfig& config);
  ProcessorConfig processorConfig_{};
  std::unique_ptr<ear::plugin::MonitoringBackend> backend_;
  std::unique_ptr<ear::plugin::MonitoringAudioProcessor> processor_;

  int samplerate_;
  int channels_;
  std::shared_ptr<ear::plugin::LevelMeterCalculator> levelMeter_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarMonitoringAudioProcessor)
};
