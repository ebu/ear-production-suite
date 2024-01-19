#include "binaural_monitoring_plugin_processor.hpp"
#include "../../shared/components/non_automatable_parameter.hpp"
#include "binaural_monitoring_backend.hpp"
#include "binaural_monitoring_frontend_connector.hpp"
#include "binaural_monitoring_plugin_editor.hpp"
#include "variable_block_adapter.hpp"
#include <version/eps_version.h>
#include <helper/resource_paths_juce-file.hpp>
#include <daw_channel_count.h>

#define DEFAULT_OSC_PORT 8000

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

juce::File getBearDataFileDirectory(){
  auto vstPath = juce::File::getSpecialLocation(
      juce::File::SpecialLocationType::currentExecutableFile);
  vstPath = vstPath.getParentDirectory();
#ifdef __APPLE__
  vstPath = vstPath.getParentDirectory();
  vstPath = vstPath.getChildFile("Resources");
#endif
  return vstPath;
};

}  // namespace plugin
}  // namespace ear

#include "binaural_monitoring_audio_processor.hpp"
#include "binaural_monitoring_backend.hpp"

using namespace ear::plugin;

//==============================================================================
EarBinauralMonitoringAudioProcessor::EarBinauralMonitoringAudioProcessor()
    : AudioProcessor(_getBusProperties()) {
  dataFileManager = std::make_unique<ear::plugin::DataFileManager>(
      getBearDataFileDirectory());
  dataFileManager->onSelectedDataFileChange(
      [this](std::shared_ptr<ear::plugin::DataFileManager::DataFile> df) {
        writeConfigFile();
        restartBearProcessor();
      });
  levelMeter_ = std::make_shared<ear::plugin::LevelMeterCalculator>(0, 0);

  /* clang-format off */
  addParameter(bypass_ = new AudioParameterBool("byps", "Bypass", false)); // Used for setting plugin state dirty
  addParameter(yaw_ = new AudioParameterFloat("yaw", "Yaw", NormalisableRange<float>{-180.f, 180.f}, 0.f));
  addParameter(pitch_ = new AudioParameterFloat("pitch", "Pitch", NormalisableRange<float>{-180.f, 180.f}, 0.f));
  addParameter(roll_ = new AudioParameterFloat("roll", "Roll", NormalisableRange<float>{-180.f, 180.f}, 0.f));
  addParameter(oscEnable_ = new ui::NonAutomatedParameter<AudioParameterBool>("oscEnable", "Enable OSC", false));
  addParameter(oscPort_ = new ui::NonAutomatedParameter<AudioParameterInt>("oscPort", "OSC Port", 1, 65535, DEFAULT_OSC_PORT));
  addParameter(oscInvertYaw_ = new ui::NonAutomatedParameter<AudioParameterBool>("oscInvertYaw", "Invert OSC Yaw Values", false));
  addParameter(oscInvertPitch_ = new ui::NonAutomatedParameter<AudioParameterBool>("oscInvertPitch", "Invert OSC Pitch Values", false));
  addParameter(oscInvertRoll_ = new ui::NonAutomatedParameter<AudioParameterBool>("oscInvertRoll", "Invert OSC Roll Values", false));
  addParameter(oscInvertQuatW_ = new ui::NonAutomatedParameter<AudioParameterBool>("oscInvertQuatW", "Invert OSC Quaternion W Values", false));
  addParameter(oscInvertQuatX_ = new ui::NonAutomatedParameter<AudioParameterBool>("oscInvertQuatX", "Invert OSC Quaternion X Values", false));
  addParameter(oscInvertQuatY_ = new ui::NonAutomatedParameter<AudioParameterBool>("oscInvertQuatY", "Invert OSC Quaternion Y Values", false));
  addParameter(oscInvertQuatZ_ = new ui::NonAutomatedParameter<AudioParameterBool>("oscInvertQuatZ", "Invert OSC Quaternion Z Values", false));
  /* clang-format on */

  static_cast<ui::NonAutomatedParameter<AudioParameterBool>*>(oscEnable_)
      ->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  static_cast<ui::NonAutomatedParameter<AudioParameterInt>*>(oscPort_)
      ->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  static_cast<ui::NonAutomatedParameter<AudioParameterBool>*>(oscInvertYaw_)
    ->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  static_cast<ui::NonAutomatedParameter<AudioParameterBool>*>(oscInvertPitch_)
    ->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  static_cast<ui::NonAutomatedParameter<AudioParameterBool>*>(oscInvertRoll_)
    ->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  static_cast<ui::NonAutomatedParameter<AudioParameterBool>*>(oscInvertQuatW_)
    ->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  static_cast<ui::NonAutomatedParameter<AudioParameterBool>*>(oscInvertQuatX_)
    ->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  static_cast<ui::NonAutomatedParameter<AudioParameterBool>*>(oscInvertQuatY_)
    ->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  static_cast<ui::NonAutomatedParameter<AudioParameterBool>*>(oscInvertQuatZ_)
    ->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  backend_ = std::make_unique<ear::plugin::BinauralMonitoringBackend>(
      nullptr, MAX_DAW_CHANNELS);
  connector_ =
      std::make_unique<ui::BinauralMonitoringJuceFrontendConnector>(this);
  connector_->setListenerOrientationInstance(backend_->listenerOrientation);

  connector_->parameterValueChanged(1, yaw_->get());
  connector_->parameterValueChanged(2, pitch_->get());
  connector_->parameterValueChanged(3, roll_->get());
  connector_->parameterValueChanged(4, oscEnable_->get());
  connector_->parameterValueChanged(5, oscPort_->get());
  connector_->parameterValueChanged(6, oscInvertYaw_->get());
  connector_->parameterValueChanged(7, oscInvertPitch_->get());
  connector_->parameterValueChanged(8, oscInvertRoll_->get());
  connector_->parameterValueChanged(9, oscInvertQuatW_->get());
  connector_->parameterValueChanged(10, oscInvertQuatX_->get());
  connector_->parameterValueChanged(11, oscInvertQuatY_->get());
  connector_->parameterValueChanged(12, oscInvertQuatZ_->get());

  oscReceiver.onReceiveEuler = [this](ListenerOrientation::Euler euler) {
    connector_->setEuler(euler);
  };

  oscReceiver.onReceiveQuaternion =
      [this](ListenerOrientation::Quaternion quat) {
        connector_->setQuaternion(quat);
      };

  oscEnable_->addListener(this);
  oscPort_->addListener(this);
  oscInvertYaw_->addListener(this);
  oscInvertPitch_->addListener(this);
  oscInvertRoll_->addListener(this);
  oscInvertQuatW_->addListener(this);
  oscInvertQuatX_->addListener(this);
  oscInvertQuatY_->addListener(this);
  oscInvertQuatZ_->addListener(this);

  configFileOptions.applicationName = ProjectInfo::projectName;
  configFileOptions.filenameSuffix = ".settings";
  configFileOptions.folderName = ResourcePaths::getSettingsDirectory(true).getFullPathName();
  configFileOptions.storageFormat = PropertiesFile::storeAsXML;
  readConfigFile(); // This will fire up the processor too because of data file change.
}

