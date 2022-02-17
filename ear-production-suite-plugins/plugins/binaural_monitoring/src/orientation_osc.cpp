#include "orientation_osc.hpp"

#define OSC_READY_MSG "OSC Ready."
#define OSC_CLOSED_MSG "OSC Closed."
#define OSC_ERROR_MSG "OSC Connection Error - Ensure port is not occupied by another plug-in or application."
#define OSC_RECEIVING_MSG "OSC Receiving..."

namespace ear {
namespace plugin {

const float oneRadInDegs = 180.f / juce::float_Pi;

ListenerOrientationOscReceiver::ListenerOrientationOscReceiver() {
  osc.addListener(this);
}

ListenerOrientationOscReceiver::~ListenerOrientationOscReceiver() {
  disconnect();
  osc.removeListener(this);
}

void ListenerOrientationOscReceiver::listenForConnections(uint16_t port) {
  disconnect();
  oscPort = port;
  isListening = osc.connect(oscPort);
  updateStatusTextForListenAttempt();
  if(!isListening) {
    // To auto-reconnect on connection errors, we constantly try rebinding.
    startTimer(timerIdPersistentListen, 1000);
  }
}

void ListenerOrientationOscReceiver::disconnect() {
  stopTimer(timerIdStatusTextReset);
  stopTimer(timerIdPersistentListen);
  osc.disconnect();
  isListening = false;
  updateStatusText(std::string(OSC_CLOSED_MSG));
  if(onInputTypeChange) onInputTypeChange(InputType::None);
}

void ListenerOrientationOscReceiver::setInverts(Inversions newInverts)
{
  invert = newInverts;
  if(lastReceivedType == InputType::Euler) doEulerCallback();
  if(lastReceivedType == InputType::Quaternion) doQuaternionCallback();
}

void ListenerOrientationOscReceiver::oscMessageReceived(
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

  if (vals.size() == 1) {
    if (add.matches("/yaw")) {
      // Messages understood by SPARTA/COMPASS. Path also used by AmbiHead,
      //   but it expects normalised values - will not implement.
      oscEulerInput.y = -vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      handleReceiveEuler();
    } else if (add.matches("/pitch")) {
      // Messages understood by SPARTA/COMPASS. Path also used by AmbiHead,
      //   but it expects normalised values - will not implement.
      oscEulerInput.p = -vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      handleReceiveEuler();
    } else if (add.matches("/roll")) {
      // Messages understood by SPARTA/COMPASS. Path also used by AmbiHead,
      //   but it expects normalised values - will not implement.
      oscEulerInput.r = vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      handleReceiveEuler();
    } else if (add.matches("/hedrot/yaw")) {

      /* NOTE:
        Hedrot TOTALLY UNTESTED due to lack of working reference implementation
        (e.g, mybino has pitch and roll swapped)
      */

      // Messages sent by Hedrot
      oscEulerInput.y = vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      handleReceiveEuler();
    } else if (add.matches("/hedrot/pitch")) {
      // Messages sent by Hedrot
      oscEulerInput.p = vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      handleReceiveEuler();
    } else if (add.matches("/hedrot/roll")) {
      // Messages sent by Hedrot
      oscEulerInput.r = vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      handleReceiveEuler();

    } else {
      return;
    }

  } else if (vals.size() == 3) {
    if (add.matches("/rotation")) {
      // Messages understood by Ambix
      oscEulerInput.p = -vals[0];
      oscEulerInput.y = -vals[1];
      oscEulerInput.r = -vals[2];
      oscEulerInput.order = ListenerOrientation::EulerOrder::PYR;
      handleReceiveEuler();

    } else if (add.matches("/rendering/htrpy")) {
      // Messages understood by AudioLab SALTE
      oscEulerInput.r = vals[0];
      oscEulerInput.p = vals[1];
      oscEulerInput.y = vals[2];
      oscEulerInput.order = ListenerOrientation::EulerOrder::RPY;
      handleReceiveEuler();

    } else if (add.matches("/ypr")) {
      // Messages understood by SPARTA/COMPASS
      oscEulerInput.y = -vals[0];
      oscEulerInput.p = -vals[1];
      oscEulerInput.r = vals[2];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      handleReceiveEuler();

    } else if (add.matches("/orientation")) {
      // Messages understood by Mach1 monitor.
      // Can support AirPods with https://github.com/Mach1Studios/M1-AirPodOSC
      oscEulerInput.y = vals[0];
      oscEulerInput.p = vals[1];
      oscEulerInput.r = vals[2];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      handleReceiveEuler();

    } else if (add.matches("/3DTI-OSC/receiver/pry")) {
      // Messages understood by 3D Tune-In Toolkit
      oscEulerInput.p = vals[0] * oneRadInDegs;
      oscEulerInput.r = vals[1] * oneRadInDegs;
      oscEulerInput.y = vals[2] * oneRadInDegs;
      oscEulerInput.order = ListenerOrientation::EulerOrder::PRY;
      handleReceiveEuler();

    } else {
      return;
    }

  } else if (vals.size() == 4) {
    if (add.matches("/quaternion")) {
      // Messages understood by Ambix
      oscQuatInput.w = vals[0];
      oscQuatInput.x = -vals[1];
      oscQuatInput.y = vals[2];
      oscQuatInput.z = vals[3];
      handleReceiveQuaternion();

    } else if (add.matches("/SceneRotator/quaternions")) {
      // Messages understood by IEM
      oscQuatInput.w = vals[0];
      oscQuatInput.x = -vals[1];
      oscQuatInput.y = vals[2];
      oscQuatInput.z = vals[3];
      handleReceiveQuaternion();

    } else if (add.matches("/quaternions")) {
      // Messages understood by Unity plugin
      oscQuatInput.w = vals[0];
      oscQuatInput.y = -vals[1];
      oscQuatInput.z = vals[2];
      oscQuatInput.x = -vals[3];
      handleReceiveQuaternion();

    } else {
      return;
    }

  } else if (vals.size() == 7) {
    if (add.matches("/head_pose")) {
      // Messages understood by Ambix
      oscEulerInput.p = -vals[4];
      oscEulerInput.y = -vals[5];
      oscEulerInput.r = -vals[6];
      oscEulerInput.order = ListenerOrientation::EulerOrder::PYR;
      handleReceiveEuler();

    } else {
      return;
    }
  }

}

void ListenerOrientationOscReceiver::timerCallback(int timerId) {
  if (timerId == timerIdStatusTextReset) {
    // "Receiving..." timer done
    stopTimer(timerIdStatusTextReset);
    updateStatusText(std::string(isListening ? OSC_READY_MSG : OSC_CLOSED_MSG));
    if(onInputTypeChange) onInputTypeChange(InputType::None);
  } else if (timerId == timerIdPersistentListen) {
    // If this timer is running, we should be listening
    isListening = osc.connect(oscPort);
    updateStatusTextForListenAttempt();
    if(isListening) {
      stopTimer(timerIdPersistentListen);
    }
  }
}

std::string ListenerOrientationOscReceiver::getStatus() {
  return curStatusText;
}

void ListenerOrientationOscReceiver::updateStatusText(std::string newStatus) {
  curStatusText = newStatus;
  updateStatusText();
}

void ListenerOrientationOscReceiver::updateStatusText() {
  if (onStatusChange) {
    onStatusChange(curStatusText);
  }
}

void ListenerOrientationOscReceiver::updateStatusTextForListenAttempt() {
  updateStatusText(
      std::string(isListening ? OSC_READY_MSG : OSC_ERROR_MSG));
}

void ListenerOrientationOscReceiver::handleReceiveEuler()
{
  lastReceivedType = InputType::Euler;
  if(onInputTypeChange) onInputTypeChange(lastReceivedType);
  doEulerCallback();
}

void ListenerOrientationOscReceiver::doEulerCallback()
{
  if(onReceiveEuler) {
    auto modVals = oscEulerInput;
    if(invert.yaw) modVals.y = -modVals.y;
    if(invert.pitch) modVals.p = -modVals.p;
    if(invert.roll) modVals.r = -modVals.r;
    onReceiveEuler(modVals);
  }
}

void ListenerOrientationOscReceiver::handleReceiveQuaternion()
{
  lastReceivedType = InputType::Quaternion;
  if(onInputTypeChange) onInputTypeChange(lastReceivedType);
  doQuaternionCallback();
}

void ListenerOrientationOscReceiver::doQuaternionCallback()
{
  if(onReceiveQuaternion) {
    auto modVals = oscQuatInput;
    if(invert.quatW) modVals.w = -modVals.w;
    if(invert.quatX) modVals.x = -modVals.x;
    if(invert.quatY) modVals.y = -modVals.y;
    if(invert.quatZ) modVals.z = -modVals.z;
    onReceiveQuaternion(modVals);
  }
}

}  // namespace plugin
}  // namespace ear
