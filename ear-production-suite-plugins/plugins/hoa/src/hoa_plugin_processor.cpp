#include "hoa_plugin_processor.hpp"

#include "components/non_automatable_parameter.hpp"
#include "hoa_plugin_editor.hpp"
#include "hoa_frontend_connector.hpp"
#include "components/level_meter_calculator.hpp"
#include "reaper_integration.hpp"

using namespace ear::plugin;

HoaAudioProcessor::HoaAudioProcessor()
    : AudioProcessor(
      // 49 channels of input supports the largest HOA order in common definitions.
      /// Use of 49 Discrete Channels avoids REAPER applying potentially incorrect labels to the input channels.
      /// Better to show just "Input #" rather than something potentially incorrect.
      // The omission of an output bus is also intentional.
      /// We do not actually manipulate audio here - only analyse it for level.
      BusesProperties().withInput("Input", AudioChannelSet::discreteChannels(49), true)),
      samplerate_(48000),
      levelMeterCalculator_(
          std::make_shared<LevelMeterCalculator>(49, samplerate_)) {
  /* clang-format off */
  addParameter(routing_ =
    new ui::NonAutomatedParameter<AudioParameterInt>(
      "routing", "Routing",
      -1, MAX_DAW_CHANNELS-1, -1));
  addParameter(packFormatIdValue_ =
    new ui::NonAutomatedParameter<AudioParameterInt>(
      "packformat_id_value", "PackFormat ID Value",
      0, 0xFFFF, 0));
  addParameter(bypass_ =
    new AudioParameterBool("byps", "Bypass", false));
  addParameter(useTrackName_ =
    new AudioParameterBool("useTrackName", "Use Track Name", true));
  addParameter(inputInstanceId_ = new ReadOnlyAudioParameterInt(
    "inputInstanceId",  // parameter ID
    "Auto-set ID to uniquely identify plugin",  // parameter name
    0, 65535, 0));  // range and default value
  /* clang-format on */

  static_cast<ui::NonAutomatedParameter<AudioParameterInt>*>(routing_)
      ->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  // Any NonAutomatedParameter will not set the plugin state as dirty when
  // changed. We used this hack to make sure the host (REAPER) knows something
  // has changed (by "changing" a standard parameter to it's current value)
  static_cast<ui::NonAutomatedParameter<AudioParameterInt>*>(packFormatIdValue_)
      ->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  connector_ = std::make_unique<ui::HoaJuceFrontendConnector>(this);
  backend_ = std::make_unique<HoaBackend>(connector_.get());

  connector_->parameterValueChanged(0, routing_->get());
  connector_->parameterValueChanged(1, packFormatIdValue_->get());
  connector_->parameterValueChanged(3, useTrackName_->get());
}

HoaAudioProcessor::~HoaAudioProcessor() {}

const String HoaAudioProcessor::getName() const { return JucePlugin_Name; }

bool HoaAudioProcessor::acceptsMidi() const { return false; }
bool HoaAudioProcessor::producesMidi() const { return false; }
bool HoaAudioProcessor::isMidiEffect() const { return false; }

double HoaAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int HoaAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int HoaAudioProcessor::getCurrentProgram() { return 0; }
void HoaAudioProcessor::setCurrentProgram(int index) {}

const String HoaAudioProcessor::getProgramName(int index) { return {}; }
void HoaAudioProcessor::changeProgramName(int index, const String& newName) {}

void HoaAudioProcessor::prepareToPlay(double samplerate, int samplesPerBlock) {
  if (samplerate_ != static_cast<int>(samplerate)) {
    samplerate_ = static_cast<int>(samplerate);
    levelMeterCalculator_->setup(49, samplerate_);
  }
}

void HoaAudioProcessor::releaseResources() {}

bool HoaAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {

  // Must accept default config specified in ctor

  if(layouts.inputBuses.size() != 1)
    return false;
  if(layouts.inputBuses[0] != AudioChannelSet::discreteChannels(49))
    return false;

  if(layouts.outputBuses.size() != 0)
    return false;

  return true;
}

void HoaAudioProcessor::processBlock(AudioBuffer<float>& buffer,
                                     MidiBuffer& midiMessages) {
  if(!bypass_->get()) {
    if(getActiveEditor()) {
      levelMeterCalculator_->process(buffer);
    } else {
      levelMeterCalculator_->processForClippingOnly(buffer);
    }
    backend_->triggerMetadataSend();
  }
}

bool HoaAudioProcessor::hasEditor() const { return true; }

AudioProcessorEditor* HoaAudioProcessor::createEditor() {
  levelMeterCalculator_->resetLevels();
  return new HoaAudioProcessorEditor(this);
}

void HoaAudioProcessor::getStateInformation(MemoryBlock& destData) {
  std::unique_ptr<XmlElement> xml(new XmlElement("HoaPlugin"));
  connectionId_ = backend_->getConnectionId();
  xml->setAttribute("connection_id", connectionId_.string());
  xml->setAttribute("routing", (int)*routing_);
  xml->setAttribute("packformat_id_value", (int)*packFormatIdValue_);
  xml->setAttribute("use_track_name", (bool)*useTrackName_);
  xml->setAttribute("name", connector_->getActiveName());
  copyXmlToBinary(*xml, destData);
}

void HoaAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
  std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
  if(xmlState) setStateInformation(xmlState.get());
}

void HoaAudioProcessor::setStateInformation(XmlElement * xmlState, bool useDefaultsIfUnspecified)
{
  if (xmlState->hasTagName("HoaPlugin")) {
    if(useDefaultsIfUnspecified || xmlState->hasAttribute("connection_id")) {
      connectionId_ = communication::ConnectionId{
          xmlState
              ->getStringAttribute("connection_id",
                                    "00000000-0000-0000-0000-000000000000")
              .toStdString() };
      backend_->setConnectionId(connectionId_);
    }

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("routing")) {
      *routing_ = xmlState->getIntAttribute("routing", -1);
    }

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("packformat_id_value")) {
      *packFormatIdValue_ = xmlState->getIntAttribute("packformat_id_value", 0);
    }

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("use_track_name")) {
      *useTrackName_ = xmlState->getBoolAttribute("use_track_name", true);
    }

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("name")) {
      connector_->setName(xmlState->getStringAttribute("name", "No Name").toStdString());
    }
  }
}

void HoaAudioProcessor::updateTrackProperties(
    const TrackProperties& properties) {
  connector_->trackPropertiesChanged(properties);
}

void HoaAudioProcessor::setIHostApplication(Steinberg::FUnknown * unknown)
{
  reaperHost = dynamic_cast<IReaperHostApplication*>(unknown);
  VST3ClientExtensions::setIHostApplication(unknown);
  if(reaperHost) {

    inputInstanceId_->internalSetIntAndNotifyHost(
        requestInstanceIdFromExtension(reaperHost));

    registerPluginLoadWithExtension(reaperHost,
                                    [this](std::string const& xmlState) {
                                      this->extensionSetState(xmlState);
                                    });

    numDawChannels_ = DetermineChannelCount(reaperHost);
  }
}

void HoaAudioProcessor::extensionSetState(std::string const & xmlStateStr)
{
  auto doc = XmlDocument(xmlStateStr);
  auto xmlState = doc.getDocumentElement();
  setStateInformation(xmlState.get(), false);
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new HoaAudioProcessor();
}