EarBinauralMonitoringAudioProcessor::~EarBinauralMonitoringAudioProcessor() {}

bool EarBinauralMonitoringAudioProcessor::rendererStarted() {
  std::lock_guard<std::mutex> lock(processorMutex_);
  return (!processor_ || processor_->rendererStarted());
}

void EarBinauralMonitoringAudioProcessor::parameterValueChanged(
  int parameterIndex, float newValue) {
  if(parameterIndex >= 4 && parameterIndex <= 12) {
    // OSC controls
    if(parameterIndex == 4 || parameterIndex == 5) {
      // OSC Enable || OSC Port
      if(oscEnable_->get()) {
        oscReceiver.listenForConnections(oscPort_->get());
      } else {
        oscReceiver.disconnect();
      }
    } else {
      // Invert control
      oscReceiver.setInverts({
        oscInvertYaw_->get(),
        oscInvertPitch_->get(),
        oscInvertRoll_->get(),
        oscInvertQuatW_->get(),
        oscInvertQuatX_->get(),
        oscInvertQuatY_->get(),
        oscInvertQuatZ_->get()
      });
    }
    writeConfigFile();
  }
}

void EarBinauralMonitoringAudioProcessor::parameterGestureChanged(
    int parameterIndex, bool gestureIsStarting) {}

