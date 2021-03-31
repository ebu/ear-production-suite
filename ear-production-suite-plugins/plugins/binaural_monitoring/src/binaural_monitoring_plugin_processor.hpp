#pragma once

#include "JuceHeader.h"

#include <ear/ear.hpp>
#include <memory>
#include <optional>

#include "components/level_meter_calculator.hpp"

namespace ear {
namespace plugin {
class BinauralMonitoringBackend;
class BinauralMonitoringAudioProcessor;

}  // namespace plugin
}  // namespace ear


class ListenerOrientation
{
public:
  ListenerOrientation();
  ~ListenerOrientation();

  enum EulerOrder {
    YPR, PYR, RPY, PRY, YRP, RYP
  };

  struct Euler {
    double y, p, r;
    EulerOrder order;
  };
  struct Quaternion {
    double w, x, y, z;
  };

  Euler getEuler();
  void setEuler(Euler e);

  Quaternion getQuaternion();
  void setQuaternion(Quaternion q);

  void setCoordinateUpdateHandler(std::function<void()> callback);

private:
  std::optional<Euler> lastEulerInput;
  std::optional<Quaternion> lastQuatInput;

  std::optional<Euler> eulerOutput;
  std::optional<Quaternion> quatOutput;

  Euler toEuler(Quaternion q, EulerOrder o);
  Quaternion toQuaternion(Euler e);

  std::function<void()> coordinateUpdateCallback;
};

class ListenerOrientationOscReceiver :  private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>, private MultiTimer
{
public:
  ListenerOrientationOscReceiver();
  ~ListenerOrientationOscReceiver();

  void setListenerOrientationHandler(std::shared_ptr<ListenerOrientation>);
  void setOnConnectionStatusChangeTextHandler(std::function<void(std::string newStatus)> callback);

  void listenForConnections(uint16_t port);
  void disconnect();

  void oscMessageReceived(const OSCMessage& message) override;
  void timerCallback(int timerId) override;

private:
  std::function<void(std::string newStatus)> statusTextCallback;
  std::string curStatusText{ "Disconnected" };
  void updateStatusText(std::string& newStatus);
  void updateStatusText();

  bool isListening{ false };
  uint16_t oscPort{ 8000 };

  const int timerIdStatusTextReset = 0;
  const int timerIdPersistentListen = 1;

  OSCReceiver osc;
  std::shared_ptr<ListenerOrientation> listenerOrientation;

  // Have to track this because we can receive one coord at a time, but they're only useful together
  ListenerOrientation::Euler oscEulerInput{ 0.0, 0.0, 0.0, ListenerOrientation::EulerOrder::YPR };

};



class EarBinauralMonitoringAudioProcessor : public AudioProcessor {
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

  std::shared_ptr<ListenerOrientation> listenerOrientation{};
  ListenerOrientationOscReceiver oscReceiver{};

 private:
  BusesProperties _getBusProperties();

  void updateAudioProcessorListenerPosition();

  std::unique_ptr<ear::plugin::BinauralMonitoringBackend> backend_;
  std::unique_ptr<ear::plugin::BinauralMonitoringAudioProcessor> processor_;
  std::string bearDataFilePath;

  int samplerate_;
  int blocksize_;

  std::shared_ptr<ear::plugin::LevelMeterCalculator> levelMeter_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
      EarBinauralMonitoringAudioProcessor)
};
