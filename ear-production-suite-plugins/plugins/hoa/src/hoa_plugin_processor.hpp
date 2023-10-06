#pragma once

#include "JuceHeader.h"

#include "hoa_backend.hpp"
#include "helper/common_definition_helper.h"
#include "reaper_vst3_interfaces.h"
#include "components/read_only_audio_parameter_int.hpp"
#include <daw_channel_count.h>

namespace ear {
namespace plugin {
namespace ui {
class HoaJuceFrontendConnector;

}
class HoaBackend;
class LevelMeterCalculator;
}  // namespace plugin
}  // namespace ear

class HoaAudioProcessor : public AudioProcessor, public VST3ClientExtensions {
 public:
  HoaAudioProcessor();
  ~HoaAudioProcessor();

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
  void setStateInformation(XmlElement* xmlState, bool useDefaultsIfUnspecified = true);

  void updateTrackProperties(const TrackProperties& properties) override;

  AdmCommonDefinitionHelper admCommonDefinitions{};

  AudioParameterInt* getRouting() { return routing_; }
  AudioParameterInt* getPackFormatIdValue() { return packFormatIdValue_; }
  AudioParameterBool* getUseTrackName() { return useTrackName_; }
  AudioParameterInt* getInputInstanceId() { return inputInstanceId_; }

  AudioProcessorParameter* getBypassParameter() { return bypass_; }

  std::weak_ptr<ear::plugin::LevelMeterCalculator> getLevelMeter() {
    return levelMeterCalculator_;
  };

  ear::plugin::ui::HoaJuceFrontendConnector* getFrontendConnector() {
    return connector_.get();
  }

  void setIHostApplication(Steinberg::FUnknown *unknown) override;
  void extensionSetState(std::string const& xmlState);

  int getNumDawChannels() { return numDawChannels_; }

 private:
  IReaperHostApplication* reaperHost{ nullptr };
  ear::plugin::communication::ConnectionId connectionId_;

  AudioParameterInt* routing_;
  AudioParameterInt* packFormatIdValue_;
  AudioParameterBool* bypass_;
  AudioParameterBool* useTrackName_;
  ReadOnlyAudioParameterInt* inputInstanceId_;

  std::unique_ptr<ear::plugin::ui::HoaJuceFrontendConnector> connector_;
  std::unique_ptr<ear::plugin::HoaBackend> backend_;

  int samplerate_;
  int numDawChannels_{MAX_DAW_CHANNELS};
  std::shared_ptr<ear::plugin::LevelMeterCalculator> levelMeterCalculator_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HoaAudioProcessor)
};
