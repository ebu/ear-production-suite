#pragma once

#include "JuceHeader.h"

#include "direct_speakers_backend.hpp"
#include "components/level_meter_calculator.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {
class DirectSpeakersJuceFrontendConnector;
}
class DirectSpeakersBackend;
}  // namespace plugin
}  // namespace ear

class DirectSpeakersAudioProcessor : public AudioProcessor {
 public:
  DirectSpeakersAudioProcessor();
  ~DirectSpeakersAudioProcessor();

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

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

  void updateTrackProperties(const TrackProperties& properties) override;

  AudioParameterInt* getRouting() { return routing_; }
  AudioParameterInt* getPackFormatIdValue() { return packFormatIdValue_; }

  AudioProcessorParameter* getBypassParameter() {
    return bypass_;
  }

  std::weak_ptr<ear::plugin::LevelMeterCalculator> getLevelMeter() {
    return levelMeter_;
  };

  ear::plugin::ui::DirectSpeakersJuceFrontendConnector* getFrontendConnector() {
    return connector_.get();
  }

 private:
  ear::plugin::communication::ConnectionId connectionId_;

  AudioParameterInt* routing_;
  AudioParameterInt* packFormatIdValue_;
  AudioParameterBool* bypass_;

  std::unique_ptr<ear::plugin::ui::DirectSpeakersJuceFrontendConnector>
      connector_;
  std::unique_ptr<ear::plugin::DirectSpeakersBackend> backend_;

  int samplerate_;
  std::shared_ptr<ear::plugin::LevelMeterCalculator> levelMeter_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectSpeakersAudioProcessor)
};
