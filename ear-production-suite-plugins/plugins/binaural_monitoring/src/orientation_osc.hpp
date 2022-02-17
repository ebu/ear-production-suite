#pragma once

#include "JuceHeader.h"

#include <memory>
#include "listener_orientation.hpp"

namespace ear {
namespace plugin {

class ListenerOrientationOscReceiver
    : private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>,
      private MultiTimer {
 public:
  ListenerOrientationOscReceiver();
  ~ListenerOrientationOscReceiver();

  enum InputType { None, Euler, Quaternion };

  struct Inversions {
    bool yaw = false;
    bool pitch = false;
    bool roll = false;
    bool quatW = false;
    bool quatX = false;
    bool quatY = false;
    bool quatZ = false;
  };

  std::function<void(ListenerOrientation::Euler euler)> onReceiveEuler;
  std::function<void(ListenerOrientation::Quaternion quat)> onReceiveQuaternion;
  std::function<void(InputType inputType)> onInputTypeChange;
  std::function<void(std::string newStatus)> onStatusChange;

  void listenForConnections(uint16_t port);
  void disconnect();
  void setInverts(Inversions newInverts);

  void oscMessageReceived(const OSCMessage& message) override;
  void timerCallback(int timerId) override;

  std::string getStatus();

 private:
  std::string curStatusText{"OSC Closed."};
  void updateStatusText(std::string newStatus);
  void updateStatusText();
  void updateStatusTextForListenAttempt();

  void handleReceiveEuler();
  void doEulerCallback();
  void handleReceiveQuaternion();
  void doQuaternionCallback();

  bool isListening{false};
  uint16_t oscPort{8000};

  const int timerIdStatusTextReset = 0;
  const int timerIdPersistentListen = 1;

  OSCReceiver osc;
  Inversions invert;

  // Have to track this because we can receive one coord at a time,
  //   but they're only useful together
  ListenerOrientation::Euler oscEulerInput{
      0.0, 0.0, 0.0, ListenerOrientation::EulerOrder::YPR};
  ListenerOrientation::Quaternion oscQuatInput;
  InputType lastReceivedType;
};

}  // namespace plugin
}  // namespace ear
