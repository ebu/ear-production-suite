#pragma once

#include "JuceHeader.h"

#include <memory>
#include "orientation.hpp"

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
  void updateStatusTextForListenAttempt();

  bool isListening{ false };
  uint16_t oscPort{ 8000 };

  const int timerIdStatusTextReset = 0;
  const int timerIdPersistentListen = 1;

  OSCReceiver osc;
  std::shared_ptr<ListenerOrientation> listenerOrientation;

  // Have to track this because we can receive one coord at a time, but they're only useful together
  ListenerOrientation::Euler oscEulerInput{ 0.0, 0.0, 0.0, ListenerOrientation::EulerOrder::YPR };

};
