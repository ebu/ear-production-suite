#include "binaural_monitoring_frontend_connector.hpp"

#include "components/ear_slider.hpp"
#include "components/ear_inverted_slider.hpp"

namespace ear {
namespace plugin {
namespace ui {

BinauralMonitoringJuceFrontendConnector::BinauralMonitoringJuceFrontendConnector(
  EarBinauralMonitoringAudioProcessor* processor)
    : BinauralMonitoringFrontendBackendConnector(), p_(processor) {
  if (p_) {
    auto& parameters = p_->getParameters();
    for (int i = 0; i < parameters.size(); ++i) {
      if (parameters[i]) {
        auto rangedParameter =
            dynamic_cast<RangedAudioParameter*>(parameters[i]);
        if (rangedParameter) {
          parameters_[i] = rangedParameter;
          rangedParameter->addListener(this);
        }
      }
    }
  }
}

BinauralMonitoringJuceFrontendConnector::~BinauralMonitoringJuceFrontendConnector() {
  for (auto parameter : parameters_) {
    if (parameter.second) {
      parameter.second->removeListener(this);
    }
  }

  if (auto orientationControl = yawControl_.lock()) {
    orientationControl->removeListener(this);
  }
  if (auto orientationControl = pitchControl_.lock()) {
    orientationControl->removeListener(this);
  }
  if (auto orientationControl = rollControl_.lock()) {
    orientationControl->removeListener(this);
  }

  if(listenerOrientation) {
    listenerOrientation->removeListener(this);
  }
}

void BinauralMonitoringJuceFrontendConnector::setYawView(std::shared_ptr<OrientationView> view)
{
  view->addListener(this);
  yawControl_ = view;
  if(listenerOrientation) {
    auto euler = listenerOrientation->getEuler();
    setYaw(euler.y);
  }
}

void BinauralMonitoringJuceFrontendConnector::setPitchView(std::shared_ptr<OrientationView> view)
{
  view->addListener(this);
  pitchControl_ = view;
  if(listenerOrientation) {
    auto euler = listenerOrientation->getEuler();
    setPitch(euler.p);
  }
}

void BinauralMonitoringJuceFrontendConnector::setRollView(std::shared_ptr<OrientationView> view)
{
  view->addListener(this);
  rollControl_ = view;
  if(listenerOrientation) {
    auto euler = listenerOrientation->getEuler();
    setRoll(euler.r);
  }
}

void BinauralMonitoringJuceFrontendConnector::setListenerOrientationInstance(std::shared_ptr<ListenerOrientation> lo)
{
  listenerOrientation = lo;
  listenerOrientation->addListener(this);
}

void BinauralMonitoringJuceFrontendConnector::setYaw(float yaw)
{
  if(listenerOrientation) {
    auto euler = listenerOrientation->getEuler();
    euler.y = yaw;
    listenerOrientation->setEuler(euler);
    if(auto orientationControl = yawControl_.lock()) {
      orientationControl->setValue(yaw, dontSendNotification);
    }
  }
}

void BinauralMonitoringJuceFrontendConnector::setPitch(float pitch)
{
  if(listenerOrientation) {
    auto euler = listenerOrientation->getEuler();
    euler.p = pitch;
    listenerOrientation->setEuler(euler);
    if(auto orientationControl = pitchControl_.lock()) {
      orientationControl->setValue(pitch, dontSendNotification);
    }
  }
}

void BinauralMonitoringJuceFrontendConnector::setRoll(float roll)
{
  if(listenerOrientation) {
    auto euler = listenerOrientation->getEuler();
    euler.r = roll;
    listenerOrientation->setEuler(euler);
    if(auto orientationControl = rollControl_.lock()) {
      orientationControl->setValue(roll, dontSendNotification);
    }
  }
}

void BinauralMonitoringJuceFrontendConnector::setEuler(ListenerOrientation::Euler euler)
{
  if(listenerOrientation) {
    listenerOrientation->setEuler(euler);
  }
}

void BinauralMonitoringJuceFrontendConnector::setQuaternion(ListenerOrientation::Quaternion quat)
{
  if(listenerOrientation) {
    listenerOrientation->setQuaternion(quat);
  }
}

void BinauralMonitoringJuceFrontendConnector::orientationChange(ear::plugin::ListenerOrientation::Euler euler)
{
  *(p_->getYaw()) = euler.y;
  *(p_->getPitch()) = euler.p;
  *(p_->getRoll()) = euler.r;
}

void BinauralMonitoringJuceFrontendConnector::parameterValueChanged(int parameterIndex,
                                                         float newValue) {
  updater_.callOnMessageThread([this, parameterIndex, newValue]() {
    using ParameterId = ui::BinauralMonitoringFrontendBackendConnector::ParameterId;
    switch(parameterIndex) {
      case 0:
        notifyParameterChanged(ParameterId::YAW, p_->getYaw()->get());
        setYaw(p_->getYaw()->get());
        break;
      case 1:
        notifyParameterChanged(ParameterId::PITCH, p_->getPitch()->get());
        setPitch(p_->getPitch()->get());
        break;
      case 2:
        notifyParameterChanged(ParameterId::ROLL, p_->getRoll()->get());
        setRoll(p_->getRoll()->get());
        break;
    }
  });
}

void BinauralMonitoringJuceFrontendConnector::orientationValueChanged(ear::plugin::ui::OrientationView * view)
{
  if(!yawControl_.expired() && view == yawControl_.lock().get()) {
    *(p_->getYaw()) = view->getValue();

  } else if(!pitchControl_.expired() && view == pitchControl_.lock().get()) {
    *(p_->getPitch()) = view->getValue();

  } else if(!rollControl_.expired() && view == rollControl_.lock().get()) {
    *(p_->getRoll()) = view->getValue();
  }
}

void BinauralMonitoringJuceFrontendConnector::orientationDragStarted(ear::plugin::ui::OrientationView * view)
{
  if(!yawControl_.expired() && view == yawControl_.lock().get()) {
    p_->getYaw()->beginChangeGesture();

  } else if(!pitchControl_.expired() && view == pitchControl_.lock().get()) {
    p_->getPitch()->beginChangeGesture();

  } else if(!rollControl_.expired() && view == rollControl_.lock().get()) {
    p_->getRoll()->beginChangeGesture();
  }
}

void BinauralMonitoringJuceFrontendConnector::orientationDragEnded(ear::plugin::ui::OrientationView * view)
{
  if(!yawControl_.expired() && view == yawControl_.lock().get()) {
    p_->getYaw()->endChangeGesture();

  } else if(!pitchControl_.expired() && view == pitchControl_.lock().get()) {
    p_->getPitch()->endChangeGesture();

  } else if(!rollControl_.expired() && view == rollControl_.lock().get()) {
    p_->getRoll()->endChangeGesture();
  }
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
