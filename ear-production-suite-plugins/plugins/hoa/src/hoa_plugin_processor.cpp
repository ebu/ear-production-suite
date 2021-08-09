#include "hoa_plugin_processor.hpp"
/* Old DS Code
#include "speaker_setups.hpp"
// TODO - we probably need something like this for different flavours of HOA
*/
#include "components/non_automatable_parameter.hpp"
#include "hoa_plugin_editor.hpp"
#include "hoa_frontend_connector.hpp"

using namespace ear::plugin;

// TODO - update bus properties to max channel count for max HOA order
HoaAudioProcessor::HoaAudioProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", AudioChannelSet::discreteChannels(64), true)
              .withOutput("Output", AudioChannelSet::discreteChannels(64),
                          true)),
      samplerate_(48000),
      levelMeter_(std::make_shared<LevelMeterCalculator>(24, samplerate_)) {

  /* clang-format off */
  addParameter(routing_ =
    new ui::NonAutomatedParameter<AudioParameterInt>(
      "routing", "Routing",
      -1, 63, -1));
  //ME add, similar to DS
  addParameter(hoaTypeId_ =
    new ui::NonAutomatedParameter<AudioParameterInt>(
      "hoa_type", "HOA Type",
      0, 0xFFFF, 0));
  //Me end
  /*DS code
    addParameter(speakerSetupIndex_ =
    new ui::NonAutomatedParameter<AudioParameterInt>(
      "speaker_setup_index", "Speaker Setup Index",
      -1, static_cast<int>(SPEAKER_SETUPS.size() - 1), -1));
  */

  addParameter(bypass_ = new AudioParameterBool("byps", "Bypass", false));
  /* clang-format on */

  static_cast<ui::NonAutomatedParameter<AudioParameterInt>*>(routing_)->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  /* Old DS Code
  // Any NonAutomatedParameter will not set the plugin state as dirty when changed.
  // We used this hack to make sure the host (REAPER) knows something has changed (by "changing" a standard parameter to it's current value)
  static_cast<ui::NonAutomatedParameter<AudioParameterInt>*>(speakerSetupIndex_)->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };
  */
  //ME add similar to SD but don't fully get
// Any NonAutomatedParameter will not set the plugin state as dirty when changed.
// We used this hack to make sure the host (REAPER) knows something has changed (by "changing" a standard parameter to it's current value)
  static_cast<ui::NonAutomatedParameter<AudioParameterInt>*>(hoaTypeId_)
      ->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };
//ME end

  connector_ = std::make_unique<ui::HoaJuceFrontendConnector>(this);//creates instance of front end connector
  backend_ = std::make_unique<HoaBackend>(connector_.get());//creates instance of backend, passing to it a pointer to front end connector

  connector_->parameterValueChanged(0, routing_->get());
  connector_->parameterValueChanged(1, hoaTypeId_->get());//ME added, similar to DS (1.)

}

HoaAudioProcessor::~HoaAudioProcessor() {}

const String HoaAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool HoaAudioProcessor::acceptsMidi() const { return false; }
bool HoaAudioProcessor::producesMidi() const { return false; }
bool HoaAudioProcessor::isMidiEffect() const { return false; }

double HoaAudioProcessor::getTailLengthSeconds() const {
  return 0.0;
}

int HoaAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int HoaAudioProcessor::getCurrentProgram() { return 0; }
void HoaAudioProcessor::setCurrentProgram(int index) {}

//ME add not sure if good
void HoaAudioProcessor::setNumHoaTypes(int &numHoaTypes) {
  numHoaTypes_ = numHoaTypes;//THIS LINE CAUSING PROBLEMS
}//Me end

const String HoaAudioProcessor::getProgramName(int index) {
  return {};
}
void HoaAudioProcessor::changeProgramName(int index,
                                                     const String& newName) {}

void HoaAudioProcessor::prepareToPlay(double samplerate,
                                                 int samplesPerBlock) {
  if (samplerate_ != static_cast<int>(samplerate)) {
    samplerate_ = static_cast<int>(samplerate);
    levelMeter_->setup(24, samplerate_);
  }
}

void HoaAudioProcessor::releaseResources() {}

bool HoaAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
  if (layouts.getMainOutputChannelSet() ==
          AudioChannelSet::discreteChannels(24) &&
      layouts.getMainInputChannelSet() ==
          AudioChannelSet::discreteChannels(24)) {
    return true;
  }
  return false;
}

void HoaAudioProcessor::processBlock(AudioBuffer<float>& buffer,
MidiBuffer& midiMessages) {
  if(! bypass_->get()) {
    levelMeter_->process(buffer);
    backend_->triggerMetadataSend();
  }
}

bool HoaAudioProcessor::hasEditor() const { return true; }

AudioProcessorEditor* HoaAudioProcessor::createEditor() {//(1..)
  return new HoaAudioProcessorEditor(this);
}

void HoaAudioProcessor::getStateInformation(MemoryBlock& destData) {
  std::unique_ptr<XmlElement> xml(new XmlElement("HoaPlugin"));
  connectionId_ = backend_->getConnectionId();
  xml->setAttribute("connection_id", connectionId_.string());
  xml->setAttribute("routing", (int)*routing_);
  xml->setAttribute("hoa_type", (int)*hoaTypeId_);//ME added, similar to DS, not completely sure about

  copyXmlToBinary(*xml, destData);
}

void HoaAudioProcessor::setStateInformation(const void* data,
                                                       int sizeInBytes) {
  std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName("HoaPlugin")) {
      connectionId_ = communication::ConnectionId{
          xmlState
              ->getStringAttribute("connection_id",
                                   "00000000-0000-0000-0000-000000000000")
              .toStdString()};
      backend_->setConnectionId(connectionId_);
      auto con_id = connectionId_;
      *routing_ = xmlState->getIntAttribute("routing", -1);
      *hoaTypeId_ = xmlState->getIntAttribute("hoa_type", 0);//ME added, similar to DS but don't really know what this does

    }
}

void HoaAudioProcessor::updateTrackProperties(
    const TrackProperties& properties) {
  connector_->trackPropertiesChanged(properties);
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new HoaAudioProcessor();
}
