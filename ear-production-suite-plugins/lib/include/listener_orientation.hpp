#pragma once

#include <memory>
#include <optional>
#include <functional>
#include <vector>
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

  class QuaternionListener
  {
  public:
    EAR_PLUGIN_BASE_EXPORT QuaternionListener();
    EAR_PLUGIN_BASE_EXPORT ~QuaternionListener();

    EAR_PLUGIN_BASE_EXPORT virtual void orientationChange(ListenerOrientation::Quaternion quat);
  };

  class EulerListener
  {
  public:
    EAR_PLUGIN_BASE_EXPORT EulerListener();
    EAR_PLUGIN_BASE_EXPORT ~EulerListener();

    EAR_PLUGIN_BASE_EXPORT virtual void orientationChange(ListenerOrientation::Euler euler);
  };

  EAR_PLUGIN_BASE_EXPORT void addListener(QuaternionListener* listener);
  EAR_PLUGIN_BASE_EXPORT void addListener(EulerListener* listener);
  EAR_PLUGIN_BASE_EXPORT void removeListener(QuaternionListener* listener);
  EAR_PLUGIN_BASE_EXPORT void removeListener(EulerListener* listener);

private:
  std::optional<Euler> lastEulerInput;
  std::optional<Quaternion> lastQuatInput;

  std::optional<Euler> eulerOutput;
  std::optional<Quaternion> quatOutput;

  Euler toEuler(Quaternion q, EulerOrder o);
  Quaternion toQuaternion(Euler e);

  std::vector<QuaternionListener*> quatListeners;
  std::vector<EulerListener*> eulerListeners;

  void callListeners();

};

}
}