void EarBinauralMonitoringAudioProcessor::timerCallback() {
  stopTimer();
  std::lock_guard<std::mutex> lock(processorMutex_);
  processor_->setIsPlaying(false);
}

//==============================================================================
juce::AudioProcessor::BusesProperties
EarBinauralMonitoringAudioProcessor::_getBusProperties() {
  auto ret = BusesProperties().withInput(
      "Input", AudioChannelSet::discreteChannels(MAX_DAW_CHANNELS), true);
  ret = ret.withOutput("Left Ear", AudioChannelSet::mono(), true);
  ret = ret.withOutput("Right Ear", AudioChannelSet::mono(), true);
  ret = ret.withOutput("(Unused)", AudioChannelSet::discreteChannels(MAX_DAW_CHANNELS-2), true);
  return ret;
}

void EarBinauralMonitoringAudioProcessor::restartBearProcessor(
    bool onlyOnConfigChange) {
  std::lock_guard<std::mutex> lock(processorMutex_);
  if (!onlyOnConfigChange || !processor_ ||
      !processor_->configMatches(samplerate_, blocksize_)) {
    auto bearDataFile = dataFileManager->getSelectedDataFileInfo();
    std::string dataFilePath;
    if (bearDataFile) {
      dataFilePath = bearDataFile->fullPath.getFullPathName().toStdString();
    }
    processor_ =
        std::make_unique<ear::plugin::BinauralMonitoringAudioProcessor>(
            MAX_DAW_CHANNELS, MAX_DAW_CHANNELS, MAX_DAW_CHANNELS, samplerate_,
            blocksize_, dataFilePath);
    if (connector_) {
      connector_->setRendererStatus(processor_->getBearStatus());
    }
  }
}

bool EarBinauralMonitoringAudioProcessor::readConfigFile() {
  configRestoreState = ConfigRestoreState::IN_PROGRESS;
  PropertiesFile props(configFileOptions);
  // reload() check alone isn't sufficient - returns true if file didn't exist
  if(props.reload() && props.getAllProperties().size() > 0) {
    *oscEnable_ = props.getBoolValue("oscEnable", false);
    *oscPort_ = props.getIntValue("oscPort", DEFAULT_OSC_PORT);
    *oscInvertYaw_ = props.getBoolValue("oscInvertYaw", false);
    *oscInvertPitch_ = props.getBoolValue("oscInvertPitch", false);
    *oscInvertRoll_ = props.getBoolValue("oscInvertRoll", false);
    *oscInvertQuatW_ = props.getBoolValue("oscInvertQuatW", false);
    *oscInvertQuatX_ = props.getBoolValue("oscInvertQuatX", false);
    *oscInvertQuatY_ = props.getBoolValue("oscInvertQuatY", false);
    *oscInvertQuatZ_ = props.getBoolValue("oscInvertQuatZ", false);
    auto selectedDataFile = props.getValue("bearPreferredDataFile");
    if (selectedDataFile.isEmpty() ||
        !dataFileManager->setSelectedDataFile(selectedDataFile)) {
      dataFileManager->setSelectedDataFileDefault();
    }
    configRestoreState = ConfigRestoreState::RESTORED;
    return true;
  }
  configRestoreState = ConfigRestoreState::NOT_RESTORED;
  return false;
}

bool EarBinauralMonitoringAudioProcessor::writeConfigFile()
{
  if(configRestoreState == ConfigRestoreState::IN_PROGRESS) return false;
  PropertiesFile props(configFileOptions);
  props.setValue("epsVersion", juce::String(eps::baseVersion()));
  props.setValue("oscEnable", (bool)*oscEnable_);
  props.setValue("oscPort", (int)*oscPort_);
  props.setValue("oscInvertYaw", (bool)*oscInvertYaw_);
  props.setValue("oscInvertPitch", (bool)*oscInvertPitch_);
  props.setValue("oscInvertRoll", (bool)*oscInvertRoll_);
  props.setValue("oscInvertQuatW", (bool)*oscInvertQuatW_);
  props.setValue("oscInvertQuatX", (bool)*oscInvertQuatX_);
  props.setValue("oscInvertQuatY", (bool)*oscInvertQuatY_);
  props.setValue("oscInvertQuatZ", (bool)*oscInvertQuatZ_);
  if (auto selectedDataFile = dataFileManager->getSelectedDataFileInfo()) {
    props.setValue("bearPreferredDataFile", selectedDataFile->filename);
  }
  return props.save();
}

