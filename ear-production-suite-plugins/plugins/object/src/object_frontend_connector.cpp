#include "object_frontend_connector.hpp"

#include "components/ear_combo_box.hpp"
#include "components/ear_name_text_editor.hpp"
#include "components/ear_inverted_slider.hpp"
#include "components/panner_top_view.hpp"
#include "components/panner_side_view.hpp"

#include "detail/weak_ptr_helpers.hpp"

namespace ear {
namespace plugin {
namespace ui {

using detail::lockIfSame;

inline bool clipToBool(float value) { return value < 0.5 ? false : true; }

ObjectsJuceFrontendConnector::ObjectsJuceFrontendConnector(
    ObjectsAudioProcessor* processor)
    : ObjectsFrontendBackendConnector(), p_(processor) {
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

ObjectsJuceFrontendConnector::~ObjectsJuceFrontendConnector() {
  for (auto parameter : parameters_) {
    if (parameter.second) {
      parameter.second->removeListener(this);
    }
  }
  if (auto comboBox = routingComboBox_.lock()) {
    comboBox->removeListener(this);
  }
  if (auto comboBox = colourComboBox_.lock()) {
    comboBox->removeListener(this);
  }
  if (auto slider = gainSlider_.lock()) {
    slider->removeListener(this);
  }
  if (auto slider = azimuthSlider_.lock()) {
    slider->removeListener(this);
  }
  if (auto slider = elevationSlider_.lock()) {
    slider->removeListener(this);
  }
  if (auto slider = distanceSlider_.lock()) {
    slider->removeListener(this);
  }
  if (auto slider = sizeSlider_.lock()) {
    slider->removeListener(this);
  }
  if (auto slider = widthSlider_.lock()) {
    slider->removeListener(this);
  }
  if (auto slider = heightSlider_.lock()) {
    slider->removeListener(this);
  }
  if (auto slider = depthSlider_.lock()) {
    slider->removeListener(this);
  }
  if (auto slider = diffuseSlider_.lock()) {
    slider->removeListener(this);
  }
  if (auto slider = factorSlider_.lock()) {
    slider->removeListener(this);
  }
  if (auto slider = rangeSlider_.lock()) {
    slider->removeListener(this);
  }
  if (auto button = linkSizeButton_.lock()) {
    button->removeListener(this);
  }
  if (auto button = divergenceButton_.lock()) {
    button->removeListener(this);
  }
  if (auto button = useTrackNameCheckbox_.lock()) {
    button->removeListener(this);
  }
  if (auto panner = pannerTopView_.lock()) {
    panner->removeListener(this);
  }
  if (auto panner = pannerSideView_.lock()) {
    panner->removeListener(this);
  }
}

void ObjectsJuceFrontendConnector::setStatusBarLabel(
    std::shared_ptr<Label> statusBarLabel) {
  statusBar_ = statusBarLabel;
  doSetStatusBarText(cachedStatusBarText_);
}

void ObjectsJuceFrontendConnector::doSetStatusBarText(const std::string& text) {
  auto statusBar = statusBar_;

  updater_.callOnMessageThread([text, statusBar]() {
    if (auto statusBarLocked = statusBar.lock()) {
      statusBarLocked->setText(text, dontSendNotification);
    }
  });
  cachedStatusBarText_ = text;
}

void ObjectsJuceFrontendConnector::setNameTextEditor(
    std::shared_ptr<EarNameTextEditor> textEditor) {
  textEditor->addListener(this);
  nameTextEditor_ = textEditor;
  updateNameState();
}

void ObjectsJuceFrontendConnector::setUseTrackNameCheckbox(
    std::shared_ptr<ToggleButton> useTrackNameCheckbox){
  useTrackNameCheckbox->addListener(this);
  useTrackNameCheckbox_ = useTrackNameCheckbox;
  setUseTrackName(cachedUseTrackName_);
}

void ObjectsJuceFrontendConnector::setColourComboBox(
    std::shared_ptr<EarComboBox> comboBox) {
  comboBox->addListener(this);
  colourComboBox_ = comboBox;
  setColour(cachedColour_);
}

void ObjectsJuceFrontendConnector::setPannerTopView(
    std::shared_ptr<PannerTopView> panner) {
  panner->addListener(this);
  pannerTopView_ = panner;
  setColour(cachedColour_);
}

void ObjectsJuceFrontendConnector::setPannerSideView(
    std::shared_ptr<PannerSideView> panner) {
  panner->addListener(this);
  pannerSideView_ = panner;
  setColour(cachedColour_);
}

void ObjectsJuceFrontendConnector::setRoutingComboBox(
    std::shared_ptr<EarComboBox> comboBox) {
  comboBox->addListener(this);
  routingComboBox_ = comboBox;
  setRouting(cachedRouting_);
}

void ObjectsJuceFrontendConnector::setGainSlider(
    std::shared_ptr<EarSlider> slider) {
  slider->addListener(this);
  gainSlider_ = slider;
  setGain(cachedGain_);
}

void ObjectsJuceFrontendConnector::setAzimuthSlider(
    std::shared_ptr<EarInvertedSlider> slider) {
  slider->addListener(this);
  azimuthSlider_ = slider;
  setAzimuth(cachedAzimuth_);
}

void ObjectsJuceFrontendConnector::setElevationSlider(
    std::shared_ptr<EarSlider> slider) {
  slider->addListener(this);
  elevationSlider_ = slider;
  setElevation(cachedElevation_);
}

void ObjectsJuceFrontendConnector::setDistanceSlider(
    std::shared_ptr<EarSlider> slider) {
  slider->addListener(this);
  distanceSlider_ = slider;
  setDistance(cachedDistance_);
}

void ObjectsJuceFrontendConnector::setLinkSizeButton(
    std::shared_ptr<EarButton> button) {
  button->addListener(this);
  linkSizeButton_ = button;
  setLinkSize(cachedLinkSize_);
}
void ObjectsJuceFrontendConnector::setSizeLabel(std::shared_ptr<Label> label) {
  sizeLabel_ = label;
  setLinkSize(cachedLinkSize_);
}
void ObjectsJuceFrontendConnector::setSizeSlider(
    std::shared_ptr<EarSlider> slider) {
  slider->addListener(this);
  sizeSlider_ = slider;
  setSize(cachedSize_);
  setLinkSize(cachedLinkSize_);
}
void ObjectsJuceFrontendConnector::setWidthLabel(std::shared_ptr<Label> label) {
  widthLabel_ = label;
  setLinkSize(cachedLinkSize_);
}
void ObjectsJuceFrontendConnector::setWidthSlider(
    std::shared_ptr<EarSlider> slider) {
  slider->addListener(this);
  widthSlider_ = slider;
  setWidth(cachedWidth_);
  setLinkSize(cachedLinkSize_);
}
void ObjectsJuceFrontendConnector::setHeightLabel(
    std::shared_ptr<Label> label) {
  heightLabel_ = label;
  setLinkSize(cachedLinkSize_);
}
void ObjectsJuceFrontendConnector::setHeightSlider(
    std::shared_ptr<EarSlider> slider) {
  slider->addListener(this);
  heightSlider_ = slider;
  setHeight(cachedHeight_);
  setLinkSize(cachedLinkSize_);
}
void ObjectsJuceFrontendConnector::setDepthLabel(std::shared_ptr<Label> label) {
  depthLabel_ = label;
  setLinkSize(cachedLinkSize_);
}
void ObjectsJuceFrontendConnector::setDepthSlider(
    std::shared_ptr<EarSlider> slider) {
  slider->addListener(this);
  depthSlider_ = slider;
  setDepth(cachedDepth_);
  setLinkSize(cachedLinkSize_);
}
void ObjectsJuceFrontendConnector::setDiffuseSlider(
    std::shared_ptr<EarSlider> slider) {
  slider->addListener(this);
  diffuseSlider_ = slider;
  setDiffuse(cachedDiffuse_);
}
void ObjectsJuceFrontendConnector::setDivergenceButton(
    std::shared_ptr<EarButton> button) {
  button->addListener(this);
  divergenceButton_ = button;
  setDivergence(cachedDivergence_);
}
void ObjectsJuceFrontendConnector::setFactorLabel(
    std::shared_ptr<Label> label) {
  factorLabel_ = label;
  setDivergence(cachedDivergence_);
}
void ObjectsJuceFrontendConnector::setFactorSlider(
    std::shared_ptr<EarSlider> slider) {
  slider->addListener(this);
  factorSlider_ = slider;
  setFactor(cachedFactor_);
  setDivergence(cachedDivergence_);
}
void ObjectsJuceFrontendConnector::setRangeLabel(std::shared_ptr<Label> label) {
  rangeLabel_ = label;
  setDivergence(cachedDivergence_);
}
void ObjectsJuceFrontendConnector::setRangeSlider(
    std::shared_ptr<EarSlider> slider) {
  slider->addListener(this);
  rangeSlider_ = slider;
  setRange(cachedRange_);
  setDivergence(cachedDivergence_);
}

void ObjectsJuceFrontendConnector::setName(const std::string& name) {
  if (auto nameTextEditorLocked = nameTextEditor_.lock()) {
    nameTextEditorLocked->setText(name);
  }
  if(!cachedUseTrackName_ || lastKnownCustomName_.empty()) {
    lastKnownCustomName_ = name;
  }
  cachedName_ = name;
  // Normally we'd set a processor parameter which in turn calls
  //   ObjectsJuceFrontendConnector::parameterValueChanged as a listener,
  //   which in turns fires notifyParameterChanged.
  // Instead, we must do that directly (name state not held by a parameter)
  notifyParameterChanged(ParameterId::NAME, cachedName_);
}

void ObjectsJuceFrontendConnector::setUseTrackName(bool useTrackName)
{
  auto useTrackNameCheckboxLocked = useTrackNameCheckbox_.lock();
  auto nameTextEditorLocked = nameTextEditor_.lock();

  if (useTrackNameCheckboxLocked && nameTextEditorLocked) {
    auto oldState = useTrackNameCheckboxLocked->getToggleState();
    if(oldState != useTrackName) {
      useTrackNameCheckboxLocked->setToggleState(useTrackName, dontSendNotification);
    }
  }
  cachedUseTrackName_ = useTrackName;
  updateNameState();
}

void ObjectsJuceFrontendConnector::setColour(Colour colour) {
  if (auto colourComboBoxLocked = colourComboBox_.lock()) {
    colourComboBoxLocked->clearEntries();
    colourComboBoxLocked->addColourEntry(colour);
    colourComboBoxLocked->selectEntry(0, dontSendNotification);
  }
  if (auto pannerTopViewLocked = pannerTopView_.lock()) {
    pannerTopViewLocked->setColour(PannerTopView::highlightColourId, colour);
    pannerTopViewLocked->repaint();
  }
  if (auto pannerSideViewLocked = pannerSideView_.lock()) {
    pannerSideViewLocked->setColour(PannerSideView::highlightColourId, colour);
    pannerSideViewLocked->repaint();
  }
  cachedColour_ = colour;
}

void ObjectsJuceFrontendConnector::setRouting(int routing) {
  if (auto routingComboBoxLocked = routingComboBox_.lock()) {
    routingComboBoxLocked->selectEntry(routing, dontSendNotification);
  }
  cachedRouting_ = routing;
}

void ObjectsJuceFrontendConnector::setGain(float gain) {
  if (auto gainSliderLocked = gainSlider_.lock()) {
    gainSliderLocked->setValue(gain, dontSendNotification);
  }
  cachedGain_ = gain;
}

void ObjectsJuceFrontendConnector::setAzimuth(float azimuth) {
  if (auto azimuthSliderLocked = azimuthSlider_.lock()) {
    azimuthSliderLocked->setValue(azimuth, dontSendNotification);
  }
  if (auto pannerTopViewLocked = pannerTopView_.lock()) {
    pannerTopViewLocked->setAzimuth(azimuth, dontSendNotification);
  }
  cachedAzimuth_ = azimuth;
}

void ObjectsJuceFrontendConnector::setElevation(float elevation) {
  if (auto elevationSliderLocked = elevationSlider_.lock()) {
    elevationSliderLocked->setValue(elevation, dontSendNotification);
  }
  if (auto pannerSideViewLocked = pannerSideView_.lock()) {
    pannerSideViewLocked->setElevation(elevation, dontSendNotification);
  }
  cachedElevation_ = elevation;
}

void ObjectsJuceFrontendConnector::setDistance(float distance) {
  if (auto distanceSliderLocked = distanceSlider_.lock()) {
    distanceSliderLocked->setValue(distance, dontSendNotification);
  }
  if (auto pannerTopViewLocked = pannerTopView_.lock()) {
    pannerTopViewLocked->setDistance(distance, dontSendNotification);
  }
  cachedDistance_ = distance;
}

void ObjectsJuceFrontendConnector::setLinkSize(bool linkSize) {
  auto linkSizeButtonLocked = linkSizeButton_.lock();
  auto sizeLabelLocked = sizeLabel_.lock();
  auto sizeSliderLocked = sizeSlider_.lock();
  auto widthLabelLocked = widthLabel_.lock();
  auto widthSliderLocked = widthSlider_.lock();
  auto heightLabelLocked = heightLabel_.lock();
  auto heightSliderLocked = heightSlider_.lock();
  auto depthLabelLocked = depthLabel_.lock();
  auto depthSliderLocked = depthSlider_.lock();
  if (linkSizeButtonLocked && sizeLabelLocked && sizeSliderLocked &&
      widthLabelLocked && widthSliderLocked && heightLabelLocked &&
      heightSliderLocked && depthLabelLocked && depthSliderLocked) {
    linkSizeButtonLocked->setToggleState(linkSize, dontSendNotification);
    if (linkSize) {
      sizeSliderLocked->setGrabFocusOnTextChange(false);
      widthSliderLocked->setGrabFocusOnTextChange(true);
      heightSliderLocked->setGrabFocusOnTextChange(true);
      depthSliderLocked->setGrabFocusOnTextChange(true);
      sizeLabelLocked->setAlpha(Emphasis::disabled);
      sizeSliderLocked->setAlpha(Emphasis::disabled);
      sizeSliderLocked->setEnabled(false);
      widthLabelLocked->setAlpha(1.f);
      widthSliderLocked->setAlpha(1.f);
      widthSliderLocked->setEnabled(true);
      heightLabelLocked->setAlpha(1.f);
      heightSliderLocked->setAlpha(1.f);
      heightSliderLocked->setEnabled(true);
      depthLabelLocked->setAlpha(1.f);
      depthSliderLocked->setAlpha(1.f);
      depthSliderLocked->setEnabled(true);
      widthSliderLocked->grabKeyboardFocus();
    } else {
      widthSliderLocked->setValue(sizeSliderLocked->getValue(),
                                  sendNotificationAsync);
      heightSliderLocked->setValue(sizeSliderLocked->getValue(),
                                   sendNotificationAsync);
      depthSliderLocked->setValue(sizeSliderLocked->getValue(),
                                  sendNotificationAsync);
      sizeSliderLocked->setGrabFocusOnTextChange(true);
      widthSliderLocked->setGrabFocusOnTextChange(false);
      heightSliderLocked->setGrabFocusOnTextChange(false);
      depthSliderLocked->setGrabFocusOnTextChange(false);
      sizeLabelLocked->setAlpha(1.f);
      sizeSliderLocked->setAlpha(1.f);
      sizeSliderLocked->setEnabled(true);
      widthLabelLocked->setAlpha(Emphasis::disabled);
      widthSliderLocked->setAlpha(Emphasis::disabled);
      widthSliderLocked->setEnabled(false);
      heightLabelLocked->setAlpha(Emphasis::disabled);
      heightSliderLocked->setAlpha(Emphasis::disabled);
      heightSliderLocked->setEnabled(false);
      depthLabelLocked->setAlpha(Emphasis::disabled);
      depthSliderLocked->setAlpha(Emphasis::disabled);
      depthSliderLocked->setEnabled(false);
      sizeSliderLocked->grabKeyboardFocus();
    }
  }
  cachedLinkSize_ = linkSize;
}

void ObjectsJuceFrontendConnector::setSize(float size) {
  if (auto sizeSliderLocked = sizeSlider_.lock()) {
    sizeSliderLocked->setValue(size, dontSendNotification);
  }
  if (auto linkSizeLocked = linkSizeButton_.lock()) {
    if(linkSizeLocked->getToggleState()) {
      setWidth(size);
      setHeight(size);
      setDepth(size);
    }
  }
  cachedSize_ = size;
}

void ObjectsJuceFrontendConnector::setWidth(float width) {
  if (auto widthSliderLocked = widthSlider_.lock()) {
    widthSliderLocked->setValue(width, dontSendNotification);
  }
  cachedWidth_ = width;
}

void ObjectsJuceFrontendConnector::setHeight(float height) {
  if (auto heightSliderLocked = heightSlider_.lock()) {
    heightSliderLocked->setValue(height, dontSendNotification);
  }
  cachedHeight_ = height;
}

void ObjectsJuceFrontendConnector::setDepth(float depth) {
  if (auto depthSliderLocked = depthSlider_.lock()) {
    depthSliderLocked->setValue(depth, dontSendNotification);
  }
  cachedDepth_ = depth;
}

void ObjectsJuceFrontendConnector::setDiffuse(float diffuse) {
  if (auto diffuseSliderLocked = diffuseSlider_.lock()) {
    diffuseSliderLocked->setValue(diffuse, dontSendNotification);
  }
  cachedDiffuse_ = diffuse;
}

void ObjectsJuceFrontendConnector::setDivergence(bool divergence) {
  auto divergenceButtonLocked = divergenceButton_.lock();
  auto factorLabelLocked = factorLabel_.lock();
  auto factorSliderLocked = factorSlider_.lock();
  auto rangeLabelLocked = rangeLabel_.lock();
  auto rangeSliderLocked = rangeSlider_.lock();
  if (divergenceButtonLocked && rangeLabelLocked && rangeSliderLocked) {
    divergenceButtonLocked->setToggleState(divergence, dontSendNotification);
    if (divergence) {
      factorLabelLocked->setAlpha(Emphasis::full);
      factorSliderLocked->setEnabled(true);
      factorSliderLocked->setAlpha(Emphasis::full);
      rangeLabelLocked->setAlpha(Emphasis::full);
      rangeSliderLocked->setEnabled(true);
      rangeSliderLocked->setAlpha(Emphasis::full);
    } else {
      factorLabelLocked->setAlpha(Emphasis::disabled);
      factorSliderLocked->setEnabled(false);
      factorSliderLocked->setAlpha(Emphasis::disabled);
      rangeLabelLocked->setAlpha(Emphasis::disabled);
      rangeSliderLocked->setEnabled(false);
      rangeSliderLocked->setAlpha(Emphasis::disabled);
    }
  }
  cachedDivergence_ = divergence;
}

void ObjectsJuceFrontendConnector::setFactor(float factor) {
  if (auto factorSliderLocked = factorSlider_.lock()) {
    factorSliderLocked->setValue(factor, dontSendNotification);
  }
  cachedFactor_ = factor;
}

void ObjectsJuceFrontendConnector::setRange(float range) {
  if (auto rangeSliderLocked = rangeSlider_.lock()) {
    rangeSliderLocked->setValue(range, dontSendNotification);
  }
  cachedRange_ = range;
}

void ObjectsJuceFrontendConnector::parameterValueChanged(int parameterIndex,
                                                         float newValue) {
  updater_.callOnMessageThread([this, parameterIndex, newValue]() {
    using ParameterId = ui::ObjectsFrontendBackendConnector::ParameterId;
    switch (parameterIndex) {
      case 0:
        notifyParameterChanged(ParameterId::ROUTING, p_->getRouting()->get());
        setRouting(p_->getRouting()->get());
        break;
      case 1:
        notifyParameterChanged(ParameterId::GAIN,
                               Decibels::decibelsToGain(p_->getGain()->get()));
        setGain(p_->getGain()->get());
        break;
      case 2:
        notifyParameterChanged(ParameterId::AZIMUTH, p_->getAzimuth()->get());
        setAzimuth(p_->getAzimuth()->get());
        break;
      case 3:
        notifyParameterChanged(ParameterId::ELEVATION, p_->getElevation()->get());
        setElevation(p_->getElevation()->get());
        break;
      case 4:
        notifyParameterChanged(ParameterId::DISTANCE, p_->getDistance()->get());
        setDistance(p_->getDistance()->get());
        break;
      case 5:
        setLinkSize(p_->getLinkSize()->get());
        break;
      case 6:
        setSize(p_->getSize()->get());
        break;
      case 7:
        notifyParameterChanged(ParameterId::WIDTH, p_->getWidth()->get() * 360.f);
        setWidth(p_->getWidth()->get());
        break;
      case 8:
        notifyParameterChanged(ParameterId::HEIGHT,
                               p_->getHeight()->get() * 360.f);
        setHeight(p_->getHeight()->get());
        break;
      case 9:
        notifyParameterChanged(ParameterId::DEPTH, p_->getDepth()->get());
        setDepth(p_->getDepth()->get());
        break;
      case 10:
        notifyParameterChanged(ParameterId::DIFFUSE, p_->getDiffuse()->get());
        setDiffuse(p_->getDiffuse()->get());
        break;
      case 11:
        setDivergence(p_->getDivergence()->get());
        if (p_->getDivergence()->get()) {
          notifyParameterChanged(ParameterId::FACTOR, p_->getFactor()->get());
          notifyParameterChanged(ParameterId::RANGE, p_->getRange()->get());
        } else {
          notifyParameterChanged(ParameterId::FACTOR, 0.f);
          notifyParameterChanged(ParameterId::RANGE, 0.f);
        }
        break;
      case 12:
        notifyParameterChanged(ParameterId::FACTOR, p_->getFactor()->get());
        setFactor(p_->getFactor()->get());
        break;
      case 13:
        notifyParameterChanged(ParameterId::RANGE, p_->getRange()->get());
        setRange(p_->getRange()->get());
        break;
      case 15:
        setUseTrackName(p_->getUseTrackName()->get());
        break;
    }
  });
}

void ObjectsJuceFrontendConnector::trackPropertiesChanged(
    const juce::AudioProcessor::TrackProperties& properties) {
  // It is unclear if this will be called from the message thread so to be sure
  // we call the following async using the MessageManager
  updater_.callOnMessageThread([properties, this]() {
    this->lastKnownTrackName_ = properties.name.toStdString();
    if(this->cachedUseTrackName_) {
      this->setName(this->lastKnownTrackName_);
      notifyParameterChanged(ParameterId::NAME, this->lastKnownTrackName_);
    }
    if (properties.colour.isTransparent()) {
      this->setColour(EarColours::PrimaryVariant);
      notifyParameterChanged(ParameterId::COLOUR,
                             EarColours::PrimaryVariant.getARGB());
    } else {
      this->setColour(properties.colour);
      notifyParameterChanged(ParameterId::COLOUR, properties.colour.getARGB());
    }
  });
}

namespace {
void performSliderAction(AudioParameterFloat* p,
              std::shared_ptr<Slider> const& slider,
              SliderAction action) {
  switch(action) {
    case SliderAction::UPDATE: {
      *p = static_cast<float>(slider->getValue());
      break;
    }
    case SliderAction::DRAG_START: {
      p->beginChangeGesture();
      break;
    }
    case SliderAction::DRAG_END: {
      p->endChangeGesture();
      break;
    }
    }
  }
}

void ObjectsJuceFrontendConnector::sliderValueChanged(Slider* slider) {
  dispatchSliderAction(slider, SliderAction::UPDATE);
}

void ObjectsJuceFrontendConnector::sliderDragStarted(Slider* slider) {
  dispatchSliderAction(slider, SliderAction::DRAG_START);
}

void ObjectsJuceFrontendConnector::sliderDragEnded(Slider* slider) {
  dispatchSliderAction(slider, SliderAction::DRAG_END);
}

void ObjectsJuceFrontendConnector::updateNameState()
{
  auto nameTextEditorLocked = nameTextEditor_.lock();
  if(nameTextEditorLocked) {
    if(cachedUseTrackName_) {
      nameTextEditorLocked->setEnabled(false);
      nameTextEditorLocked->setAlpha(0.38f);
    } else {
      nameTextEditorLocked->setEnabled(true);
      nameTextEditorLocked->setAlpha(1.f);
    }
  }
  if(cachedUseTrackName_ || lastKnownCustomName_.empty()) {
    setName(lastKnownTrackName_);
  } else {
    setName(lastKnownCustomName_);
  }
}

void ObjectsJuceFrontendConnector::dispatchSliderAction(Slider* slider, SliderAction action) {
  if(auto gainSlider = lockIfSame(gainSlider_, slider)) {
    performSliderAction(p_->getGain(), gainSlider, action);
    return;
  }
  if(auto azimuthSlider = lockIfSame(azimuthSlider_, slider)) {
    performSliderAction(p_->getAzimuth(), azimuthSlider, action);
    return;
  }
  if(auto elevationSlider = lockIfSame(elevationSlider_, slider)) {
    performSliderAction(p_->getElevation(), elevationSlider, action);
    return;
  }
  if(auto distanceSlider = lockIfSame(distanceSlider_, slider)) {
    performSliderAction(p_->getDistance(), distanceSlider, action);
    return;
  }
  if(auto sizeSlider = lockIfSame(sizeSlider_, slider)) {
    performSliderAction(p_->getHeight(), sizeSlider, action);
    performSliderAction(p_->getWidth(), sizeSlider, action);
    performSliderAction(p_->getDepth(), sizeSlider, action);
    return;
  }
  if(auto heightSlider = lockIfSame(heightSlider_, slider)) {
    performSliderAction(p_->getHeight(), heightSlider, action);
    return;
  }
  if(auto widthSlider = lockIfSame(widthSlider_, slider)) {
    performSliderAction(p_->getWidth(), widthSlider, action);
    return;
  }
  if(auto depthSlider = lockIfSame(depthSlider_, slider)) {
    performSliderAction(p_->getDepth(), depthSlider, action);
    return;
  }
  if(auto diffuseSlider = lockIfSame(diffuseSlider_, slider)) {
    performSliderAction(p_->getDiffuse(), diffuseSlider, action);
    return;
  }
  if(auto factorSlider = lockIfSame(factorSlider_, slider)) {
    performSliderAction(p_->getFactor(), factorSlider, action);
    return;
  }
  if(auto rangeSlider = lockIfSame(rangeSlider_, slider)) {
    performSliderAction(p_->getRange(), rangeSlider, action);
    return;
  }
}

void ObjectsJuceFrontendConnector::pannerValueChanged(
    ear::plugin::ui::PannerTopView* panner) {
  if (auto pannerTopView = lockIfSame(pannerTopView_, panner)) {
    *(p_->getAzimuth()) = panner->getAzimuth();
    *(p_->getDistance()) = panner->getDistance();
  }
}

void ObjectsJuceFrontendConnector::pannerDragStarted(
    ear::plugin::ui::PannerTopView* panner) {
  if (auto pannerTopView = lockIfSame(pannerTopView_, panner)) {
    p_->getAzimuth()->beginChangeGesture();
    p_->getDistance()->beginChangeGesture();
  }
}

void ObjectsJuceFrontendConnector::pannerDragEnded(
    ear::plugin::ui::PannerTopView* panner) {
  if (auto pannerTopView = lockIfSame(pannerTopView_, panner)) {
    p_->getAzimuth()->endChangeGesture();
    p_->getDistance()->endChangeGesture();
  }
}

void ObjectsJuceFrontendConnector::pannerValueChanged(
    ear::plugin::ui::PannerSideView* panner) {
  if (auto pannerSideView = lockIfSame(pannerSideView_, panner)) {
    *(p_->getElevation()) = panner->getElevation();
  }
}

void ObjectsJuceFrontendConnector::pannerDragStarted(
    ear::plugin::ui::PannerSideView* panner) {
  if (auto pannerSideView = lockIfSame(pannerSideView_, panner)) {
    p_->getElevation()->beginChangeGesture();
  }
}

void ObjectsJuceFrontendConnector::pannerDragEnded(
    ear::plugin::ui::PannerSideView* panner) {
  if (auto pannerSideView = lockIfSame(pannerSideView_, panner)) {
    p_->getElevation()->endChangeGesture();
  }
}

void ObjectsJuceFrontendConnector::comboBoxChanged(EarComboBox* comboBox) {
  if (auto routingComboBox = lockIfSame(routingComboBox_, comboBox)) {
    *(p_->getRouting()) = comboBox->getSelectedEntryIndex();
  }
}

void ObjectsJuceFrontendConnector::buttonClicked(Button* button) {
  if (auto divergenceButton = lockIfSame(divergenceButton_, button)) {
    // note: getToggleState still has the old value when this is called
    *(p_->getDivergence()) = !divergenceButton->getToggleState();
  } else if (auto linkSizeButton = lockIfSame(linkSizeButton_, button)) {
    *(p_->getLinkSize()) = !linkSizeButton->getToggleState();
  }  else if (auto useTrackNameCheckbox = lockIfSame(useTrackNameCheckbox_, button)) {
    *(p_->getUseTrackName()) = !useTrackNameCheckbox->getToggleState();
  }
}

void ObjectsJuceFrontendConnector::textEditorTextChanged(TextEditor& textEditor)
{
  if (auto nameTextEditor = lockIfSame(nameTextEditor_, &textEditor)) {
    setName(nameTextEditor->getText().toStdString());
  }
}

std::string ObjectsJuceFrontendConnector::getActiveName()
{
  return cachedName_;
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
