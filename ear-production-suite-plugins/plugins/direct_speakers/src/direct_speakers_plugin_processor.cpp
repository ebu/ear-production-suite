#include "direct_speakers_plugin_processor.hpp"
#include "speaker_setups.hpp"

#include "components/non_automatable_parameter.hpp"
#include "direct_speakers_plugin_editor.hpp"
#include "direct_speakers_frontend_connector.hpp"
#include "reaper_integration.hpp"

using namespace ear::plugin;

DirectSpeakersAudioProcessor::DirectSpeakersAudioProcessor()
    : AudioProcessor(
      // 24 channels of input supports the largest DS layout in common definitions.
      /// Use of 24 Discrete Channels avoids REAPER applying potentially incorrect labels to the input channels.
      /// Better to show just "Input #" rather than something potentially incorrect.
      // The omission of an output bus is also intentional.
      /// We do not actually manipulate audio here - only analyse it for level.
      BusesProperties().withInput("Input", AudioChannelSet::discreteChannels(24), true)),
      samplerate_(48000),
      levelMeter_(std::make_shared<LevelMeterCalculator>(24, samplerate_)) {
  /* clang-format off */
  addParameter(routing_ =
    new ui::NonAutomatedParameter<AudioParameterInt>(
      "routing", "Routing",
      -1, MAX_DAW_CHANNELS-1, -1));
  addParameter(packFormatIdValue_ =
    new ui::NonAutomatedParameter<AudioParameterInt>(
      "packFormatIdValue", "PackFormat ID Value",
      0x0, 0xFFFF, 0));
  addParameter(bypass_ =
    new AudioParameterBool("byps", "Bypass", false));
  addParameter(useTrackName_ =
    new AudioParameterBool("useTrackName", "Use Track Name", true));
  addParameter(inputInstanceId_ = new ReadOnlyAudioParameterInt(
    "inputInstanceId",  // parameter ID
    "Auto-set ID to uniquely identify plugin",  // parameter name
    0, 65535, 0));  // range and default value
  /* clang-format on */

  static_cast<ui::NonAutomatedParameter<AudioParameterInt>*>(routing_)->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  static_cast<ui::NonAutomatedParameter<AudioParameterInt>*>(packFormatIdValue_)->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  connector_ = std::make_unique<ui::DirectSpeakersJuceFrontendConnector>(this);
  backend_ = std::make_unique<DirectSpeakersBackend>(connector_.get());

  connector_->parameterValueChanged(0, routing_->get());
  connector_->parameterValueChanged(1, packFormatIdValue_->get());
  connector_->parameterValueChanged(3, useTrackName_->get());
}

DirectSpeakersAudioProcessor::~DirectSpeakersAudioProcessor() {}

const String DirectSpeakersAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool DirectSpeakersAudioProcessor::acceptsMidi() const { return false; }
bool DirectSpeakersAudioProcessor::producesMidi() const { return false; }
bool DirectSpeakersAudioProcessor::isMidiEffect() const { return false; }

double DirectSpeakersAudioProcessor::getTailLengthSeconds() const {
  return 0.0;
}

int DirectSpeakersAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int DirectSpeakersAudioProcessor::getCurrentProgram() { return 0; }
void DirectSpeakersAudioProcessor::setCurrentProgram(int index) {}

const String DirectSpeakersAudioProcessor::getProgramName(int index) {
  return {};
}
void DirectSpeakersAudioProcessor::changeProgramName(int index,
                                                     const String& newName) {}

void DirectSpeakersAudioProcessor::prepareToPlay(double samplerate,
                                                 int samplesPerBlock) {
  if (samplerate_ != static_cast<int>(samplerate)) {
    samplerate_ = static_cast<int>(samplerate);
    levelMeter_->setup(24, samplerate_);
  }
}

void DirectSpeakersAudioProcessor::releaseResources() {}

bool DirectSpeakersAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {

  // Must accept default config specified in ctor

  if(layouts.inputBuses.size() != 1)
    return false;
  if(layouts.inputBuses[0] != AudioChannelSet::discreteChannels(24))
    return false;

  if(layouts.outputBuses.size() != 0)
    return false;

  return true;
}

void DirectSpeakersAudioProcessor::processBlock(AudioBuffer<float>& buffer,
                                                MidiBuffer& midiMessages) {
  if(!bypass_->get()) {
    if(getActiveEditor()) {
      levelMeter_->process(buffer);
    }
    backend_->triggerMetadataSend();
  }
}

bool DirectSpeakersAudioProcessor::hasEditor() const { return true; }

AudioProcessorEditor* DirectSpeakersAudioProcessor::createEditor() {
  levelMeter_->resetLevels();
  return new DirectSpeakersAudioProcessorEditor(this);
}

void DirectSpeakersAudioProcessor::getStateInformation(MemoryBlock& destData) {
  std::unique_ptr<XmlElement> xml(new XmlElement("DirectSpeakersPlugin"));
  connectionId_ = backend_->getConnectionId();
  xml->setAttribute("connection_id", connectionId_.string());
  xml->setAttribute("routing", (int)*routing_);
  xml->setAttribute("packformat_id_value", (int)*packFormatIdValue_);
  xml->setAttribute("use_track_name", (bool)*useTrackName_);
  xml->setAttribute("name", connector_->getActiveName());
  copyXmlToBinary(*xml, destData);
}

void DirectSpeakersAudioProcessor::setStateInformation(const void* data,
                                                       int sizeInBytes) {
  std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
  if(xmlState) setStateInformation(xmlState.get());

}

void DirectSpeakersAudioProcessor::setStateInformation(XmlElement * xmlState, bool useDefaultsIfUnspecified)
{
    if (xmlState->hasTagName("DirectSpeakersPlugin")) {

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

      if(xmlState->hasAttribute("packformat_id_value")) {
        *packFormatIdValue_ = xmlState->getIntAttribute("packformat_id_value", 0);
      }
      else if(xmlState->hasAttribute("speaker_setup_index")) {
        int legacySpeakerSetupIndex = xmlState->getIntAttribute("speaker_setup_index", -1);
        int newSpeakerSetupIndex = getIndexFromLegacySpeakerSetupIndex(legacySpeakerSetupIndex);
        if(newSpeakerSetupIndex >= 0) {
          auto speakerSetup = speakerSetupByIndex(newSpeakerSetupIndex);
          *packFormatIdValue_ = speakerSetup.packFormatIdValue;
        } else {
          *packFormatIdValue_ = 0;
        }
      }

      if(useDefaultsIfUnspecified || xmlState->hasAttribute("use_track_name")) {
        *useTrackName_ = xmlState->getBoolAttribute("use_track_name", true);
      }

      if(useDefaultsIfUnspecified || xmlState->hasAttribute("name")) {
        connector_->setName(xmlState->getStringAttribute("name", "No Name").toStdString());
      }
    }
}

void DirectSpeakersAudioProcessor::updateTrackProperties(
    const TrackProperties& properties) {
  connector_->trackPropertiesChanged(properties);
}

void DirectSpeakersAudioProcessor::setIHostApplication(Steinberg::FUnknown * unknown)
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

void DirectSpeakersAudioProcessor::extensionSetState(std::string const & xmlStateStr)
{
  auto doc = XmlDocument(xmlStateStr);
  auto xmlState = doc.getDocumentElement();
  setStateInformation(xmlState.get(), false);
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new DirectSpeakersAudioProcessor();
}
