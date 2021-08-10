#include "direct_speakers_plugin_processor.hpp"
#include "speaker_setups.hpp"

#include "components/non_automatable_parameter.hpp"
#include "direct_speakers_plugin_editor.hpp"
#include "direct_speakers_frontend_connector.hpp"

using namespace ear::plugin;

DirectSpeakersAudioProcessor::DirectSpeakersAudioProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", AudioChannelSet::discreteChannels(24), true)
              .withOutput("Output", AudioChannelSet::discreteChannels(24),
                          true)),
      samplerate_(48000),
      levelMeter_(std::make_shared<LevelMeterCalculator>(24, samplerate_)) {
  /* clang-format off */
  addParameter(routing_ =
    new ui::NonAutomatedParameter<AudioParameterInt>(
      "routing", "Routing",
      -1, 63, -1));
  addParameter(speakerSetupIndex_ =
    new ui::NonAutomatedParameter<AudioParameterInt>(
      "speaker_setup_index", "Speaker Setup Index",
      -1, static_cast<int>(SPEAKER_SETUPS.size() - 1), -1));
      
  addParameter(bypass_ = new AudioParameterBool("byps", "Bypass", false));
  /* clang-format on */
  
  static_cast<ui::NonAutomatedParameter<AudioParameterInt>*>(routing_)->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };
  
  static_cast<ui::NonAutomatedParameter<AudioParameterInt>*>(speakerSetupIndex_)->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  connector_ = std::make_unique<ui::DirectSpeakersJuceFrontendConnector>(this);
  backend_ = std::make_unique<DirectSpeakersBackend>(connector_.get());

  connector_->parameterValueChanged(0, routing_->get());
  connector_->parameterValueChanged(1, speakerSetupIndex_->get());
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
  if (layouts.getMainOutputChannelSet() ==
          AudioChannelSet::discreteChannels(24) &&
      layouts.getMainInputChannelSet() ==
          AudioChannelSet::discreteChannels(24)) {
    return true;
  }
  return false;
}

void DirectSpeakersAudioProcessor::processBlock(AudioBuffer<float>& buffer,
                                                MidiBuffer& midiMessages) {
  if(! bypass_->get()) {
    levelMeter_->process(buffer);
    backend_->triggerMetadataSend();
  }
}

bool DirectSpeakersAudioProcessor::hasEditor() const { return true; }

AudioProcessorEditor* DirectSpeakersAudioProcessor::createEditor() {
  return new DirectSpeakersAudioProcessorEditor(this);
}

void DirectSpeakersAudioProcessor::getStateInformation(MemoryBlock& destData) {
  std::unique_ptr<XmlElement> xml(new XmlElement("DirectSpeakersPlugin"));
  connectionId_ = backend_->getConnectionId();
  xml->setAttribute("connection_id", connectionId_.string());
  xml->setAttribute("routing", (int)*routing_);
  xml->setAttribute("speaker_setup_index", (int)*speakerSetupIndex_);
  copyXmlToBinary(*xml, destData);
}

void DirectSpeakersAudioProcessor::setStateInformation(const void* data,
                                                       int sizeInBytes) {
  std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName("DirectSpeakersPlugin")) {
      connectionId_ = communication::ConnectionId{
          xmlState
              ->getStringAttribute("connection_id",
                                   "00000000-0000-0000-0000-000000000000")
              .toStdString()};
      backend_->setConnectionId(connectionId_);
      *routing_ = xmlState->getIntAttribute("routing", -1);
      *speakerSetupIndex_ =
          xmlState->getIntAttribute("speaker_setup_index", -1);
    }
}

void DirectSpeakersAudioProcessor::updateTrackProperties(
    const TrackProperties& properties) {
  connector_->trackPropertiesChanged(properties);
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new DirectSpeakersAudioProcessor();
}
