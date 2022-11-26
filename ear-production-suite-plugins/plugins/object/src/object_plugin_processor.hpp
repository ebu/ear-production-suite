#pragma once

#include "JuceHeader.h"

#include <memory>
#include "components/level_meter_calculator.hpp"
#include "communication/common_types.hpp"
#include "reaper_vst3_interfaces.h"
#include "components/read_only_audio_parameter_int.hpp"

namespace ear {
namespace plugin {
namespace ui {
class ObjectsJuceFrontendConnector;
}
class ObjectBackend;
}  // namespace plugin
}  // namespace ear

class ObjectsAudioProcessor : public AudioProcessor, public VST3ClientExtensions {
 public:
  ObjectsAudioProcessor();
  ~ObjectsAudioProcessor();

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

  AudioParameterInt* getRouting() { return routing_; }
  AudioParameterFloat* getGain() { return gain_; }
  AudioParameterFloat* getAzimuth() { return azimuth_; }
  AudioParameterFloat* getElevation() { return elevation_; }
  AudioParameterFloat* getDistance() { return distance_; }
  AudioParameterBool* getLinkSize() { return linkSize_; }
  AudioParameterFloat* getSize() { return size_; }
  AudioParameterFloat* getWidth() { return width_; }
  AudioParameterFloat* getHeight() { return height_; }
  AudioParameterFloat* getDepth() { return depth_; }
  AudioParameterFloat* getDiffuse() { return diffuse_; }
  AudioParameterBool* getDivergence() { return divergence_; }
  AudioParameterFloat* getFactor() { return factor_; }
  AudioParameterFloat* getRange() { return range_; }
  AudioParameterBool* getUseTrackName() { return useTrackName_; }
  AudioParameterInt* getInputInstanceId() { return inputInstanceId_; }

  AudioProcessorParameter* getBypassParameter() {
    return bypass_;
  }

  std::weak_ptr<ear::plugin::LevelMeterCalculator> getLevelMeter() {
    return levelMeter_;
  };

  ear::plugin::ui::ObjectsJuceFrontendConnector* getFrontendConnector() {
    return connector_.get();
  }

  void setIHostApplication(Steinberg::FUnknown *unknown) override;
  void extensionSetState(std::string const& xmlState);

 private:
  IReaperHostApplication* reaperHost{ nullptr };
  ear::plugin::communication::ConnectionId connectionId_;

  AudioParameterInt* routing_;
  AudioParameterFloat* gain_;
  AudioParameterFloat* azimuth_;
  AudioParameterFloat* elevation_;
  AudioParameterFloat* distance_;
  AudioParameterBool* linkSize_;
  AudioParameterFloat* size_;
  AudioParameterFloat* width_;
  AudioParameterFloat* height_;
  AudioParameterFloat* depth_;
  AudioParameterFloat* diffuse_;
  AudioParameterBool* divergence_;
  AudioParameterFloat* factor_;
  AudioParameterFloat* range_;
  AudioParameterBool* bypass_;
  AudioParameterBool* useTrackName_;
  ReadOnlyAudioParameterInt* inputInstanceId_;

  std::unique_ptr<ear::plugin::ui::ObjectsJuceFrontendConnector> connector_;
  std::unique_ptr<ear::plugin::ObjectBackend> backend_;

  int samplerate_;
  std::shared_ptr<ear::plugin::LevelMeterCalculator> levelMeter_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ObjectsAudioProcessor)
};
