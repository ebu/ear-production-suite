#include "hoa_frontend_connector.hpp"

// TODO - remove unrequired components once UI dev complete
#include "components/ear_combo_box.hpp"
#include "components/ear_name_text_editor.hpp"

namespace ear {
namespace plugin {
namespace ui {

inline bool clipToBool(float value) { return value < 0.5 ? false : true; }

HoaJuceFrontendConnector::HoaJuceFrontendConnector(
    HoaAudioProcessor* processor)
    : HoaFrontendBackendConnector(), p_(processor) {
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

HoaJuceFrontendConnector::~HoaJuceFrontendConnector() {
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

  /* Old DS code
  // We must remove any listeners we have assigned when this class is destroyed!
  if (auto comboBox = speakerSetupsComboBox_.lock()) {
    comboBox->removeListener(this);
  }
  */
}

void HoaJuceFrontendConnector::setStatusBarLabel(
    std::shared_ptr<Label> statusBarLabel) {
  statusBarLabel_ = statusBarLabel;
  setStatusBarText(cachedStatusBarText_);
}

void HoaJuceFrontendConnector::doSetStatusBarText(
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

void HoaJuceFrontendConnector::setNameTextEditor(
    std::shared_ptr<EarNameTextEditor> textEditor) {
  nameTextEditor_ = textEditor;
  setName(cachedName_);
}

void HoaJuceFrontendConnector::setName(const std::string& name) {
  if (auto nameTextEditorLocked = nameTextEditor_.lock()) {
    nameTextEditorLocked->setText(name);
  }
  cachedName_ = name;
}

void HoaJuceFrontendConnector::setColourComboBox(
    std::shared_ptr<EarComboBox> comboBox) {
  comboBox->addListener(this);
  colourComboBox_ = comboBox;
  setColour(cachedColour_);
}

void HoaJuceFrontendConnector::setColour(Colour colour) {
  if (auto colourComboBoxLocked = colourComboBox_.lock()) {
    colourComboBoxLocked->clearEntries();
    colourComboBoxLocked->addColourEntry(colour);
    colourComboBoxLocked->selectEntry(0, dontSendNotification);
  }
  /* Old DS Code
  // When the track colour changes, we need to inform the component panels so they can change their highlight colours accordingly
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
  */
  cachedColour_ = colour;
}

void HoaJuceFrontendConnector::setRoutingComboBox(
    std::shared_ptr<EarComboBox> comboBox) {
  comboBox->addListener(this);
  routingComboBox_ = comboBox;
  setRouting(cachedRouting_);
}

void HoaJuceFrontendConnector::setRouting(int routing) {
  if (auto routingComboBoxLocked = routingComboBox_.lock()) {
    routingComboBoxLocked->selectEntry(routing, dontSendNotification);
  }
  cachedRouting_ = routing;
}
//ME new added methods for adding common definition
void HoaJuceFrontendConnector::setCommonDefinitionComboBox(
    std::shared_ptr<EarComboBox> comboBox) {
  comboBox->addListener(this);
  commonDefinitionComboBox_ = comboBox;
  setCommonDefinition(cachedCommonDefinition_);
}
void HoaJuceFrontendConnector::setCommonDefinition(int commonDefinition) {
  if (auto routingComboBoxLocked = commonDefinitionComboBox_.lock()) {
    routingComboBoxLocked->selectEntry(commonDefinition, dontSendNotification);
  }
  cachedCommonDefinition_ = commonDefinition;
}
//end ME

/* Old DS Code - see header for info
// This code also sets the entries in the "Routing" combo-box based on the number of channels required.
// We will need to do something similar when the user selects a HOA type

void HoaJuceFrontendConnector::setSpeakerSetupsComboBox(
    std::shared_ptr<EarComboBox> comboBox) {
  comboBox->addListener(this);
  speakerSetupsComboBox_ = comboBox;
  setSpeakerSetup(cachedSpeakerSetupIndex_);
}

void HoaJuceFrontendConnector::setSpeakerSetup(
    int speakerSetupIndex) {
  cachedSpeakerSetupIndex_ = speakerSetupIndex;
  if (auto speakerSetupsComboBoxLocked = speakerSetupsComboBox_.lock())
    speakerSetupsComboBoxLocked->selectEntry(speakerSetupIndex,
                                             dontSendNotification);
  if (auto routingComboBoxLocked = routingComboBox_.lock()) {
    int layoutSize =
        speakerSetupByIndex(cachedSpeakerSetupIndex_).speakers.size();
    routingComboBoxLocked->clearEntries();
    auto layoutSizeFixed = layoutSize != 0 ? layoutSize - 1 : layoutSize;
    for (int i = 1; i + layoutSizeFixed <= 64; ++i) {
      routingComboBoxLocked->addTextEntry(String(i) + String::fromUTF8("–") +
                                          String(i + layoutSizeFixed));
    }
    routingComboBoxLocked->selectEntry(cachedRouting_, sendNotification);
  }
  auto speakerSetup = speakerSetupByIndex(speakerSetupIndex);
  if (auto upperLayerLocked = upperLayer_.lock()) {
    upperLayerLocked->setSpeakerSetup(speakerSetup);
  }
  if (auto middleLayerLocked = middleLayer_.lock()) {
    middleLayerLocked->setSpeakerSetup(speakerSetup);
  }
  if (auto bottomLayerLocked = bottomLayer_.lock()) {
    bottomLayerLocked->setSpeakerSetup(speakerSetup);
  }
  if (auto channelGainsLocked = channelGains_.lock()) {
    channelGainsLocked->setSpeakerSetup(speakerSetup);
  }
}

void HoaJuceFrontendConnector::setUpperLayerValueBox(
    std::shared_ptr<ValueBoxSpeakerLayer> layer) {
  layer->setHighlightColour(cachedColour_);
  layer->setSpeakerSetup(speakerSetupByIndex(cachedSpeakerSetupIndex_));
  upperLayer_ = layer;
}
void HoaJuceFrontendConnector::setMiddleLayerValueBox(
    std::shared_ptr<ValueBoxSpeakerLayer> layer) {
  layer->setHighlightColour(cachedColour_);
  layer->setSpeakerSetup(speakerSetupByIndex(cachedSpeakerSetupIndex_));
  middleLayer_ = layer;
}
void HoaJuceFrontendConnector::setBottomLayerValueBox(
    std::shared_ptr<ValueBoxSpeakerLayer> layer) {
  layer->setHighlightColour(cachedColour_);
  layer->setSpeakerSetup(speakerSetupByIndex(cachedSpeakerSetupIndex_));
  bottomLayer_ = layer;
}
void HoaJuceFrontendConnector::setChannelGainsValueBox(
    std::shared_ptr<ValueBoxChannelGain> gains) {
  gains->setSpeakerSetup(speakerSetupByIndex(cachedSpeakerSetupIndex_));
  channelGains_ = gains;
}
*/

void HoaJuceFrontendConnector::parameterValueChanged(
    int parameterIndex, float newValue) {
  using ParameterId = ui::HoaFrontendBackendConnector::ParameterId;
  switch (parameterIndex) {
    case 0:
      notifyParameterChanged(ParameterId::ROUTING, p_->getRouting()->get());
      updater_.callOnMessageThread([this]() {
        setRouting(p_->getRouting()->get());
      });
      break;
    /* Old DS Code
    // Other parameters should be added here
    case 1:
      notifyParameterChanged(ParameterId::SPEAKER_SETUP_INDEX,
                             p_->getSpeakerSetupIndex()->get());
      updater_.callOnMessageThread([this]() {
        setSpeakerSetup(p_->getSpeakerSetupIndex()->get());
      });
      break;
    */
  }
}

void HoaJuceFrontendConnector::trackPropertiesChanged(
    const juce::AudioProcessor::TrackProperties& properties) {
  // It is unclear if this will be called from the message thread so to be sure
  // we call the following async using the MessageManager
  updater_.callOnMessageThread([properties, this]() {
    this->setName(properties.name.toStdString());
    notifyParameterChanged(ParameterId::NAME, properties.name.toStdString());
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

/* Old DS Code
// Listener code - see header for info
void HoaJuceFrontendConnector::sliderValueChanged(Slider* slider) {}
void HoaJuceFrontendConnector::sliderDragStarted(Slider* slider) {}
void HoaJuceFrontendConnector::sliderDragEnded(Slider* slider) {}
*/

void HoaJuceFrontendConnector::comboBoxChanged(
    EarComboBox* comboBox) {
  /* Old DS Code
  // Probably need similar when selecting HOOA type
  if (!speakerSetupsComboBox_.expired() &&
      comboBox == speakerSetupsComboBox_.lock().get()) {
    *(p_->getSpeakerSetupIndex()) = comboBox->getSelectedEntryIndex();
  }
  */
  if (!routingComboBox_.expired() &&
      comboBox == routingComboBox_.lock().get()) {
    *(p_->getRouting()) = comboBox->getSelectedEntryIndex();
  }
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
