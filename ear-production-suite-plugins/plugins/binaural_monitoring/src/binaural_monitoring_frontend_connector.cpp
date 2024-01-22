#include "binaural_monitoring_frontend_connector.hpp"

#include "components/ear_slider.hpp"
#include "components/ear_inverted_slider.hpp"
#include "detail/weak_ptr_helpers.hpp"

#include <algorithm>
#include <cassert>

namespace {
void removePath(std::string& filePath) {
  // Find the last occurrence of forward slash or backslash
  size_t lastSlash = filePath.find_last_of("/\\");
  if (lastSlash != std::string::npos) {
    filePath = filePath.substr(lastSlash + 1);
  }
}
}

namespace ear {
namespace plugin {
namespace ui {
using detail::lockIfSame;

BinauralMonitoringJuceFrontendConnector::
    BinauralMonitoringJuceFrontendConnector(
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

BinauralMonitoringJuceFrontendConnector::
    ~BinauralMonitoringJuceFrontendConnector() {
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
  if (auto oscControl = oscInvertYawButton_.lock()) {
    oscControl->removeListener(this);
  }
  if (auto oscControl = oscInvertPitchButton_.lock()) {
    oscControl->removeListener(this);
  }
  if (auto oscControl = oscInvertRollButton_.lock()) {
    oscControl->removeListener(this);
  }
  if (auto oscControl = oscInvertQuatWButton_.lock()) {
    oscControl->removeListener(this);
  }
  if (auto oscControl = oscInvertQuatXButton_.lock()) {
    oscControl->removeListener(this);
  }
  if (auto oscControl = oscInvertQuatYButton_.lock()) {
    oscControl->removeListener(this);
  }
  if (auto oscControl = oscInvertQuatZButton_.lock()) {
    oscControl->removeListener(this);
  }

  if (listenerOrientation) {
    listenerOrientation->removeListener(this);
  }
}

void BinauralMonitoringJuceFrontendConnector::setYawView(
    std::shared_ptr<OrientationView> view) {
  view->addListener(this);
  yawControl_ = view;
  if (listenerOrientation) {
    auto euler = listenerOrientation->getEuler();
    setYaw(euler.y);
  }
}

void BinauralMonitoringJuceFrontendConnector::setPitchView(
    std::shared_ptr<OrientationView> view) {
  view->addListener(this);
  pitchControl_ = view;
  if (listenerOrientation) {
    auto euler = listenerOrientation->getEuler();
    setPitch(euler.p);
  }
}

void BinauralMonitoringJuceFrontendConnector::setRollView(
    std::shared_ptr<OrientationView> view) {
  view->addListener(this);
  rollControl_ = view;
  if (listenerOrientation) {
    auto euler = listenerOrientation->getEuler();
    setRoll(euler.r);
  }
}

void BinauralMonitoringJuceFrontendConnector::setOscEnableButton(
    std::shared_ptr<EarButton> button) {
  button->addListener(this);
  oscEnableButton_ = button;
  setOscEnable(cachedOscEnable_);
}

void BinauralMonitoringJuceFrontendConnector::setOscPortControl(
    std::shared_ptr<EarSlider> slider) {
  slider->addListener(this);
  oscPortControl_ = slider;
  setOscPort(cachedOscPort_);
}

void BinauralMonitoringJuceFrontendConnector::setOscInvertYawButton(
  std::shared_ptr<ToggleButton> button) {
  button->addListener(this);
  oscInvertYawButton_ = button;
  setOscInvertYaw(cachedOscInvertYaw_);
}

void BinauralMonitoringJuceFrontendConnector::setOscInvertPitchButton(
  std::shared_ptr<ToggleButton> button) {
  button->addListener(this);
  oscInvertPitchButton_ = button;
  setOscInvertPitch(cachedOscInvertPitch_);
}

void BinauralMonitoringJuceFrontendConnector::setOscInvertRollButton(
  std::shared_ptr<ToggleButton> button) {
  button->addListener(this);
  oscInvertRollButton_ = button;
  setOscInvertRoll(cachedOscInvertRoll_);
}

void BinauralMonitoringJuceFrontendConnector::setOscInvertQuatWButton(
  std::shared_ptr<ToggleButton> button) {
  button->addListener(this);
  oscInvertQuatWButton_ = button;
  setOscInvertQuatW(cachedOscInvertQuatW_);
}

void BinauralMonitoringJuceFrontendConnector::setOscInvertQuatXButton(
  std::shared_ptr<ToggleButton> button) {
  button->addListener(this);
  oscInvertQuatXButton_ = button;
  setOscInvertQuatX(cachedOscInvertQuatX_);
}

void BinauralMonitoringJuceFrontendConnector::setOscInvertQuatYButton(
  std::shared_ptr<ToggleButton> button) {
  button->addListener(this);
  oscInvertQuatYButton_ = button;
  setOscInvertQuatY(cachedOscInvertQuatY_);
}

void BinauralMonitoringJuceFrontendConnector::setOscInvertQuatZButton(
  std::shared_ptr<ToggleButton> button) {
  button->addListener(this);
  oscInvertQuatZButton_ = button;
  setOscInvertQuatZ(cachedOscInvertQuatZ_);
}

void BinauralMonitoringJuceFrontendConnector::setDataFileComponent(
    std::shared_ptr<Component> comp) {
  dataFileComponent_ = comp;
  p_->dataFileManager.updateAvailableFiles();
  auto dfs = p_->dataFileManager.getAvailableDataFiles();
  auto df = p_->dataFileManager.getSelectedDataFileInfo();
  if (dfs.size() == 1 && df && dfs[0] == df && df->isBearRelease) {
    // Only 1 data file and it is a bear release. Don't show options.
    comp->setVisible(false);
  } else {
    comp->setVisible(true);
  }
}

void BinauralMonitoringJuceFrontendConnector::setDataFileComboBox(
    std::shared_ptr<EarComboBox> comboBox) {
  comboBox->addListener(this);
  dataFileComboBox_ = comboBox;
  p_->dataFileManager.updateAvailableFiles();
  comboBox->clearEntries();
  auto dfs = p_->dataFileManager.getAvailableDataFiles();
  std::sort(
      dfs.begin(), dfs.end(),
      [](const std::shared_ptr<ear::plugin::DataFileManager::DataFile>& a,
         const std::shared_ptr<ear::plugin::DataFileManager::DataFile>& b) {
        if (a->isBearRelease != b->isBearRelease) {
          return a->isBearRelease;
        }
        return a->filename.compareIgnoreCase(b->filename) < 0;
      });
  for (const auto& df : dfs) {
    juce::String txt;
    if (!df->isBearRelease) {
      txt = "[CUSTOM]   ";
    }
    if (df->label.isEmpty()) {
      // use filename - will be an unusual case in future
      txt += df->filename;
    } else {
      txt += df->label;
      txt += "   (" + df->filename + ")";
    }
    juce::StringArray subtexts;
    if (!df->description.isEmpty()) {
      subtexts.add(df->description);
    }
    subtexts.add(juce::String(df->fullPath.getFullPathName()));
    auto entry = comboBox->addTextWithSubtextEntry(txt, subtexts, df->fullPath.getFullPathName());
    entry->setLightFont(!df->isBearRelease);
  }
  if (auto df = p_->dataFileManager.getSelectedDataFileInfo()) {
    comboBox->setSelectedId(df->fullPath.getFullPathName(),
                            juce::NotificationType::dontSendNotification);
  }
}

void BinauralMonitoringJuceFrontendConnector::setRendererStatusLabel(
    std::shared_ptr<Label> label) {
  rendererStatusLabel_ = label;
  setRendererStatus(cachedBearStatusText_, cachedBearStatusColour_);
}

void BinauralMonitoringJuceFrontendConnector::setListenerOrientationInstance(
    std::shared_ptr<ListenerOrientation> lo) {
  listenerOrientation = lo;
  listenerOrientation->addListener(this);
}

void BinauralMonitoringJuceFrontendConnector::setYaw(float yaw) {
  if (listenerOrientation) {
    auto euler = listenerOrientation->getEuler();
    euler.y = yaw;
    listenerOrientation->setEuler(euler);
  }
}

void BinauralMonitoringJuceFrontendConnector::setPitch(float pitch) {
  if (listenerOrientation) {
    auto euler = listenerOrientation->getEuler();
    euler.p = pitch;
    listenerOrientation->setEuler(euler);
  }
}

void BinauralMonitoringJuceFrontendConnector::setRoll(float roll) {
  if (listenerOrientation) {
    auto euler = listenerOrientation->getEuler();
    euler.r = roll;
    listenerOrientation->setEuler(euler);
  }
}

void BinauralMonitoringJuceFrontendConnector::setEuler(
    ListenerOrientation::Euler euler) {
  if (listenerOrientation) {
    listenerOrientation->setEuler(euler);
  }

}

void BinauralMonitoringJuceFrontendConnector::setQuaternion(
    ListenerOrientation::Quaternion quat) {
  if (listenerOrientation) {
    listenerOrientation->setQuaternion(quat);
  }
}

void BinauralMonitoringJuceFrontendConnector::setOscEnable(bool enableState) {
  if (auto oscEnableButtonLocked = oscEnableButton_.lock()) {
    oscEnableButtonLocked->setToggleState(enableState, dontSendNotification);
  }
  cachedOscEnable_ = enableState;
}

void BinauralMonitoringJuceFrontendConnector::setOscPort(int port) {
  if (auto oscPortLabelLocked = oscPortControl_.lock()) {
    oscPortLabelLocked->setValue(port, dontSendNotification);
  }
  cachedOscPort_ = port;
}

void BinauralMonitoringJuceFrontendConnector::setOscInvertYaw(bool invert) {
  if (auto oscInvertButtonLocked = oscInvertYawButton_.lock()) {
    oscInvertButtonLocked->setToggleState(invert, dontSendNotification);
  }
  cachedOscInvertYaw_ = invert;
}

void BinauralMonitoringJuceFrontendConnector::setOscInvertPitch(bool invert) {
  if (auto oscInvertButtonLocked = oscInvertPitchButton_.lock()) {
    oscInvertButtonLocked->setToggleState(invert, dontSendNotification);
  }
  cachedOscInvertPitch_ = invert;
}

void BinauralMonitoringJuceFrontendConnector::setOscInvertRoll(bool invert) {
  if (auto oscInvertButtonLocked = oscInvertRollButton_.lock()) {
    oscInvertButtonLocked->setToggleState(invert, dontSendNotification);
  }
  cachedOscInvertRoll_ = invert;
}

void BinauralMonitoringJuceFrontendConnector::setOscInvertQuatW(bool invert) {
  if (auto oscInvertButtonLocked = oscInvertQuatWButton_.lock()) {
    oscInvertButtonLocked->setToggleState(invert, dontSendNotification);
  }
  cachedOscInvertQuatW_ = invert;
}

void BinauralMonitoringJuceFrontendConnector::setOscInvertQuatX(bool invert) {
  if (auto oscInvertButtonLocked = oscInvertQuatXButton_.lock()) {
    oscInvertButtonLocked->setToggleState(invert, dontSendNotification);
  }
  cachedOscInvertQuatX_ = invert;
}

void BinauralMonitoringJuceFrontendConnector::setOscInvertQuatY(bool invert) {
  if (auto oscInvertButtonLocked = oscInvertQuatYButton_.lock()) {
    oscInvertButtonLocked->setToggleState(invert, dontSendNotification);
  }
  cachedOscInvertQuatY_ = invert;
}

void BinauralMonitoringJuceFrontendConnector::setOscInvertQuatZ(bool invert) {
  if (auto oscInvertButtonLocked = oscInvertQuatZButton_.lock()) {
    oscInvertButtonLocked->setToggleState(invert, dontSendNotification);
  }
  cachedOscInvertQuatZ_ = invert;
}

void BinauralMonitoringJuceFrontendConnector::setRendererStatus(
    const ear::plugin::BearStatus& bearStatus) {

  if (bearStatus.startupSuccess == BearStatusStates::SUCCEEDED) {
    // Is running. See if accepting listener position data
    if (bearStatus.listenerDataSetSuccess == BearStatusStates::FAILED) {
      juce::String msg("Renderer did not accept listener orientation data: ");
      msg += juce::String(bearStatus.listenerDataSetErrorDesc);
      setRendererStatus(msg, ear::plugin::ui::EarColours::StatusWarning);
    } else {
      // Other states (good or insignificant) for listener data - BEAR is running
      setRendererStatus("Running...", ear::plugin::ui::EarColours::Label);
    }

  } else if (bearStatus.startupSuccess == BearStatusStates::FAILED) {
    juce::String msg("Renderer startup failed: ");
    msg += juce::String(bearStatus.startupErrorDesc) + " (";
    auto dataFile = bearStatus.startupConfig.get_data_path();
    removePath(dataFile);
    msg += juce::String(dataFile) + ").";
    setRendererStatus(msg, ear::plugin::ui::EarColours::StatusBad);

  } else if (bearStatus.startupSuccess == BearStatusStates::NOT_ATTEMPTED) {
    // Unusual case - we should always expect a start attempt (class init)
    assert(false);
    setRendererStatus("Renderer not started.",
                      ear::plugin::ui::EarColours::StatusWarning);

  } else {
    // Unimplemented case
    assert(false);
  }
}

void BinauralMonitoringJuceFrontendConnector::setRendererStatus(
    const juce::String& statusText, const juce::Colour& statusColour) {
  if (auto rendererStatusLabelLocked = rendererStatusLabel_.lock()) {
    rendererStatusLabelLocked->setColour(juce::Label::textColourId,
                                         statusColour);
    rendererStatusLabelLocked->setText(
        statusText, juce::NotificationType::dontSendNotification);
  }

  cachedBearStatusText_ = statusText;
  cachedBearStatusColour_ = statusColour;
}

void BinauralMonitoringJuceFrontendConnector::setRendererStatusRestarting() {
  setRendererStatus(juce::String("Renderer starting..."),
                    ear::plugin::ui::EarColours::Label);
}

void BinauralMonitoringJuceFrontendConnector::setDataFile(
    const juce::String& dataFile) {
  if (auto cbLocked = dataFileComboBox_.lock()) {
    cbLocked->setSelectedId(dataFile, dontSendNotification);
  }
  p_->dataFileManager.setSelectedDataFile(dataFile); // fires callback to restart bear
}

void BinauralMonitoringJuceFrontendConnector::orientationChange(
    ear::plugin::ListenerOrientation::Euler euler) {
  // ListenerOrientation callback from backend
  // ALL other orientation I/O to be set from here

  // Parameters - Temporarily disable listeners to avoid recursive loop
  parameterListenersEnabled = false;
  *(p_->getYaw()) = euler.y;
  *(p_->getPitch()) = euler.p;
  *(p_->getRoll()) = euler.r;
  parameterListenersEnabled = true;

  // UI - dontSendNotification to avoid recursive loop
  updater_.callOnMessageThread([euler, yaw = yawControl_, pitch = pitchControl_, roll = rollControl_](){
      if(auto control = yaw.lock()) {
          control->setValue(euler.y, dontSendNotification);
      }
      if(auto control = pitch.lock()) {
          control->setValue(euler.p, dontSendNotification);
      }
      if(auto control = roll.lock()) {
          control->setValue(euler.r, dontSendNotification);
      }
  });
}

void BinauralMonitoringJuceFrontendConnector::comboBoxChanged(
    EarComboBox* comboBoxThatHasChanged) {
  if (auto comboBox =
          lockIfSame(dataFileComboBox_, comboBoxThatHasChanged)) {
    setDataFile(comboBox->getSelectedId());
  }
}

void BinauralMonitoringJuceFrontendConnector::parameterValueChanged(
    int parameterIndex, float newValue) {
  // Parameter change callback from JUCE Processor
  if (!parameterListenersEnabled) return;

  if (parameterIndex == (int)ParameterId::YAW ||
      parameterIndex == (int)ParameterId::PITCH ||
      parameterIndex == (int)ParameterId::ROLL) {
    updater_.callOnMessageThread([this, parameterIndex, newValue]() {
      if (parameterIndex == (int)ParameterId::YAW) {
        notifyParameterChanged(ParameterId::YAW, p_->getYaw()->get());
      } else if (parameterIndex == (int)ParameterId::PITCH) {
        notifyParameterChanged(ParameterId::PITCH, p_->getPitch()->get());
      } else if (parameterIndex == (int)ParameterId::ROLL) {
        notifyParameterChanged(ParameterId::ROLL, p_->getRoll()->get());
      }
    });

    if (listenerOrientation) {
      listenerOrientation->setEuler(ear::plugin::ListenerOrientation::Euler{
          p_->getYaw()->get(), p_->getPitch()->get(), p_->getRoll()->get(),
          ear::plugin::ListenerOrientation::EulerOrder::YPR});
    }

  } else if (parameterIndex == (int)ParameterId::OSC_ENABLE) {
    updater_.callOnMessageThread([this, parameterIndex, newValue]() {
      notifyParameterChanged(ParameterId::OSC_ENABLE,
                             p_->getOscEnable()->get());
      setOscEnable(p_->getOscEnable()->get());
    });

  } else if (parameterIndex == (int)ParameterId::OSC_PORT) {
    updater_.callOnMessageThread([this, parameterIndex, newValue]() {
      notifyParameterChanged(ParameterId::OSC_PORT, p_->getOscPort()->get());
      setOscPort(p_->getOscPort()->get());
    });

  } else if (parameterIndex == (int)ParameterId::OSC_INVERT_YAW) {
    updater_.callOnMessageThread([this, parameterIndex, newValue]() {
      notifyParameterChanged(ParameterId::OSC_INVERT_YAW,
                             p_->getOscInvertYaw()->get());
      setOscInvertYaw(p_->getOscInvertYaw()->get());
    });

  } else if (parameterIndex == (int)ParameterId::OSC_INVERT_PITCH) {
    updater_.callOnMessageThread([this, parameterIndex, newValue]() {
      notifyParameterChanged(ParameterId::OSC_INVERT_PITCH,
                             p_->getOscInvertPitch()->get());
      setOscInvertPitch(p_->getOscInvertPitch()->get());
    });

  } else if (parameterIndex == (int)ParameterId::OSC_INVERT_ROLL) {
    updater_.callOnMessageThread([this, parameterIndex, newValue]() {
      notifyParameterChanged(ParameterId::OSC_INVERT_ROLL,
                             p_->getOscInvertRoll()->get());
      setOscInvertRoll(p_->getOscInvertRoll()->get());
    });

  } else if (parameterIndex == (int)ParameterId::OSC_INVERT_QUAT_W) {
    updater_.callOnMessageThread([this, parameterIndex, newValue]() {
      notifyParameterChanged(ParameterId::OSC_INVERT_QUAT_W,
                             p_->getOscInvertQuatW()->get());
      setOscInvertQuatW(p_->getOscInvertQuatW()->get());
    });

  } else if (parameterIndex == (int)ParameterId::OSC_INVERT_QUAT_X) {
    updater_.callOnMessageThread([this, parameterIndex, newValue]() {
      notifyParameterChanged(ParameterId::OSC_INVERT_QUAT_X,
                             p_->getOscInvertQuatX()->get());
      setOscInvertQuatX(p_->getOscInvertQuatX()->get());
    });

  } else if (parameterIndex == (int)ParameterId::OSC_INVERT_QUAT_Y) {
    updater_.callOnMessageThread([this, parameterIndex, newValue]() {
      notifyParameterChanged(ParameterId::OSC_INVERT_QUAT_Y,
                             p_->getOscInvertQuatY()->get());
      setOscInvertQuatY(p_->getOscInvertQuatY()->get());
    });

  } else if (parameterIndex == (int)ParameterId::OSC_INVERT_QUAT_Z) {
    updater_.callOnMessageThread([this, parameterIndex, newValue]() {
      notifyParameterChanged(ParameterId::OSC_INVERT_QUAT_Z,
                             p_->getOscInvertQuatZ()->get());
      setOscInvertQuatZ(p_->getOscInvertQuatZ()->get());
    });

  }
}

void BinauralMonitoringJuceFrontendConnector::orientationValueChanged(
    ear::plugin::ui::OrientationView* view) {
  // Control callback from UI
  if (listenerOrientation) {
    auto euler = listenerOrientation->getEuler();

    if (auto yawControl = lockIfSame(yawControl_, view)) {
      euler.y = yawControl->getValue();

    } else if (auto pitchControl = lockIfSame(pitchControl_, view)) {
      euler.p = pitchControl->getValue();

    } else if (auto rollControl = lockIfSame(rollControl_, view)) {
      euler.r = rollControl->getValue();
    }

    listenerOrientation->setEuler(euler);
  }
}

void BinauralMonitoringJuceFrontendConnector::orientationDragStarted(
    ear::plugin::ui::OrientationView* view) {
  if (auto yawControl = lockIfSame(yawControl_, view)) {
    p_->getYaw()->beginChangeGesture();

  } else if (auto pitchControl = lockIfSame(pitchControl_, view)) {
    p_->getPitch()->beginChangeGesture();

  } else if (auto rollControl = lockIfSame(rollControl_, view)) {
    p_->getRoll()->beginChangeGesture();
  }
}

void BinauralMonitoringJuceFrontendConnector::orientationDragEnded(
    ear::plugin::ui::OrientationView* view) {
  if (auto yawControl = lockIfSame(yawControl_, view)) {
    p_->getYaw()->endChangeGesture();

  } else if (auto pitchControl = lockIfSame(pitchControl_, view)) {
    p_->getPitch()->endChangeGesture();

  } else if (auto rollControl = lockIfSame(rollControl_, view)) {
    p_->getRoll()->endChangeGesture();
  }
}

void BinauralMonitoringJuceFrontendConnector::buttonClicked(Button* button) {
  // note: getToggleState still has the old value for toggle types when this is called
  //       due to setClickingTogglesState on the button intentionally being false
  if (auto oscButton = lockIfSame(oscEnableButton_, button)) {
    bool newState = !oscButton->getToggleState();
    *(p_->getOscEnable()) = newState;
  }
  if (auto oscButton = lockIfSame(oscInvertYawButton_, button)) {
    bool newState = !oscButton->getToggleState();
    *(p_->getOscInvertYaw()) = newState;
  }
  if (auto oscButton = lockIfSame(oscInvertPitchButton_, button)) {
    bool newState = !oscButton->getToggleState();
    *(p_->getOscInvertPitch()) = newState;
  }
  if (auto oscButton = lockIfSame(oscInvertRollButton_, button)) {
    bool newState = !oscButton->getToggleState();
    *(p_->getOscInvertRoll()) = newState;
  }
  if (auto oscButton = lockIfSame(oscInvertQuatWButton_, button)) {
    bool newState = !oscButton->getToggleState();
    *(p_->getOscInvertQuatW()) = newState;
  }
  if (auto oscButton = lockIfSame(oscInvertQuatXButton_, button)) {
    bool newState = !oscButton->getToggleState();
    *(p_->getOscInvertQuatX()) = newState;
  }
  if (auto oscButton = lockIfSame(oscInvertQuatYButton_, button)) {
    bool newState = !oscButton->getToggleState();
    *(p_->getOscInvertQuatY()) = newState;
  }
  if (auto oscButton = lockIfSame(oscInvertQuatZButton_, button)) {
    bool newState = !oscButton->getToggleState();
    *(p_->getOscInvertQuatZ()) = newState;
  }
}

void BinauralMonitoringJuceFrontendConnector::sliderValueChanged(
    Slider* slider) {
  if (auto oscPortControl = lockIfSame(oscPortControl_, slider)) {
    *(p_->getOscPort()) = oscPortControl->getValue();
  }
}

void BinauralMonitoringJuceFrontendConnector::sliderDragStarted(
    Slider* slider) {
  if (auto oscPortControl = lockIfSame(oscPortControl_, slider)) {
    p_->getOscPort()->beginChangeGesture();
  }
}

void BinauralMonitoringJuceFrontendConnector::sliderDragEnded(Slider* slider) {
  if (auto oscPortControl = lockIfSame(oscPortControl_, slider)) {
    p_->getOscPort()->endChangeGesture();
  }
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
