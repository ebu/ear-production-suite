#pragma once

#include "JuceHeader.h"

#include <ear/ear.hpp>
#include <memory>
#include <optional>
#include <mutex>

#include "components/level_meter_calculator.hpp"

#include "bear_data_files.hpp"
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

class EarBinauralMonitoringAudioProcessor
    : private AudioProcessorParameter::Listener,
      Timer,
      public AudioProcessor {
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

  AudioProcessorParameter* getBypassParameter() { return bypass_; }
  AudioParameterFloat* getYaw() { return yaw_; }
  AudioParameterFloat* getPitch() { return pitch_; }
  AudioParameterFloat* getRoll() { return roll_; }
  AudioParameterBool* getOscEnable() { return oscEnable_; }
  AudioParameterInt* getOscPort() { return oscPort_; }
  AudioParameterBool* getOscInvertYaw() { return oscInvertYaw_; }
  AudioParameterBool* getOscInvertPitch() { return oscInvertPitch_; }
  AudioParameterBool* getOscInvertRoll() { return oscInvertRoll_; }
  AudioParameterBool* getOscInvertQuatW() { return oscInvertQuatW_; }
  AudioParameterBool* getOscInvertQuatX() { return oscInvertQuatX_; }
  AudioParameterBool* getOscInvertQuatY() { return oscInvertQuatY_; }
  AudioParameterBool* getOscInvertQuatZ() { return oscInvertQuatZ_; }

  ear::plugin::ui::BinauralMonitoringJuceFrontendConnector*
  getFrontendConnector() {
    return connector_.get();
  }

  bool rendererStarted();
  std::unique_ptr<ear::plugin::DataFileManager> dataFileManager;

 protected:
  void parameterValueChanged(int parameterIndex, float newValue) override;
  void parameterGestureChanged(int parameterIndex,
                               bool gestureIsStarting) override;
  void timerCallback() override;

 private:
  BusesProperties _getBusProperties();

  AudioParameterBool* bypass_;
  AudioParameterFloat* yaw_;
  AudioParameterFloat* pitch_;
  AudioParameterFloat* roll_;
  AudioParameterBool* oscEnable_;
  AudioParameterInt* oscPort_;
  AudioParameterBool* oscInvertYaw_;
  AudioParameterBool* oscInvertPitch_;
  AudioParameterBool* oscInvertRoll_;
  AudioParameterBool* oscInvertQuatW_;
  AudioParameterBool* oscInvertQuatX_;
  AudioParameterBool* oscInvertQuatY_;
  AudioParameterBool* oscInvertQuatZ_;

  std::unique_ptr<ear::plugin::ui::BinauralMonitoringJuceFrontendConnector>
      connector_;

  std::unique_ptr<ear::plugin::BinauralMonitoringBackend> backend_;
  std::mutex processorMutex_;  // used to prevent access during (re)construction
  std::unique_ptr<ear::plugin::BinauralMonitoringAudioProcessor> processor_;
  std::string bearDataFilePath;
  juce::File bearDataFileDir;

  int samplerate_;
  int blocksize_;

  std::shared_ptr<ear::plugin::LevelMeterCalculator> levelMeter_;

  bool readConfigFile();
  bool writeConfigFile();
  enum ConfigRestoreState { NOT_RESTORED, IN_PROGRESS, RESTORED };
  ConfigRestoreState configRestoreState{ NOT_RESTORED };
  PropertiesFile::Options configFileOptions;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
      EarBinauralMonitoringAudioProcessor)
};
