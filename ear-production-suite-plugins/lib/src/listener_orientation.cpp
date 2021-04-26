#include "listener_orientation.hpp"
#include <string>
#include <algorithm>
#include <cmath>

template <typename T>
T clamp(const T& n, const T& lower, const T& upper) {
  return std::max(lower, std::min(n, upper));
}

template <typename T>
T radiansToDegrees(T radians) {
  return radians * (T(180) / T(3.141592653589793238));
}

template <typename T>
T degreesToRadians(T degrees) {
  return degrees * (T(3.141592653589793238) / T(180));
}

namespace ear {
namespace plugin {

ListenerOrientation::ListenerOrientation() {}

ListenerOrientation::~ListenerOrientation() {}

ListenerOrientation::Euler ListenerOrientation::getEuler() {
  if (!eulerOutput.has_value()) {
    if (lastQuatInput.has_value()) {
      eulerOutput = toEuler(*lastQuatInput, YPR);
    } else {
      eulerOutput = Euler{0.0, 0.0, 0.0, YPR};
    }
  }
  return *eulerOutput;
}

void ListenerOrientation::setEuler(Euler e) {
  if (runningListeners) return;  // Prevent listeners causing recursive loop
  lastQuatInput.reset();
  if (lastEulerInput.has_value()) {
    auto& lE = *lastEulerInput;
    if (lE.y == e.y && lE.p == e.p && lE.r == e.r && lE.order == e.order)
      return;
  }
  lastEulerInput = e;
  eulerOutput = e;
  // Other output needs reconvert
  quatOutput.reset();

  callListeners();
}

ListenerOrientation::Quaternion ListenerOrientation::getQuaternion() {
  if (!quatOutput.has_value()) {
    if (lastEulerInput.has_value()) {
      quatOutput = toQuaternion(*lastEulerInput);
    } else {
      quatOutput = toQuaternion(Euler{0.0, 0.0, 0.0, YPR});
    }
  }
  return *quatOutput;
}

void ListenerOrientation::setQuaternion(Quaternion q) {
  if (runningListeners) return;  // Prevent listeners causing recursive loop
  lastEulerInput.reset();
  if (lastQuatInput.has_value()) {
    auto& lQ = *lastQuatInput;
    if (lQ.w == q.w && lQ.x == q.x && lQ.y == q.y && lQ.z == q.z) return;
  }
  lastQuatInput = q;
  quatOutput = q;
  // Other output needs reconvert
  eulerOutput.reset();

  callListeners();
}

ListenerOrientation::Euler ListenerOrientation::toEuler(Quaternion q,
                                                        EulerOrder order) {
  double eRadX, eRadY, eRadZ;

  auto x2 = q.x + q.x, y2 = q.y + q.y, z2 = q.z + q.z;
  auto xx = q.x * x2, xy = q.x * y2, xz = q.x * z2;
  auto yy = q.y * y2, yz = q.y * z2, zz = q.z * z2;
  auto wx = q.w * x2, wy = q.w * y2, wz = q.w * z2;

  double m11 = (1.0 - (yy + zz));
  double m12 = (xy - wz);
  double m13 = (xz + wy);
  double m21 = (xy + wz);
  double m22 = (1 - (xx + zz));
  double m23 = (yz - wx);
  double m31 = (xz - wy);
  double m32 = (yz + wx);
  double m33 = (1.0 - (xx + yy));

  switch (order) {
    case RPY:  // XYZ:
      eRadY = asin(clamp(m13, -1.0, 1.0));
      if (abs(m13) < 0.9999999) {
        eRadX = atan2(-m23, m33);
        eRadZ = atan2(-m12, m11);
      } else {
        eRadX = atan2(m32, m22);
        eRadZ = 0.0;
      }
      break;

    case PRY:  // YXZ:
      eRadX = asin(-clamp(m23, -1.0, 1.0));
      if (abs(m23) < 0.9999999) {
        eRadY = atan2(m13, m33);
        eRadZ = atan2(m21, m22);
      } else {
        eRadY = atan2(-m31, m11);
        eRadZ = 0.0;
      }
      break;

    case YRP:  // ZXY:
      eRadX = asin(clamp(m32, -1.0, 1.0));
      if (abs(m32) < 0.9999999) {
        eRadY = atan2(-m31, m33);
        eRadZ = atan2(-m12, m22);
      } else {
        eRadY = 0.0;
        eRadZ = atan2(m21, m11);
      }
      break;

    case YPR:  // ZYX:
      eRadY = asin(-clamp(m31, -1.0, 1.0));
      if (abs(m31) < 0.9999999) {
        eRadX = atan2(m32, m33);
        eRadZ = atan2(m21, m11);
      } else {
        eRadX = 0.0;
        eRadZ = atan2(-m12, m22);
      }
      break;

    case PYR:  // YZX:
      eRadZ = asin(clamp(m21, -1.0, 1.0));
      if (abs(m21) < 0.9999999) {
        eRadX = atan2(-m23, m22);
        eRadY = atan2(-m31, m11);
      } else {
        eRadX = 0.0;
        eRadY = atan2(m13, m33);
      }
      break;

    case RYP:  // XZY:
      eRadZ = asin(-clamp(m12, -1.0, 1.0));
      if (abs(m12) < 0.9999999) {
        eRadX = atan2(m32, m22);
        eRadY = atan2(m13, m11);
      } else {
        eRadX = atan2(-m23, m33);
        eRadY = 0.0;
      }
      break;

    default:
      throw std::runtime_error(
          "setFromRotationMatrix() encountered an unknown order");
  }

  return Euler{radiansToDegrees(eRadZ), radiansToDegrees(eRadY),
               radiansToDegrees(eRadX), order};
}

ListenerOrientation::Quaternion ListenerOrientation::toQuaternion(Euler e) {
  Quaternion q;

  double eRadX = degreesToRadians(e.r);
  double eRadY = degreesToRadians(e.p);
  double eRadZ = degreesToRadians(e.y);

  double c1 = cos(eRadX / 2.0);
  double c2 = cos(eRadY / 2.0);
  double c3 = cos(eRadZ / 2.0);

  double s1 = sin(eRadX / 2.0);
  double s2 = sin(eRadY / 2.0);
  double s3 = sin(eRadZ / 2.0);

  switch (e.order) {
    case RPY:  // XYZ:
      q.x = s1 * c2 * c3 + c1 * s2 * s3;
      q.y = c1 * s2 * c3 - s1 * c2 * s3;
      q.z = c1 * c2 * s3 + s1 * s2 * c3;
      q.w = c1 * c2 * c3 - s1 * s2 * s3;
      break;

    case PRY:  // YXZ:
      q.x = s1 * c2 * c3 + c1 * s2 * s3;
      q.y = c1 * s2 * c3 - s1 * c2 * s3;
      q.z = c1 * c2 * s3 - s1 * s2 * c3;
      q.w = c1 * c2 * c3 + s1 * s2 * s3;
      break;

    case YRP:  // ZXY:
      q.x = s1 * c2 * c3 - c1 * s2 * s3;
      q.y = c1 * s2 * c3 + s1 * c2 * s3;
      q.z = c1 * c2 * s3 + s1 * s2 * c3;
      q.w = c1 * c2 * c3 - s1 * s2 * s3;
      break;

    case YPR:  // ZYX:
      q.x = s1 * c2 * c3 - c1 * s2 * s3;
      q.y = c1 * s2 * c3 + s1 * c2 * s3;
      q.z = c1 * c2 * s3 - s1 * s2 * c3;
      q.w = c1 * c2 * c3 + s1 * s2 * s3;
      break;

    case PYR:  // YZX:
      q.x = s1 * c2 * c3 + c1 * s2 * s3;
      q.y = c1 * s2 * c3 + s1 * c2 * s3;
      q.z = c1 * c2 * s3 - s1 * s2 * c3;
      q.w = c1 * c2 * c3 - s1 * s2 * s3;
      break;

    case RYP:  // XZY:
      q.x = s1 * c2 * c3 - c1 * s2 * s3;
      q.y = c1 * s2 * c3 - s1 * c2 * s3;
      q.z = c1 * c2 * s3 + s1 * s2 * c3;
      q.w = c1 * c2 * c3 + s1 * s2 * s3;
      break;

    default:
      throw std::runtime_error("setFromEuler() encountered an unknown order");
  }

  return q;
}

void ListenerOrientation::addListener(QuaternionListener* listener) {
  quatListeners.push_back(listener);
}

void ListenerOrientation::addListener(EulerListener* listener) {
  eulerListeners.push_back(listener);
}

void ListenerOrientation::removeListener(QuaternionListener* listener) {
  quatListeners.erase(
      std::remove(quatListeners.begin(), quatListeners.end(), listener),
      quatListeners.end());
}

void ListenerOrientation::removeListener(EulerListener* listener) {
  eulerListeners.erase(
      std::remove(eulerListeners.begin(), eulerListeners.end(), listener),
      eulerListeners.end());
}

void ListenerOrientation::callListeners() {
  if (runningListeners) return;  // Prevent listeners causing recursive loop
  runningListeners = true;
  if (quatListeners.size() > 0) {
    auto quat = getQuaternion();
    for (auto listener : quatListeners) {
      listener->orientationChange(quat);
    }
  }
  if (eulerListeners.size() > 0) {
    auto euler = getEuler();
    for (auto listener : eulerListeners) {
      listener->orientationChange(euler);
    }
  }
  runningListeners = false;
}

ListenerOrientation::QuaternionListener::QuaternionListener() {}
ListenerOrientation::QuaternionListener::~QuaternionListener() {}
void ListenerOrientation::QuaternionListener::orientationChange(
    ListenerOrientation::Quaternion quat) {}
ListenerOrientation::EulerListener::EulerListener() {}
ListenerOrientation::EulerListener::~EulerListener() {}
void ListenerOrientation::EulerListener::orientationChange(
    ListenerOrientation::Euler euler) {}

}  // namespace plugin
}  // namespace ear
