#pragma once
#include "ui/binaural_monitoring_frontend_backend_connector.hpp"

#include "JuceHeader.h"

#include "components/orientation.hpp"
#include "helper/multi_async_updater.h"
#include "binaural_monitoring_plugin_processor.hpp"
#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class BinauralMonitoringJuceFrontendConnector
    : public ear::plugin::ui::BinauralMonitoringFrontendBackendConnector,
      private AudioProcessorParameter::Listener,
      ear::plugin::ui::OrientationView::Listener,
      ear::plugin::ListenerOrientation::EulerListener {

public:
  /**
   * Note: Make sure that all AudioProcessorParameters are already
   * added to the ObjectsAudioProcessor at the time the ctor
   * is called.
   */
   BinauralMonitoringJuceFrontendConnector(EarBinauralMonitoringAudioProcessor* processor);
  ~BinauralMonitoringJuceFrontendConnector();

  void parameterValueChanged(int parameterIndex, float newValue) override;
  void parameterGestureChanged(int parameterIndex,
                               bool gestureIsStarting) override{};

  // Orientation Controls
  void setYawView(std::shared_ptr<OrientationView> view);
  void setPitchView(std::shared_ptr<OrientationView> view);
  void setRollView(std::shared_ptr<OrientationView> view);

  // Listener Orientation Object
  void setListenerOrientationInstance(std::shared_ptr<ListenerOrientation> lo);

  // Value Setters
  void setYaw(float yaw);
  void setPitch(float pitch);
  void setRoll(float roll);
  void setEuler(ListenerOrientation::Euler euler);
  void setQuaternion(ListenerOrientation::Quaternion quat);

protected:
  // Orientation::Listener
  void orientationValueChanged(ear::plugin::ui::OrientationView* view) override;
  void orientationDragStarted(ear::plugin::ui::OrientationView* view) override;
  void orientationDragEnded(ear::plugin::ui::OrientationView* view) override;

  // ListenerOrientation::EulerListener
  void orientationChange(ear::plugin::ListenerOrientation::Euler euler) override;

private:
  EarBinauralMonitoringAudioProcessor* p_;
  std::map<int, RangedAudioParameter*> parameters_;

  bool parameterListenersEnabled{ true };

  // Orientation Controls
  std::weak_ptr<OrientationView> yawControl_;
  std::weak_ptr<OrientationView> pitchControl_;
  std::weak_ptr<OrientationView> rollControl_;

  // Listener Orientation Object
  std::shared_ptr<ListenerOrientation> listenerOrientation;

   MultiAsyncUpdater updater_;

};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
