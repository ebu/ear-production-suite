#pragma once

#include "JuceHeader.h"

#include <ear/ear.hpp>
#include <memory>

#include "components/level_meter_calculator.hpp"

namespace ear {
namespace plugin {
class BinauralMonitoringBackend;
class BinauralMonitoringAudioProcessor;

}  // namespace plugin
}  // namespace ear

class EarBinauralMonitoringAudioProcessor : public AudioProcessor, private OSCReceiver::Listener<OSCReceiver::RealtimeCallback> {
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

  void oscMessageReceived(const OSCMessage& message) override;

 private:
  BusesProperties _getBusProperties();

  std::unique_ptr<ear::plugin::BinauralMonitoringBackend> backend_;
  std::unique_ptr<ear::plugin::BinauralMonitoringAudioProcessor> processor_;
  std::string bearDataFilePath;

  int samplerate_;
  int blocksize_;

  std::shared_ptr<ear::plugin::LevelMeterCalculator> levelMeter_;

  enum EulerOrder {
    YPR, PYR, RPY, PRY, YRP, RYP
  };

  OSCReceiver osc;
  uint16_t oscPort{ 8000 };
  bool oscConnected{ false };
  struct Euler { float y, p, r; };
  struct Quaternion { float w, x, y, z; };
  Euler oscEulerInput{ 0.0, 0.0, 0.0 };
  Quaternion latestQuat{ 0.0, 0.0, 0.0, 0.0 };

  Quaternion eulerToQuaternion(Euler euler, EulerOrder order);



  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
      EarBinauralMonitoringAudioProcessor)
};
