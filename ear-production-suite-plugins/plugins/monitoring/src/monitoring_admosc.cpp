#include "monitoring_admosc.hpp"

#define OSC_READY_MSG "OSC Ready."
#define OSC_CLOSED_MSG "OSC Closed."
#define OSC_ERROR_MSG "OSC Connection Error - Ensure port is not occupied by another plug-in or application."
#define OSC_RECEIVING_MSG "OSC Receiving..."

namespace ear {
namespace plugin {

const float oneRadInDegs = 180.f / juce::float_Pi;

AdmOscReceiver::AdmOscReceiver() {
  osc.addListener(this);
  objs.resize(64);
}

AdmOscReceiver::~AdmOscReceiver() {
  disconnect();
  osc.removeListener(this);
}

void AdmOscReceiver::listenForConnections(uint16_t port) {
  disconnect();
  oscPort = port;
  isListening = osc.connect(oscPort);
  updateStatusTextForListenAttempt();
  if(!isListening) {
    // To auto-reconnect on connection errors, we constantly try rebinding.
    startTimer(timerIdPersistentListen, 1000);
  }
}

void AdmOscReceiver::disconnect() {
  stopTimer(timerIdStatusTextReset);
  stopTimer(timerIdPersistentListen);
  osc.disconnect();
  isListening = false;
  updateStatusText(std::string(OSC_CLOSED_MSG));
}

void AdmOscReceiver::oscMessageReceived(
    const OSCMessage& message) {

  // If we have a message, we must be connected - we don't need to keep trying the rebinding
  stopTimer(timerIdPersistentListen);

  stopTimer(timerIdStatusTextReset);
  updateStatusText(std::string(OSC_RECEIVING_MSG));
  startTimer(timerIdStatusTextReset, 500);

  auto add = message.getAddressPattern();

  std::vector<float> vals(message.size(), 0.0);
  for (int i = 0; i < vals.size(); i++) {
    if (message[i].isFloat32()) {
      vals[i] = message[i].getFloat32();
    } else if (message[i].isInt32()) {
      vals[i] = (float)message[i].getInt32();
    } else {
      return;
    }
  }

  auto addStr = add.toString(); 
  if(addStr.startsWith("/adm/obj/")) {
    addStr = addStr.substring(9);
    auto slashPos = addStr.indexOf("/");
    if (slashPos > 0) {
      auto objNumStr = addStr.substring(0, slashPos);
      auto objNum = objNumStr.getIntValue();
      addStr = addStr.substring(slashPos);
      if (objNum >= 0 && objNum < 63) {
        if (addStr == "/aed" && vals.size() == 3) {
          objs[objNum].az = vals[0];
          objs[objNum].el = vals[1];
          objs[objNum].d = vals[2];
        }
        if (addStr == "/gain" && vals.size() == 1) {
          objs[objNum].gain = vals[0];
        }
        if (addStr == "/azim" && vals.size() == 1) {
          objs[objNum].az = vals[0];
        }
        if (addStr == "/elev" && vals.size() == 1) {
          objs[objNum].el = vals[0];
        }
        if (addStr == "/dist" && vals.size() == 1) {
          objs[objNum].d = vals[0];
        }
        if (onReceive) {
          ear::ObjectsTypeMetadata md;
          md.gain = objs[objNum].gain;
          md.position = ear::PolarPosition(objs[objNum].az, objs[objNum].el,
                                           objs[objNum].d);
          onReceive(objNum, md);
        }
      }
    }
  }
}

void AdmOscReceiver::timerCallback(int timerId) {
  if (timerId == timerIdStatusTextReset) {
    // "Receiving..." timer done
    stopTimer(timerIdStatusTextReset);
    updateStatusText(std::string(isListening ? OSC_READY_MSG : OSC_CLOSED_MSG));
  } else if (timerId == timerIdPersistentListen) {
    // If this timer is running, we should be listening
    isListening = osc.connect(oscPort);
    updateStatusTextForListenAttempt();
    if(isListening) {
      stopTimer(timerIdPersistentListen);
    }
  }
}

std::string AdmOscReceiver::getStatus() {
  return curStatusText;
}

void AdmOscReceiver::updateStatusText(std::string newStatus) {
  curStatusText = newStatus;
  updateStatusText();
}

void AdmOscReceiver::updateStatusText() {
  if (onStatusChange) {
    onStatusChange(curStatusText);
  }
}

void AdmOscReceiver::updateStatusTextForListenAttempt() {
  updateStatusText(
      std::string(isListening ? OSC_READY_MSG : OSC_ERROR_MSG));
}

}  // namespace plugin
}  // namespace ear
