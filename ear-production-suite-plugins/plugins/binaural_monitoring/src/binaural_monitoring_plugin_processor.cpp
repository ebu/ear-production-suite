#include "binaural_monitoring_plugin_processor.hpp"
#include "../../shared/components/non_automatable_parameter.hpp"
#include "binaural_monitoring_backend.hpp"
#include "binaural_monitoring_frontend_connector.hpp"
#include "binaural_monitoring_plugin_editor.hpp"
#include "variable_block_adapter.hpp"

namespace ear {
namespace plugin {

template <>
struct BufferTraits<juce::AudioBuffer<float>> {
  using SampleType = float;
  using Buffer = juce::AudioBuffer<float>;
  static Eigen::Index channelCount(const Buffer& b) {
    return b.getNumChannels();
  }
  static Eigen::Index size(const Buffer& b) { return b.getNumSamples(); }
  static SampleType* getChannel(Buffer& b, std::size_t n) {
    return b.getWritePointer(static_cast<int>(n));
  }
  static const SampleType* getChannel(Buffer const& b, std::size_t n) {
    return b.getReadPointer(static_cast<int>(n));
  }
  static SampleType** getChannels(Buffer& b) {
    return b.getArrayOfWritePointers();
  }
  static const SampleType** getChannels(Buffer const& b) {
    return b.getArrayOfReadPointers();
  }
};

}  // namespace plugin
}  // namespace ear

#include "binaural_monitoring_audio_processor.hpp"
#include "binaural_monitoring_backend.hpp"

using namespace ear::plugin;

//==============================================================================
EarBinauralMonitoringAudioProcessor::EarBinauralMonitoringAudioProcessor()
    : AudioProcessor(_getBusProperties()) {
  levelMeter_ = std::make_shared<ear::plugin::LevelMeterCalculator>(0, 0);

  /* clang-format off */
  addParameter(bypass_ = new AudioParameterBool("byps", "Bypass", false)); // Used for setting plugin state dirty
  addParameter(yaw_ = new ui::NonAutomatedParameter<AudioParameterFloat>("yaw", "Yaw", NormalisableRange<float>{-180.f, 180.f}, 0.f));
  addParameter(pitch_ = new ui::NonAutomatedParameter<AudioParameterFloat>("pitch", "Pitch", NormalisableRange<float>{-180.f, 180.f}, 0.f));
  addParameter(roll_ = new ui::NonAutomatedParameter<AudioParameterFloat>("roll", "Roll", NormalisableRange<float>{-180.f, 180.f}, 0.f));
  addParameter(oscEnable_ = new ui::NonAutomatedParameter<AudioParameterBool>("oscEnable", "Enable OSC", false));
  addParameter(oscPort_ = new ui::NonAutomatedParameter<AudioParameterInt>("oscPort", "OSC Port", 1, 65535, 8000));
  /* clang-format on */

  static_cast<ui::NonAutomatedParameter<AudioParameterBool>*>(oscEnable_)->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  static_cast<ui::NonAutomatedParameter<AudioParameterInt>*>(oscPort_)->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  backend_ = std::make_unique<ear::plugin::BinauralMonitoringBackend>(nullptr, 64);
  connector_ = std::make_unique<ui::BinauralMonitoringJuceFrontendConnector>(this);
  connector_->setListenerOrientationInstance(backend_->listenerOrientation);

  connector_->parameterValueChanged(1, yaw_->get());
  connector_->parameterValueChanged(2, pitch_->get());
  connector_->parameterValueChanged(3, roll_->get());
  connector_->parameterValueChanged(4, oscEnable_->get());
  connector_->parameterValueChanged(5, oscPort_->get());

  auto vstPath = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentExecutableFile);
  vstPath = vstPath.getParentDirectory();
  vstPath = vstPath.getChildFile("default.tf");
  bearDataFilePath = vstPath.getFullPathName().toStdString();

  oscReceiver.onReceiveEuler = [this](ListenerOrientation::Euler euler) {
    connector_->setEuler(euler);
  };

  oscReceiver.onReceiveQuaternion = [this](ListenerOrientation::Quaternion quat) {
    connector_->setQuaternion(quat);
  };

  oscReceiver.listenForConnections(8000);
}

EarBinauralMonitoringAudioProcessor::~EarBinauralMonitoringAudioProcessor() {
}

//==============================================================================
juce::AudioProcessor::BusesProperties
EarBinauralMonitoringAudioProcessor::_getBusProperties() {
  return BusesProperties().withInput(
      "Input", AudioChannelSet::discreteChannels(64), true).withOutput("Left Ear", AudioChannelSet::mono(), true).withOutput("Right Ear", AudioChannelSet::mono(), true);
}

const String EarBinauralMonitoringAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool EarBinauralMonitoringAudioProcessor::acceptsMidi() const { return false; }

bool EarBinauralMonitoringAudioProcessor::producesMidi() const { return false; }

bool EarBinauralMonitoringAudioProcessor::isMidiEffect() const { return false; }

double EarBinauralMonitoringAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int EarBinauralMonitoringAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int EarBinauralMonitoringAudioProcessor::getCurrentProgram() { return 0; }

void EarBinauralMonitoringAudioProcessor::setCurrentProgram(int index) {}

const String EarBinauralMonitoringAudioProcessor::getProgramName(int index) {
  return {};
}

void EarBinauralMonitoringAudioProcessor::changeProgramName(int index,
                                                    const String& newName) {}

//==============================================================================
void EarBinauralMonitoringAudioProcessor::prepareToPlay(double sampleRate,
                                                int samplesPerBlock) {
  samplerate_ = sampleRate;
  blocksize_ = samplesPerBlock;

  levelMeter_->setup(2, sampleRate);

}

void EarBinauralMonitoringAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

bool EarBinauralMonitoringAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
  if (layouts.getMainOutputChannelSet() ==
          AudioChannelSet::discreteChannels(2) &&
      layouts.getMainInputChannelSet() ==
          AudioChannelSet::discreteChannels(64)) {
    return true;
  }

  return false;
}

void EarBinauralMonitoringAudioProcessor::processBlock(AudioBuffer<float>& buffer,
                                               MidiBuffer&) {
  ScopedNoDenormals noDenormals;

  if(bypass_->get()) return;

  auto objIds = backend_->getActiveObjectIds();
  auto dsIds = backend_->getActiveDirectSpeakersIds();
  auto hoaIds = backend_->getActiveHoaIds();

  size_t numHoa = backend_->getTotalHoaChannels();
  size_t numObj = backend_->getTotalObjectChannels();
  size_t numDs = backend_->getTotalDirectSpeakersChannels();

  // Ensure BEAR has enough channels configured
  if(!processor_ || !processor_->configSupports(numObj, numDs, numHoa, samplerate_, blocksize_)) {
    processor_ = std::make_unique<ear::plugin::BinauralMonitoringAudioProcessor>(
      numObj, numDs, numHoa, samplerate_, blocksize_, bearDataFilePath);
  }

  // Listener Position

  auto latestQuat = backend_->listenerOrientation->getQuaternion();
  processor_->setListenerOrientation(latestQuat.w, latestQuat.x, latestQuat.y, latestQuat.z);

  // BEAR Metadata

  for(auto& connId : objIds) {
    auto md = backend_->getLatestObjectsTypeMetadata(connId);
    if(md->channel >= 0) {
      processor_->pushBearMetadata(md->channel, &(md->earMetadata));
    }
  }

  for(auto& connId : dsIds) {
    auto md = backend_->getLatestDirectSpeakersTypeMetadata(connId);
    if(md->startingChannel >= 0) {
      for(int index = 0; index < md->earMetadata.size(); index++) {
        processor_->pushBearMetadata(md->startingChannel + index, &(md->earMetadata[index]));
      }
    }
  }

  for(auto& connId : hoaIds) {
    auto md = backend_->getLatestHoaTypeMetadata(connId);
    if(md->startingChannel >= 0) {
      for(int index = 0; index < md->earMetadata.size(); index++) {
        processor_->pushBearMetadata(md->startingChannel + index, &(md->earMetadata[index]));
      }
    }
  }

  // BEAR audio processing
  processor_->process(buffer, buffer);

  // Zero unused output channels - probably not necessary in most cases;
  // e.g, REAPER only takes the first n channels defined by the output bus width
  //   remaining are passed through.
  auto buffMaxChns = buffer.getNumChannels();
  auto buffSamples = buffer.getNumSamples();
  for(int ch = 2; ch < buffMaxChns; ch++) {
    buffer.clear(ch, 0, buffSamples);
  }

  // Meters
  if (buffer.getNumChannels() >= levelMeter_->channels()) {
    levelMeter_->process(buffer);
  }

}

//==============================================================================
bool EarBinauralMonitoringAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* EarBinauralMonitoringAudioProcessor::createEditor() {
  return new EarBinauralMonitoringAudioProcessorEditor(this);
}

//==============================================================================
void EarBinauralMonitoringAudioProcessor::getStateInformation(MemoryBlock& destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.

  std::unique_ptr<XmlElement> xml(new XmlElement("BinauralMonitoringPlugin"));
  xml->setAttribute("oscEnable", (bool)*oscEnable_);
  xml->setAttribute("oscPort", (int)*oscPort_);
  copyXmlToBinary(*xml, destData);
}

void EarBinauralMonitoringAudioProcessor::setStateInformation(const void* data,
                                                      int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.

  std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
  if (xmlState.get() != nullptr) {
    if (xmlState->hasTagName("BinauralMonitoringPlugin")) {
      // TODO - need to push these values to backend and OSC receiver, or just works?
      if(xmlState->hasAttribute("oscEnable")) {
        *oscEnable_ = xmlState->getBoolAttribute("oscEnable", false);
      }
      if(xmlState->hasAttribute("oscPort")) {
        *oscPort_ = xmlState->getIntAttribute("oscPort", 8000);
      }
    }
  }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new EarBinauralMonitoringAudioProcessor();
}
