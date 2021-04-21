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
  if (auto oscControl = oscEnableButton_.lock()) {
    oscControl->removeListener(this);
  }
  if (auto oscControl = oscPortControl_.lock()) {
    oscControl->removeListener(this);
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

void BinauralMonitoringJuceFrontendConnector::setOscEnableButton(std::shared_ptr<EarButton> button)
{
  button->addListener(this);
  oscEnableButton_ = button;
  setOscEnable(cachedOscEnable_);
}

void BinauralMonitoringJuceFrontendConnector::setOscPortControl(std::shared_ptr<EarSlider> slider)
{
  slider->addListener(this);
  oscPortControl_ = slider;
  setOscPort(cachedOscPort_);
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
  }
}

void BinauralMonitoringJuceFrontendConnector::setPitch(float pitch)
{
  if(listenerOrientation) {
    auto euler = listenerOrientation->getEuler();
    euler.p = pitch;
    listenerOrientation->setEuler(euler);
  }
}

void BinauralMonitoringJuceFrontendConnector::setRoll(float roll)
{
  if(listenerOrientation) {
    auto euler = listenerOrientation->getEuler();
    euler.r = roll;
    listenerOrientation->setEuler(euler);
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

void BinauralMonitoringJuceFrontendConnector::setOscEnable(bool enableState)
{
  if (auto oscEnableButtonLocked = oscEnableButton_.lock()) {
    oscEnableButtonLocked->setToggleState(enableState, dontSendNotification);
  }
  cachedOscEnable_ = enableState;
}

void BinauralMonitoringJuceFrontendConnector::setOscPort(int port)
{
  if (auto oscPortLabelLocked = oscPortControl_.lock()) {
    oscPortLabelLocked->setValue(port, dontSendNotification);
  }
  cachedOscPort_ = port;
}

void BinauralMonitoringJuceFrontendConnector::orientationChange(ear::plugin::ListenerOrientation::Euler euler)
{
  // ListenerOrientation callback from backend
  // ALL other orientation I/O to be set from here

  // Parameters - Temporarily disable listeners to avoid recursive loop
  parameterListenersEnabled = false;
  *(p_->getYaw()) = euler.y;
  *(p_->getPitch()) = euler.p;
  *(p_->getRoll()) = euler.r;
  parameterListenersEnabled = true;

  // UI - dontSendNotification to avoid recursive loop
  if(auto orientationControl = yawControl_.lock()) {
    orientationControl->setValue(euler.y, dontSendNotification);
  }
  if(auto orientationControl = pitchControl_.lock()) {
    orientationControl->setValue(euler.p, dontSendNotification);
  }
  if(auto orientationControl = rollControl_.lock()) {
    orientationControl->setValue(euler.r, dontSendNotification);
  }

}

void BinauralMonitoringJuceFrontendConnector::parameterValueChanged(int parameterIndex,
                                                         float newValue) {
  // Parameter change callback from JUCE Processor
  if(!parameterListenersEnabled) return;

  if(parameterIndex == (int)ParameterId::YAW ||
     parameterIndex == (int)ParameterId::PITCH ||
     parameterIndex == (int)ParameterId::ROLL) {

    updater_.callOnMessageThread([this, parameterIndex, newValue]() {
      if(parameterIndex == (int)ParameterId::YAW) {
        notifyParameterChanged(ParameterId::YAW, p_->getYaw()->get());
      } else if(parameterIndex == (int)ParameterId::PITCH) {
        notifyParameterChanged(ParameterId::PITCH, p_->getPitch()->get());
      } else if(parameterIndex == (int)ParameterId::ROLL) {
        notifyParameterChanged(ParameterId::ROLL, p_->getRoll()->get());
      }
    });

    if(listenerOrientation) {
      listenerOrientation->setEuler(ear::plugin::ListenerOrientation::Euler{
        p_->getYaw()->get(),
        p_->getPitch()->get(),
        p_->getRoll()->get(),
        ear::plugin::ListenerOrientation::EulerOrder::YPR });
    }

  } else if(parameterIndex == (int)ParameterId::OSC_ENABLE) {
    updater_.callOnMessageThread([this, parameterIndex, newValue]() {
      notifyParameterChanged(ParameterId::OSC_ENABLE, p_->getOscEnable()->get());
      setOscEnable(p_->getOscEnable()->get());
    });

  } else if(parameterIndex == (int)ParameterId::OSC_PORT) {
    updater_.callOnMessageThread([this, parameterIndex, newValue]() {
      notifyParameterChanged(ParameterId::OSC_PORT, p_->getOscPort()->get());
      setOscPort(p_->getOscPort()->get());
    });
  }

}

void BinauralMonitoringJuceFrontendConnector::orientationValueChanged(ear::plugin::ui::OrientationView * view)
{
  // Control callback from UI
  if(listenerOrientation) {
    auto euler = listenerOrientation->getEuler();

    if(!yawControl_.expired() && view == yawControl_.lock().get()) {
      euler.y = view->getValue();

    } else if(!pitchControl_.expired() && view == pitchControl_.lock().get()) {
      euler.p = view->getValue();

    } else if(!rollControl_.expired() && view == rollControl_.lock().get()) {
      euler.r = view->getValue();
    }

    listenerOrientation->setEuler(euler);
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

void BinauralMonitoringJuceFrontendConnector::buttonClicked(Button* button)
{
  if (!oscEnableButton_.expired() &&
      button == oscEnableButton_.lock().get()) {
    // note: getToggleState still has the old value when this is called
    //       due to setClickingTogglesState on the button being false
    bool newState = !button->getToggleState();
    *(p_->getOscEnable()) = newState;
  }
}

void BinauralMonitoringJuceFrontendConnector::sliderValueChanged(Slider* slider)
{
  if (!oscPortControl_.expired() && slider == oscPortControl_.lock().get()) {
    *(p_->getOscPort()) = slider->getValue();
  }
}

void BinauralMonitoringJuceFrontendConnector::sliderDragStarted(Slider* slider)
{
  if (!oscPortControl_.expired() && slider == oscPortControl_.lock().get()) {
    p_->getOscPort()->beginChangeGesture();
  }
}

void BinauralMonitoringJuceFrontendConnector::sliderDragEnded(Slider* slider)
{
  if (!oscPortControl_.expired() && slider == oscPortControl_.lock().get()) {
    p_->getOscPort()->endChangeGesture();
  }
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
