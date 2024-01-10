#pragma once
#include "ui/binaural_monitoring_frontend_backend_connector.hpp"

#include "JuceHeader.h"

#include "components/orientation.hpp"
#include "components/ear_button.hpp"
#include "components/ear_combo_box.hpp"
#include "components/ear_slider.hpp"
#include "helper/multi_async_updater.h"

#include "binaural_monitoring_plugin_processor.hpp"
#include "binaural_monitoring_audio_processor.hpp"
#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class BinauralMonitoringJuceFrontendConnector
    : public ear::plugin::ui::BinauralMonitoringFrontendBackendConnector,
      private AudioProcessorParameter::Listener,
      Slider::Listener,
      Button::Listener,
      ear::plugin::ui::EarComboBox::Listener,
      ear::plugin::ui::OrientationView::Listener,
      ear::plugin::ListenerOrientation::EulerListener {
 public:
  /**
   * Note: Make sure that all AudioProcessorParameters are already
   * added to the ObjectsAudioProcessor at the time the ctor
   * is called.
   */
  BinauralMonitoringJuceFrontendConnector(
      EarBinauralMonitoringAudioProcessor* processor);
  ~BinauralMonitoringJuceFrontendConnector();

  void parameterValueChanged(int parameterIndex, float newValue) override;
  void parameterGestureChanged(int parameterIndex,
                               bool gestureIsStarting) override{};

  // Orientation Controls
  void setYawView(std::shared_ptr<OrientationView> view);
  void setPitchView(std::shared_ptr<OrientationView> view);
  void setRollView(std::shared_ptr<OrientationView> view);

  // OSC Controls
  void setOscEnableButton(std::shared_ptr<EarButton> button);
  void setOscPortControl(std::shared_ptr<EarSlider> slider);
  void setOscInvertYawButton(std::shared_ptr<ToggleButton> button);
  void setOscInvertPitchButton(std::shared_ptr<ToggleButton> button);
  void setOscInvertRollButton(std::shared_ptr<ToggleButton> button);
  void setOscInvertQuatWButton(std::shared_ptr<ToggleButton> button);
  void setOscInvertQuatXButton(std::shared_ptr<ToggleButton> button);
  void setOscInvertQuatYButton(std::shared_ptr<ToggleButton> button);
  void setOscInvertQuatZButton(std::shared_ptr<ToggleButton> button);

  // Renderer and Filter Set Selection Controls
  void setDataFileComponent(std::shared_ptr<Component> comp);
  void setDataFileComboBox(std::shared_ptr<EarComboBox> comboBox);
  void setRendererStatusLabel(std::shared_ptr<Label> label);

  // Listener Orientation Object
  void setListenerOrientationInstance(std::shared_ptr<ListenerOrientation> lo);

  // Value Setters
  void setYaw(float yaw);
  void setPitch(float pitch);
  void setRoll(float roll);
  void setEuler(ListenerOrientation::Euler euler);
  void setQuaternion(ListenerOrientation::Quaternion quat);

  void setOscEnable(bool enable);
  void setOscPort(int port);
  void setOscInvertYaw(bool invert);
  void setOscInvertPitch(bool invert);
  void setOscInvertRoll(bool invert);
  void setOscInvertQuatW(bool invert);
  void setOscInvertQuatX(bool invert);
  void setOscInvertQuatY(bool invert);
  void setOscInvertQuatZ(bool invert);

  // Renderer Status update
  void setRendererStatus(const ear::plugin::BearStatus& bearStatus);

 protected:
  // Orientation::Listener
  void orientationValueChanged(ear::plugin::ui::OrientationView* view) override;
  void orientationDragStarted(ear::plugin::ui::OrientationView* view) override;
  void orientationDragEnded(ear::plugin::ui::OrientationView* view) override;

  // Button::Listener
  void buttonClicked(Button* button) override;

  // Slider::Listener
  void sliderValueChanged(Slider* slider) override;
  void sliderDragStarted(Slider*) override;
  void sliderDragEnded(Slider*) override;

  // ListenerOrientation::EulerListener
  void orientationChange(
      ear::plugin::ListenerOrientation::Euler euler) override;

  // EarComboBox::Listener
  void comboBoxChanged(EarComboBox* comboBoxThatHasChanged) override;

 private:
  EarBinauralMonitoringAudioProcessor* p_;
  std::map<int, RangedAudioParameter*> parameters_;

  bool parameterListenersEnabled{true};

  // Orientation Controls
  std::weak_ptr<OrientationView> yawControl_;
  std::weak_ptr<OrientationView> pitchControl_;
  std::weak_ptr<OrientationView> rollControl_;

  // OSC Controls
  std::weak_ptr<EarButton> oscEnableButton_;
  std::weak_ptr<EarSlider> oscPortControl_;
  std::weak_ptr<ToggleButton> oscInvertYawButton_;
  std::weak_ptr<ToggleButton> oscInvertPitchButton_;
  std::weak_ptr<ToggleButton> oscInvertRollButton_;
  std::weak_ptr<ToggleButton> oscInvertQuatWButton_;
  std::weak_ptr<ToggleButton> oscInvertQuatXButton_;
  std::weak_ptr<ToggleButton> oscInvertQuatYButton_;
  std::weak_ptr<ToggleButton> oscInvertQuatZButton_;

  // Renderer Status and Filter Set Selection Controls
  std::weak_ptr<Component> dataFileComponent_;
  std::weak_ptr<EarComboBox> dataFileComboBox_;
  std::weak_ptr<Label> rendererStatusLabel_;

  // Values
  bool cachedOscEnable_;
  int cachedOscPort_;
  bool cachedOscInvertYaw_;
  bool cachedOscInvertPitch_;
  bool cachedOscInvertRoll_;
  bool cachedOscInvertQuatW_;
  bool cachedOscInvertQuatX_;
  bool cachedOscInvertQuatY_;
  bool cachedOscInvertQuatZ_;
  ear::plugin::BearStatus cachedBearStatus_;

  /// Listener Orientation Object
  std::shared_ptr<ListenerOrientation> listenerOrientation;

  MultiAsyncUpdater updater_;
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
