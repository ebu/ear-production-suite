#include "hoa_frontend_connector.hpp"

#include "value_box_order_display.hpp"
#include "components/ear_name_text_editor.hpp"
#include "detail/weak_ptr_helpers.hpp"
#include <helper/adm_preset_definitions_helper.h>

#include <daw_channel_count.h>

namespace {
String routingLayoutDescriptionAt(int position, int layoutSizeFixed) {
  return String(position) + String::fromUTF8("-") +
         String(position + layoutSizeFixed);
}
}  // namespace

namespace ear {
namespace plugin {
namespace ui {

using detail::lockIfSame;

inline bool clipToBool(float value) { return value < 0.5 ? false : true; }

HoaJuceFrontendConnector::HoaJuceFrontendConnector(HoaAudioProcessor* processor)
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
  if (auto comboBox = hoaTypeComboBox_.lock()) {
    comboBox->removeListener(this);
  }
  if (auto button = useTrackNameCheckbox_.lock()) {
    button->removeListener(this);
  }
}

void HoaJuceFrontendConnector::setStatusBarLabel(
    std::shared_ptr<Label> statusBarLabel) {
  statusBarLabel_ = statusBarLabel;
  setStatusBarText(cachedStatusBarText_);
}

void HoaJuceFrontendConnector::doSetStatusBarText(const std::string& text) {
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
  textEditor->addListener(this);
  nameTextEditor_ = textEditor;
  updateNameState();
}

void HoaJuceFrontendConnector::setUseTrackNameCheckbox(
  std::shared_ptr<ToggleButton> useTrackNameCheckbox){
  useTrackNameCheckbox->addListener(this);
  useTrackNameCheckbox_ = useTrackNameCheckbox;
  setUseTrackName(cachedUseTrackName_);
}

void HoaJuceFrontendConnector::setName(const std::string& name) {
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

void HoaJuceFrontendConnector::setUseTrackName(bool useTrackName)
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

void HoaJuceFrontendConnector::setHoaTypeComboBox(
    std::shared_ptr<EarComboBox> comboBox) {
  comboBox->addListener(this);
  hoaTypeComboBox_ = comboBox;
  setHoaType(cachedHoaType_);
}

void HoaJuceFrontendConnector::setHoaType(int hoaType) {
  if (auto hoaTypeComboBoxLocked = hoaTypeComboBox_.lock()) {
    hoaTypeComboBoxLocked->setSelectedId(hoaType, dontSendNotification);
  }
  cachedHoaType_ = hoaType;

  auto packFormatIdValue = p_->getPackFormatIdValue()->get();
  if (auto routingComboBoxLocked = routingComboBox_.lock()) {
    auto pfData = AdmPresetDefinitionsHelper::getSingleton().getPackFormatData(
        4, packFormatIdValue);
    size_t cfCount{0};

    if (pfData) {
      cfCount = pfData->relatedChannelFormats.size();
    }

    routingComboBoxLocked->clearEntries();
    auto cfCountFixed = cfCount != 0 ? cfCount - 1 : cfCount;
    for (int i = 1; i + cfCountFixed <= MAX_DAW_CHANNELS; ++i) {
      auto entry = routingComboBoxLocked->addTextEntry(
          routingLayoutDescriptionAt(i, cfCountFixed), i);
      entry->setSelectable(i + cfCountFixed <= p_->getNumDawChannels());
    }
    routingComboBoxLocked->selectEntry(cachedRouting_, sendNotification);
  }

  if (auto channelGainsLocked = orderDisplay_.lock()) {
    channelGainsLocked->setHoaType(packFormatIdValue);
  }
}

void HoaJuceFrontendConnector::setOrderDisplayValueBox(
    std::shared_ptr<ValueBoxOrderDisplay> orderDisplay) {
  orderDisplay->setHoaType(cachedHoaType_);
  orderDisplay_ = orderDisplay;
}

void HoaJuceFrontendConnector::parameterValueChanged(int parameterIndex,
                                                     float newValue) {
  using ParameterId = ui::HoaFrontendBackendConnector::ParameterId;
  switch (parameterIndex) {
    case 0:
      notifyParameterChanged(ParameterId::ROUTING, p_->getRouting()->get());
      updater_.callOnMessageThread(
          [this]() { setRouting(p_->getRouting()->get()); });
      break;
    case 1:
      notifyParameterChanged(ParameterId::PACKFORMAT_ID_VALUE,
                             p_->getPackFormatIdValue()->get());  //(2.)
      updater_.callOnMessageThread(
          [this]() { setHoaType(p_->getPackFormatIdValue()->get()); });
      break;
    case 3:
      setUseTrackName(p_->getUseTrackName()->get());
      break;
    case 4:
      notifyParameterChanged(ParameterId::INPUT_INSTANCE_ID, p_->getInputInstanceId()->get());
      break;
  }
}

void HoaJuceFrontendConnector::trackPropertiesChanged(
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

void HoaJuceFrontendConnector::comboBoxChanged(EarComboBox* comboBox) {
  if(auto hoaTypeComboBox = lockIfSame(hoaTypeComboBox_, comboBox)) {
    if(hoaTypeComboBox->hasSelection()) {
      juce::var id = hoaTypeComboBox->getSelectedId();
      *(p_->getPackFormatIdValue()) = static_cast<int>(id);
    } else {
      *(p_->getPackFormatIdValue()) = 0;
    }
  }
  if (auto routingComboBox = lockIfSame(routingComboBox_, comboBox)) {
    *(p_->getRouting()) = routingComboBox->getSelectedEntryIndex();
  }
}

void HoaJuceFrontendConnector::buttonClicked(Button* button) {
  if (auto useTrackNameCheckbox = lockIfSame(useTrackNameCheckbox_, button)) {
    *(p_->getUseTrackName()) = !useTrackNameCheckbox->getToggleState();
  }
}

void HoaJuceFrontendConnector::textEditorTextChanged(TextEditor& textEditor)
{
  if (auto nameTextEditor = lockIfSame(nameTextEditor_, &textEditor)) {
    setName(nameTextEditor->getText().toStdString());
  }
}

void HoaJuceFrontendConnector::updateNameState()
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

std::string HoaJuceFrontendConnector::getActiveName()
{
  return cachedName_;
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