const String EarBinauralMonitoringAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool EarBinauralMonitoringAudioProcessor::acceptsMidi() const { return false; }

bool EarBinauralMonitoringAudioProcessor::producesMidi() const { return false; }

bool EarBinauralMonitoringAudioProcessor::isMidiEffect() const { return false; }

double EarBinauralMonitoringAudioProcessor::getTailLengthSeconds() const {
  return 0.0;
}

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

void EarBinauralMonitoringAudioProcessor::changeProgramName(
    int index, const String& newName) {}

//==============================================================================
void EarBinauralMonitoringAudioProcessor::prepareToPlay(double sampleRate,
                                                        int samplesPerBlock) {
  samplerate_ = sampleRate;
  blocksize_ = samplesPerBlock;
  levelMeter_->setup(2, sampleRate);
  restartBearProcessor(true);
}

void EarBinauralMonitoringAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

bool EarBinauralMonitoringAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {

  // Must accept default config specified in ctor

  if(layouts.inputBuses.size() != 1)
    return false;
  if (layouts.inputBuses[0] !=
      AudioChannelSet::discreteChannels(MAX_DAW_CHANNELS))
    return false;

  if(layouts.outputBuses.size() != 3)
    return false;
  if(layouts.outputBuses[0] != AudioChannelSet::mono())
    return false;
  if(layouts.outputBuses[1] != AudioChannelSet::mono())
    return false;
  if (layouts.outputBuses[2] !=
      AudioChannelSet::discreteChannels(MAX_DAW_CHANNELS - 2))
    return false;

  return true;
}

void EarBinauralMonitoringAudioProcessor::processBlock(
    AudioBuffer<float>& buffer, MidiBuffer&) {
  ScopedNoDenormals noDenormals;

  stopTimer();

  if(bypass_->get() || backend_->isExporting()) {
    if(getTotalNumOutputChannels() > 0 && getTotalNumInputChannels() > 0) {
      // Sum all in to first channel
      for(int srcChn = 1; srcChn < buffer.getNumChannels(); srcChn++) {
        buffer.addFrom(0, 0, buffer.getReadPointer(srcChn), buffer.getNumSamples());
      }
      // Copy to/zero others
      for(int destChn = 1; destChn < buffer.getNumChannels(); destChn++) {
        if(destChn < getTotalNumOutputChannels()) {
          buffer.copyFrom(destChn, 0, buffer.getReadPointer(0), buffer.getNumSamples());
        } else {
          for(int sampleNum = 0; sampleNum < buffer.getNumSamples(); sampleNum++) {
            buffer.setSample(destChn, sampleNum, 0.f);
          }
        }
      }
    }
    return;
  }

  {
    std::lock_guard<std::mutex> lock(processorMutex_);

    // Check BEAR has started - if not, we still want to zero output to make problem obvious
    if (!processor_ || !processor_->rendererStarted()) {
      buffer.clear();
      return;
    }

    processor_->setIsPlaying(true);

    auto objIds = backend_->getActiveObjectIds();
    auto dsIds = backend_->getActiveDirectSpeakersIds();
    auto hoaIds = backend_->getActiveHoaIds();

    size_t numHoa = backend_->getTotalHoaChannels();
    size_t numObj = backend_->getTotalObjectChannels();
    size_t numDs = backend_->getTotalDirectSpeakersChannels();

    // Check BEAR has enough channels configured
    if(!processor_->updateChannelCounts(numObj, numDs, numHoa)) {
      assert(false);
      return;
    }

    // Listener Position
    auto latestQuat = backend_->listenerOrientation->getQuaternion();
    processor_->setListenerOrientation(latestQuat.w, latestQuat.x, latestQuat.y,
                                       latestQuat.z);

    // BEAR Metadata

    for(auto& connId : objIds) {
      auto md = backend_->getLatestObjectsTypeMetadata(connId);
      if(md.has_value() && md->channel >= 0) {
        processor_->pushBearMetadata(md->channel, &(md->earMetadata));
      }
    }

    for(auto& connId : dsIds) {
      auto md = backend_->getLatestDirectSpeakersTypeMetadata(connId);
      if(md.has_value() && md->startingChannel >= 0) {
        for(int index = 0; index < md->earMetadata.size(); index++) {
          processor_->pushBearMetadata(
            md->startingChannel + index,
            &(md->earMetadata[index]));  // earMetadata is a vector for DS but
                                         // not for obj or HOA
        }
      }
    }

    size_t streamIdentifier = 0;
    for(auto& connId : hoaIds) {
      auto md = backend_->getLatestHoaTypeMetadata(connId);
      if(md.has_value() && md->startingChannel >= 0) {
        processor_->pushBearMetadata(md->startingChannel, &(md->earMetadata),
                                     streamIdentifier++);
      }
    }

    // BEAR audio processing
    processor_->process(buffer, buffer);
  }

  // Clear unused output channels
  for(int outChn = 2; outChn < buffer.getNumChannels(); ++outChn) {
    buffer.clear(outChn, 0, buffer.getNumSamples());
  }

  // Meters
  if (getActiveEditor() && buffer.getNumChannels() >= levelMeter_->channels()) {
    levelMeter_->process(buffer);
  }

  startTimer(500);
}

