#pragma once

#include <memory>
#include <optional>
#include <functional>
#include "ear-plugin-base/export.h"

namespace ear {
namespace plugin {

class ListenerOrientation
{
public:
  EAR_PLUGIN_BASE_EXPORT ListenerOrientation();
  EAR_PLUGIN_BASE_EXPORT ~ListenerOrientation();

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

  EAR_PLUGIN_BASE_EXPORT Euler getEuler();
  EAR_PLUGIN_BASE_EXPORT void setEuler(Euler e);

  EAR_PLUGIN_BASE_EXPORT Quaternion getQuaternion();
  EAR_PLUGIN_BASE_EXPORT void setQuaternion(Quaternion q);

  EAR_PLUGIN_BASE_EXPORT void setCoordinateUpdateHandler(std::function<void()> callback);

private:
  std::optional<Euler> lastEulerInput;
  std::optional<Quaternion> lastQuatInput;

  std::optional<Euler> eulerOutput;
  std::optional<Quaternion> quatOutput;

  Euler toEuler(Quaternion q, EulerOrder o);
  Quaternion toQuaternion(Euler e);

  std::function<void()> coordinateUpdateCallback;
};

}
}
