#include "orientation_osc.hpp"

namespace ear {
namespace plugin {

ListenerOrientationOscReceiver::ListenerOrientationOscReceiver() {
  osc.addListener(this);
}

ListenerOrientationOscReceiver::~ListenerOrientationOscReceiver() {
  osc.disconnect();
  osc.removeListener(this);
}

void ListenerOrientationOscReceiver::listenForConnections(uint16_t port) {
  disconnect();
  isListening = osc.connect(port);
  updateStatusTextForListenAttempt();
  startTimer(timerIdPersistentListen, 1000);
}

void ListenerOrientationOscReceiver::disconnect() {
  stopTimer(timerIdStatusTextReset);
  stopTimer(timerIdPersistentListen);
  osc.disconnect();
  isListening = false;
  updateStatusText(std::string("OSC Closed."));
}

void ListenerOrientationOscReceiver::oscMessageReceived(
    const OSCMessage& message) {
  stopTimer(timerIdStatusTextReset);

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
      oscEulerInput.y = vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      if (onReceiveEuler) onReceiveEuler(oscEulerInput);
    } else if (add.matches("/pitch")) {
      // Messages understood by SPARTA/COMPASS. Path also used by AmbiHead,
      //   but it expects normalised values - will not implement.
      oscEulerInput.p = vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      if (onReceiveEuler) onReceiveEuler(oscEulerInput);
    } else if (add.matches("/roll")) {
      // Messages understood by SPARTA/COMPASS. Path also used by AmbiHead,
      //   but it expects normalised values - will not implement.
      oscEulerInput.r = vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      if (onReceiveEuler) onReceiveEuler(oscEulerInput);
    } else if (add.matches("/hedrot/yaw")) {
      // Messages sent by Hedrot
      oscEulerInput.y = vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      if (onReceiveEuler) onReceiveEuler(oscEulerInput);
    } else if (add.matches("/hedrot/pitch")) {
      // Messages sent by Hedrot
      oscEulerInput.p = vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      if (onReceiveEuler) onReceiveEuler(oscEulerInput);
    } else if (add.matches("/hedrot/roll")) {
      // Messages sent by Hedrot
      oscEulerInput.r = vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      if (onReceiveEuler) onReceiveEuler(oscEulerInput);

    } else {
      return;
    }

  } else if (vals.size() == 3) {
    if (add.matches("/rotation")) {
      // Messages understood by Ambix
      oscEulerInput.p = vals[0];
      oscEulerInput.y = vals[1];
      oscEulerInput.r = vals[2];
      oscEulerInput.order = ListenerOrientation::EulerOrder::PYR;
      if (onReceiveEuler) onReceiveEuler(oscEulerInput);

    } else if (add.matches("/rendering/htrpy")) {
      // Messages understood by AudioLab SALTE
      oscEulerInput.r = vals[0];
      oscEulerInput.p = vals[1];
      oscEulerInput.y = vals[2];
      oscEulerInput.order = ListenerOrientation::EulerOrder::RPY;
      if (onReceiveEuler) onReceiveEuler(oscEulerInput);

    } else if (add.matches("/ypr")) {
      // Messages understood by SPARTA/COMPASS
      oscEulerInput.y = vals[0];
      oscEulerInput.p = vals[1];
      oscEulerInput.r = vals[2];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      if (onReceiveEuler) onReceiveEuler(oscEulerInput);

    } else {
      return;
    }

  } else if (vals.size() == 4) {
    if (add.matches("/quaternion")) {
      // Messages understood by Ambix
      if (onReceiveQuaternion) {
        ListenerOrientation::Quaternion quat;
        quat.w = vals[0];
        quat.y = vals[1];
        quat.x = -vals[2];
        quat.z = vals[3];
        onReceiveQuaternion(quat);
      }

    } else if (add.matches("/SceneRotator/quaternions")) {
      // Messages understood by IEM
      if (onReceiveQuaternion) {
        ListenerOrientation::Quaternion quat;
        quat.w = vals[0];
        quat.x = vals[1];
        quat.y = -vals[2];
        quat.z = -vals[3];
        onReceiveQuaternion(quat);
      }

    } else if (add.matches("/quaternions")) {
      // Messages understood by Unity plugin
      if (onReceiveQuaternion) {
        ListenerOrientation::Quaternion quat;
        quat.w = vals[0];
        quat.x = -vals[1];
        quat.z = -vals[2];
        quat.y = -vals[3];
        onReceiveQuaternion(quat);
      }

    } else {
      return;
    }

  } else if (vals.size() == 7) {
    if (add.matches("/head_pose")) {
      // Messages understood by Ambix
      oscEulerInput.p = vals[4];
      oscEulerInput.y = vals[5];
      oscEulerInput.r = vals[6];
      oscEulerInput.order = ListenerOrientation::EulerOrder::PYR;
      if (onReceiveEuler) onReceiveEuler(oscEulerInput);

    } else {
      return;
    }
  }

  updateStatusText(std::string("OSC Receiving..."));
  startTimer(timerIdStatusTextReset, 500);
}

void ListenerOrientationOscReceiver::timerCallback(int timerId) {
  if (timerId == timerIdStatusTextReset) {
    // "Receiving..." timer done
    stopTimer(timerIdStatusTextReset);
    updateStatusText(std::string(isListening ? "OSC Ready." : "OSC Closed."));
  } else if (timerId == timerIdPersistentListen) {
    // If this timer is running, we should be listening
    if (!isListening) {
      isListening = osc.connect(oscPort);
      updateStatusTextForListenAttempt();
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
      std::string(isListening ? "OSC Ready." : "OSC Connection Error."));
}

}  // namespace plugin
}  // namespace ear