//==============================================================================
bool EarBinauralMonitoringAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* EarBinauralMonitoringAudioProcessor::createEditor() {
  levelMeter_->resetLevels();
  return new EarBinauralMonitoringAudioProcessorEditor(this);
}

//==============================================================================
void EarBinauralMonitoringAudioProcessor::getStateInformation(
    MemoryBlock& destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.

  std::unique_ptr<XmlElement> xml(new XmlElement("BinauralMonitoringPlugin"));
  xml->setAttribute("oscEnable", (bool)*oscEnable_);
  xml->setAttribute("oscPort", (int)*oscPort_);
  xml->setAttribute("oscInvertYaw", (bool)*oscInvertYaw_);
  xml->setAttribute("oscInvertPitch", (bool)*oscInvertPitch_);
  xml->setAttribute("oscInvertRoll", (bool)*oscInvertRoll_);
  xml->setAttribute("oscInvertQuatW", (bool)*oscInvertQuatW_);
  xml->setAttribute("oscInvertQuatX", (bool)*oscInvertQuatX_);
  xml->setAttribute("oscInvertQuatY", (bool)*oscInvertQuatY_);
  xml->setAttribute("oscInvertQuatZ", (bool)*oscInvertQuatZ_);
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
      // OSC should only restore from project data if wasn't restored from global settings (config file)
      if(configRestoreState == ConfigRestoreState::NOT_RESTORED) {
        if(xmlState->hasAttribute("oscEnable")) {
          *oscEnable_ = xmlState->getBoolAttribute("oscEnable", false);
        }
        if(xmlState->hasAttribute("oscPort")) {
          *oscPort_ = xmlState->getIntAttribute("oscPort", DEFAULT_OSC_PORT);
        }
        if(xmlState->hasAttribute("oscInvertYaw")) {
          *oscInvertYaw_ = xmlState->getBoolAttribute("oscInvertYaw", false);
        }
        if(xmlState->hasAttribute("oscInvertPitch")) {
          *oscInvertPitch_ = xmlState->getBoolAttribute("oscInvertPitch", false);
        }
        if(xmlState->hasAttribute("oscInvertRoll")) {
          *oscInvertRoll_ = xmlState->getBoolAttribute("oscInvertRoll", false);
        }
        if(xmlState->hasAttribute("oscInvertQuatW")) {
          *oscInvertQuatW_ = xmlState->getBoolAttribute("oscInvertQuatW", false);
        }
        if(xmlState->hasAttribute("oscInvertQuatX")) {
          *oscInvertQuatX_ = xmlState->getBoolAttribute("oscInvertQuatX", false);
        }
        if(xmlState->hasAttribute("oscInvertQuatY")) {
          *oscInvertQuatY_ = xmlState->getBoolAttribute("oscInvertQuatY", false);
        }
        if(xmlState->hasAttribute("oscInvertQuatZ")) {
          *oscInvertQuatZ_ = xmlState->getBoolAttribute("oscInvertQuatZ", false);
        }
      }
    }
  }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new EarBinauralMonitoringAudioProcessor();
}
