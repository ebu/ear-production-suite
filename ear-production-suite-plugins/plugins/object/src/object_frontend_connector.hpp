#pragma once
#include "ui/object_frontend_backend_connector.hpp"

#include "JuceHeader.h"

#include "components/ear_button.hpp"
#include "components/ear_combo_box.hpp"
#include "components/panner_top_view.hpp"
#include "components/panner_side_view.hpp"
#include "helper/multi_async_updater.h"
#include "object_plugin_processor.hpp"
#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class EarComboBox;
class EarNameTextEditor;
class EarSlider;
class EarInvertedSlider;
class PannerTopView;
class PannerSideView;

class ObjectsJuceFrontendConnector
    : public ear::plugin::ui::ObjectsFrontendBackendConnector,
      private AudioProcessorParameter::Listener,
      TextEditor::Listener,
      Slider::Listener,
      Button::Listener,
      ear::plugin::ui::PannerTopView::Listener,
      ear::plugin::ui::PannerSideView::Listener,
      ear::plugin::ui::EarComboBox::Listener {
 public:
  /**
   * Note: Make sure that all AudioProcessorParameters are already
   * added to the ObjectsAudioProcessor at the time the ctor
   * is called.
   */
  ObjectsJuceFrontendConnector(ObjectsAudioProcessor* processor);
  ~ObjectsJuceFrontendConnector();

  void parameterValueChanged(int parameterIndex, float newValue) override;
  void parameterGestureChanged(int parameterIndex,
                               bool gestureIsStarting) override{};

  void trackPropertiesChanged(
      const AudioProcessor::TrackProperties& properties);

  // Main Value Box
  void setStatusBarLabel(std::shared_ptr<Label> statusBarLabel);
  void setNameTextEditor(std::shared_ptr<EarNameTextEditor> nameTextEditor);
  void setColourComboBox(std::shared_ptr<EarComboBox> colourComboBox);
  void setRoutingComboBox(std::shared_ptr<EarComboBox> routingComboBox);

  // Panning Value Box
  void setGainSlider(std::shared_ptr<EarSlider> gainSlider);
  void setAzimuthSlider(std::shared_ptr<EarInvertedSlider> azimuthSlider);
  void setElevationSlider(std::shared_ptr<EarSlider> elevationSlider);
  void setDistanceSlider(std::shared_ptr<EarSlider> distanceSlider);

  // Extent Value Box
  void setLinkSizeButton(std::shared_ptr<EarButton> button);
  void setSizeLabel(std::shared_ptr<Label> label);
  void setSizeSlider(std::shared_ptr<EarSlider> slider);
  void setWidthLabel(std::shared_ptr<Label> label);
  void setWidthSlider(std::shared_ptr<EarSlider> slider);
  void setHeightLabel(std::shared_ptr<Label> label);
  void setHeightSlider(std::shared_ptr<EarSlider> slider);
  void setDepthLabel(std::shared_ptr<Label> label);
  void setDepthSlider(std::shared_ptr<EarSlider> slider);
  void setDiffuseSlider(std::shared_ptr<EarSlider> slider);
  void setDivergenceButton(std::shared_ptr<EarButton> button);
  void setFactorLabel(std::shared_ptr<Label> label);
  void setFactorSlider(std::shared_ptr<EarSlider> slider);
  void setRangeLabel(std::shared_ptr<Label> label);
  void setRangeSlider(std::shared_ptr<EarSlider> slider);

  // Panning View Value Box
  void setPannerTopView(std::shared_ptr<PannerTopView> panner);
  void setPannerSideView(std::shared_ptr<PannerSideView> panner);

  // Value setter
  void setName(const std::string& name);
  void setColour(Colour colour);
  void setRouting(int routing);
  void setGain(float gain);

  void setAzimuth(float azimuth);
  void setElevation(float elevation);
  void setDistance(float distance);

  void setLinkSize(bool value);
  void setSize(float value);
  void setWidth(float value);
  void setHeight(float value);
  void setDepth(float value);
  void setDiffuse(float value);
  void setDivergence(bool value);
  void setFactor(float value);
  void setRange(float value);

 protected:
  // Slider::Listener
  void sliderValueChanged(Slider* slider) override;
  void sliderDragStarted(Slider*) override;
  void sliderDragEnded(Slider*) override;

  // PannerTopView::Listener
  void pannerValueChanged(ear::plugin::ui::PannerTopView* panner) override;
  void pannerDragStarted(ear::plugin::ui::PannerTopView* panner) override;
  void pannerDragEnded(ear::plugin::ui::PannerTopView* panner) override;

  // PannerSideView::Listener
  void pannerValueChanged(ear::plugin::ui::PannerSideView* panner) override;
  void pannerDragStarted(ear::plugin::ui::PannerSideView* panner) override;
  void pannerDragEnded(ear::plugin::ui::PannerSideView* panner) override;

  // EarComboBox::Listener
  void comboBoxChanged(
      ear::plugin::ui::EarComboBox* comboBoxThatHasChanged) override;

  // ear::plugin::ui::InputFrontendBackendConnector
  void doSetStatusBarText(const std::string& text) override;

  // Button::Listener
  void buttonClicked(Button*) override;

 private:
  ObjectsAudioProcessor* p_;
  std::map<int, RangedAudioParameter*> parameters_;

  // Main Value Box
  std::weak_ptr<EarComboBox> colourComboBox_;
  std::weak_ptr<EarComboBox> routingComboBox_;
  std::weak_ptr<EarNameTextEditor> nameTextEditor_;

  // Panning Value Box
  std::weak_ptr<EarSlider> gainSlider_;
  std::weak_ptr<EarInvertedSlider> azimuthSlider_;
  std::weak_ptr<EarSlider> elevationSlider_;
  std::weak_ptr<EarSlider> distanceSlider_;

  // Extent Value Box
  std::weak_ptr<EarButton> linkSizeButton_;
  std::weak_ptr<Label> sizeLabel_;
  std::weak_ptr<EarSlider> sizeSlider_;
  std::weak_ptr<Label> widthLabel_;
  std::weak_ptr<EarSlider> widthSlider_;
  std::weak_ptr<Label> heightLabel_;
  std::weak_ptr<EarSlider> heightSlider_;
  std::weak_ptr<Label> depthLabel_;
  std::weak_ptr<EarSlider> depthSlider_;
  std::weak_ptr<EarSlider> diffuseSlider_;
  std::weak_ptr<EarButton> divergenceButton_;
  std::weak_ptr<Label> factorLabel_;
  std::weak_ptr<EarSlider> factorSlider_;
  std::weak_ptr<Label> rangeLabel_;
  std::weak_ptr<EarSlider> rangeSlider_;

  // Panning View Value Box
  std::weak_ptr<PannerTopView> pannerTopView_;
  std::weak_ptr<PannerSideView> pannerSideView_;

  std::weak_ptr<Label> statusBar_;

  MultiAsyncUpdater updater_;

  // Values
  Colour cachedColour_{ juce::Colours::transparentBlack };
  int cachedRouting_{ -1 };
  std::string cachedName_{};
  float cachedGain_{ 1.f };
  float cachedAzimuth_{ 0.f };
  float cachedElevation_{ 0.f };
  float cachedDistance_{ 1.f };
  bool cachedLinkSize_{ false };
  float cachedSize_{ 1.f };
  float cachedWidth_{ 1.f };
  float cachedHeight_{ 1.f };
  float cachedDepth_{ 1.f };
  float cachedDiffuse_{ 0.f };
  bool cachedDivergence_{ false };
  float cachedFactor_{ 0.f };
  float cachedRange_{ 0.f };
  std::string cachedStatusBarText_{};
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
