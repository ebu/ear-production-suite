#include "direct_speakers_frontend_connector.hpp"

#include "components/ear_combo_box.hpp"
#include "components/ear_name_text_editor.hpp"
#include "detail/weak_ptr_helpers.hpp"
#include <helper/adm_preset_definitions_helper.h>
#include "speaker_setups.hpp"
#include <daw_channel_count.h>

namespace {
  String routingLayoutDescriptionAt(int position, int layoutSizeFixed) {
    String routingDescription = String(position);
    if(layoutSizeFixed > 0) {
      routingDescription += String::fromUTF8("-") + String(position + layoutSizeFixed);
    }
    return routingDescription;
  }
}

namespace ear {
namespace plugin {
namespace ui {

using detail::lockIfSame;

inline bool clipToBool(float value) { return value < 0.5 ? false : true; }

DirectSpeakersJuceFrontendConnector::DirectSpeakersJuceFrontendConnector(
    DirectSpeakersAudioProcessor* processor)
    : DirectSpeakersFrontendBackendConnector(), p_(processor) {
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

DirectSpeakersJuceFrontendConnector::~DirectSpeakersJuceFrontendConnector() {
  for (auto parameter : parameters_) {
    if (parameter.second) {
      parameter.second->removeListener(this);
    }
  }
  if (auto comboBox = colourComboBox_.lock()) {
    comboBox->removeListener(this);
  }
  if (auto comboBox = routingComboBox_.lock()) {
    comboBox->removeListener(this);
  }
  if (auto comboBox = speakerSetupsComboBox_.lock()) {
    comboBox->removeListener(this);
  }
  if (auto button = useTrackNameCheckbox_.lock()) {
    button->removeListener(this);
  }
}

void DirectSpeakersJuceFrontendConnector::setStatusBarLabel(
    std::shared_ptr<Label> statusBarLabel) {
  statusBarLabel_ = statusBarLabel;
  setStatusBarText(cachedStatusBarText_);
}

void DirectSpeakersJuceFrontendConnector::doSetStatusBarText(
    const std::string& text) {
  if (!statusBarLabel_.expired()) {
    auto statusBar = statusBarLabel_;

    updater_.callOnMessageThread([text, statusBar]() {
      if (auto statusBarLocked = statusBar.lock()) {
        statusBarLocked->setText(text, dontSendNotification);
      }
    });
  }
  cachedStatusBarText_ = text;
}

void DirectSpeakersJuceFrontendConnector::setNameTextEditor(
    std::shared_ptr<EarNameTextEditor> textEditor) {
  textEditor->addListener(this);
  nameTextEditor_ = textEditor;
  updateNameState();
}

void DirectSpeakersJuceFrontendConnector::setUseTrackNameCheckbox(
  std::shared_ptr<ToggleButton> useTrackNameCheckbox){
  useTrackNameCheckbox->addListener(this);
  useTrackNameCheckbox_ = useTrackNameCheckbox;
  setUseTrackName(cachedUseTrackName_);
}

void DirectSpeakersJuceFrontendConnector::setName(const std::string& name) {
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

void DirectSpeakersJuceFrontendConnector::setUseTrackName(bool useTrackName)
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

void DirectSpeakersJuceFrontendConnector::setColourComboBox(
    std::shared_ptr<EarComboBox> comboBox) {
  comboBox->addListener(this);
  colourComboBox_ = comboBox;
  setColour(cachedColour_);
}

void DirectSpeakersJuceFrontendConnector::setColour(Colour colour) {
  if (auto colourComboBoxLocked = colourComboBox_.lock()) {
    colourComboBoxLocked->clearEntries();
    colourComboBoxLocked->addColourEntry(colour);
    colourComboBoxLocked->selectEntry(0, dontSendNotification);
  }
  if (auto upperLayerLocked = upperLayer_.lock()) {
    upperLayerLocked->setHighlightColour(colour);
    upperLayerLocked->repaint();
  }
  if (auto middleLayerLocked = middleLayer_.lock()) {
    middleLayerLocked->setHighlightColour(colour);
    middleLayerLocked->repaint();
  }
  if (auto bottomLayerLocked = bottomLayer_.lock()) {
    bottomLayerLocked->setHighlightColour(colour);
    bottomLayerLocked->repaint();
  }
  cachedColour_ = colour;
}

void DirectSpeakersJuceFrontendConnector::setRoutingComboBox(
    std::shared_ptr<EarComboBox> comboBox) {
  comboBox->addListener(this);
  routingComboBox_ = comboBox;
  setRouting(cachedRouting_);
}

void DirectSpeakersJuceFrontendConnector::setRouting(int routing) {
  if (auto routingComboBoxLocked = routingComboBox_.lock()) {
    routingComboBoxLocked->selectEntry(routing, dontSendNotification);
  }
  cachedRouting_ = routing;
}

void DirectSpeakersJuceFrontendConnector::setSpeakerSetupsComboBox(
    std::shared_ptr<EarComboBox> comboBox) {
  comboBox->addListener(this);
  speakerSetupsComboBox_ = comboBox;
  setSpeakerSetupPackFormat(cachedPackFormatId_);
}

void DirectSpeakersJuceFrontendConnector::setSpeakerSetupPackFormat(
    int speakerSetupPackFormatIdValue) {
  cachedPackFormatId_ = speakerSetupPackFormatIdValue;
  if (auto speakerSetupsComboBoxLocked = speakerSetupsComboBox_.lock())
    speakerSetupsComboBoxLocked->setSelectedId(speakerSetupPackFormatIdValue,
                                             dontSendNotification);
  auto pfData = AdmPresetDefinitionsHelper::getSingleton()->getPackFormatData(
      1, cachedPackFormatId_);
  if (auto routingComboBoxLocked = routingComboBox_.lock()) {
    int layoutSize = pfData ? pfData->relatedChannelFormats.size() : 0;
    routingComboBoxLocked->clearEntries();
    auto layoutSizeFixed = layoutSize != 0 ? layoutSize - 1 : layoutSize;
    for (int i = 1; i + layoutSizeFixed <= MAX_DAW_CHANNELS; ++i) {
      auto entry = routingComboBoxLocked->addTextEntry(routingLayoutDescriptionAt(i, layoutSizeFixed), i);
      entry->setSelectable(i + layoutSizeFixed <= p_->getNumDawChannels());
    }
    routingComboBoxLocked->selectEntry(cachedRouting_, sendNotification);
  }
  if (auto upperLayerLocked = upperLayer_.lock()) {
    upperLayerLocked->setSpeakerSetupPackFormat(cachedPackFormatId_);
  }
  if (auto middleLayerLocked = middleLayer_.lock()) {
    middleLayerLocked->setSpeakerSetupPackFormat(cachedPackFormatId_);
  }
  if (auto bottomLayerLocked = bottomLayer_.lock()) {
    bottomLayerLocked->setSpeakerSetupPackFormat(cachedPackFormatId_);
  }
  if (auto channelGainsLocked = channelGains_.lock()) {
    channelGainsLocked->setSpeakerSetupPackFormat(cachedPackFormatId_);
  }
}

void DirectSpeakersJuceFrontendConnector::setUpperLayerValueBox(
    std::shared_ptr<ValueBoxSpeakerLayer> layer) {
  layer->setHighlightColour(cachedColour_);
  layer->setSpeakerSetupPackFormat(cachedPackFormatId_);
  upperLayer_ = layer;
}
void DirectSpeakersJuceFrontendConnector::setMiddleLayerValueBox(
    std::shared_ptr<ValueBoxSpeakerLayer> layer) {
  layer->setHighlightColour(cachedColour_);
  layer->setSpeakerSetupPackFormat(cachedPackFormatId_);
  middleLayer_ = layer;
}
void DirectSpeakersJuceFrontendConnector::setBottomLayerValueBox(
    std::shared_ptr<ValueBoxSpeakerLayer> layer) {
  layer->setHighlightColour(cachedColour_);
  layer->setSpeakerSetupPackFormat(cachedPackFormatId_);
  bottomLayer_ = layer;
}
void DirectSpeakersJuceFrontendConnector::setChannelGainsValueBox(
    std::shared_ptr<ChannelMeterLayout> gains) {
  gains->setSpeakerSetupPackFormat(cachedPackFormatId_);
  channelGains_ = gains;
}

void DirectSpeakersJuceFrontendConnector::parameterValueChanged(
    int parameterIndex, float newValue) {
  using ParameterId = ui::DirectSpeakersFrontendBackendConnector::ParameterId;
  switch (parameterIndex) {
    case 0:
      notifyParameterChanged(ParameterId::ROUTING, p_->getRouting()->get());
      updater_.callOnMessageThread([this]() {
        setRouting(p_->getRouting()->get());
      });
      break;
    case 1:
      notifyParameterChanged(ParameterId::PACKFORMAT_ID_VALUE,
                             p_->getPackFormatIdValue()->get());
      updater_.callOnMessageThread([this]() {
        setSpeakerSetupPackFormat(p_->getPackFormatIdValue()->get());
      });
      break;
    case 3:
      setUseTrackName(p_->getUseTrackName()->get());
      break;
    case 4:
      notifyParameterChanged(ParameterId::INPUT_INSTANCE_ID, p_->getInputInstanceId()->get());
      break;
  }
}

void DirectSpeakersJuceFrontendConnector::trackPropertiesChanged(
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

void DirectSpeakersJuceFrontendConnector::sliderValueChanged(Slider* slider) {}
void DirectSpeakersJuceFrontendConnector::sliderDragStarted(Slider* slider) {}
void DirectSpeakersJuceFrontendConnector::sliderDragEnded(Slider* slider) {}

void DirectSpeakersJuceFrontendConnector::comboBoxChanged(
    EarComboBox* comboBox) {
  if (auto speakerSetupsComboBox = lockIfSame(speakerSetupsComboBox_, comboBox)) {
    *(p_->getPackFormatIdValue()) = speakerSetupsComboBox->getSelectedId();
  }
  if (auto routingComboBox = lockIfSame(routingComboBox_, comboBox)) {
    *(p_->getRouting()) = routingComboBox->getSelectedEntryIndex();
  }
}

void DirectSpeakersJuceFrontendConnector::buttonClicked(Button* button) {
  if (auto useTrackNameCheckbox = lockIfSame(useTrackNameCheckbox_, button)) {
    *(p_->getUseTrackName()) = !useTrackNameCheckbox->getToggleState();
  }
}

void DirectSpeakersJuceFrontendConnector::textEditorTextChanged(TextEditor& textEditor)
{
  if (auto nameTextEditor = lockIfSame(nameTextEditor_, &textEditor)) {
    setName(nameTextEditor->getText().toStdString());
  }
}

void DirectSpeakersJuceFrontendConnector::updateNameState()
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

std::string DirectSpeakersJuceFrontendConnector::getActiveName()
{
  return cachedName_;
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
