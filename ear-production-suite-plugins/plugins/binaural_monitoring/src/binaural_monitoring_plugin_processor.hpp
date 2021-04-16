#pragma once

#include "JuceHeader.h"

#include <ear/ear.hpp>
#include <memory>
#include <optional>

#include "components/level_meter_calculator.hpp"

#include "orientation_osc.hpp"

namespace ear {
namespace plugin {
namespace ui {
  class BinauralMonitoringJuceFrontendConnector;
}
class BinauralMonitoringBackend;
class BinauralMonitoringAudioProcessor;

}  // namespace plugin
}  // namespace ear


class EarBinauralMonitoringAudioProcessor : public AudioProcessor{
 public:
  EarBinauralMonitoringAudioProcessor();
  ~EarBinauralMonitoringAudioProcessor();

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

  std::weak_ptr<ear::plugin::LevelMeterCalculator> getLevelMeter() {
    return levelMeter_;
  };

  ear::plugin::ListenerOrientationOscReceiver oscReceiver{};

  AudioParameterFloat* getYaw() { return yaw_; }
  AudioParameterFloat* getPitch() { return pitch_; }
  AudioParameterFloat* getRoll() { return roll_; }
  AudioParameterBool* getOscEnable() { return oscEnable_; }
  AudioParameterInt* getOscPort() { return oscPort_; }

  ear::plugin::ui::BinauralMonitoringJuceFrontendConnector* getFrontendConnector() {
    return connector_.get();
  }

 private:
  BusesProperties _getBusProperties();

  AudioParameterFloat* yaw_;
  AudioParameterFloat* pitch_;
  AudioParameterFloat* roll_;
  AudioParameterBool* oscEnable_;
  AudioParameterInt* oscPort_;

  std::unique_ptr<ear::plugin::ui::BinauralMonitoringJuceFrontendConnector> connector_;

  std::unique_ptr<ear::plugin::BinauralMonitoringBackend> backend_;
  std::unique_ptr<ear::plugin::BinauralMonitoringAudioProcessor> processor_;
  std::string bearDataFilePath;

  int samplerate_;
  int blocksize_;

  std::shared_ptr<ear::plugin::LevelMeterCalculator> levelMeter_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
      EarBinauralMonitoringAudioProcessor)
};
