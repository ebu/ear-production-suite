#pragma once

#include "JuceHeader.h"

#include <memory>
#include <optional>
#include <functional>

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
