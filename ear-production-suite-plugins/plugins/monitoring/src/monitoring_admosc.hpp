#pragma once

#include "JuceHeader.h"

#include <memory>
#include "ear/metadata.hpp"

namespace ear {
namespace plugin {

class AdmOscReceiver
    : private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>,
      private MultiTimer {
 public:
  AdmOscReceiver();
  ~AdmOscReceiver();

  std::function<void(int objNum, ObjectsTypeMetadata earMd)> onReceive;
  std::function<void(std::string newStatus)> onStatusChange;

  void listenForConnections(uint16_t port);
  void disconnect();

  void oscMessageReceived(const OSCMessage& message) override;
  void timerCallback(int timerId) override;

  void purgeAllObjects(
      std::function<void(int objNum, ObjectsTypeMetadata earMd)>);

  std::string getStatus();

 private:
  std::string curStatusText{"OSC Closed."};
  void updateStatusText(std::string newStatus);
  void updateStatusText();
  void updateStatusTextForListenAttempt();

  bool isListening{false};
  uint16_t oscPort{8000};

  const int timerIdStatusTextReset = 0;
  const int timerIdPersistentListen = 1;

  OSCReceiver osc;

  // Have to track this because we can receive one param at a time,
  //   but we have to send a full set
  struct SimpleObj {
    float az = 0.0;
    float el = 0.0;
    float d = 0.0;
    float gain = 1.0;
  };
  std::vector<SimpleObj> objs;
};

}  // namespace plugin
}  // namespace ear
