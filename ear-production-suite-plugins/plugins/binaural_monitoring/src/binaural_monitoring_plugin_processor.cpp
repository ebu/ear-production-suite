#include "binaural_monitoring_plugin_processor.hpp"
#include "binaural_monitoring_plugin_editor.hpp"
#include "variable_block_adapter.hpp"

#define deg2rad(angleDegrees) ((angleDegrees) * 0.01745329)
#define rad2deg(angleRadians) ((angleRadians) * 57.29577951)

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

void EarBinauralMonitoringAudioProcessor::oscMessageReceived(const OSCMessage & message)
{
  auto add = message.getAddressPattern();

  std::vector<float> vals(message.size(), 0.0);
  for(int i = 0; i < vals.size(); i++) {
    if(message[i].isFloat32()) {
      vals[i] = message[i].getFloat32();
    } else if(message[i].isInt32()) {
      vals[i] = (float)message[i].getInt32();
    } else {
      return;
    }
  }

  if(vals.size() == 1) {
    if(add.matches("/yaw")) {
      // Messages understood by SPARTA/COMPASS. Path also used by AmbiHead, but it expects normalised values - will not implement.
      oscEulerInput.y = vals[0];
    } else if(add.matches("/pitch")) {
      // Messages understood by SPARTA/COMPASS. Path also used by AmbiHead, but it expects normalised values - will not implement.
      oscEulerInput.p = vals[0];
    } else if(add.matches("/roll")) {
      // Messages understood by SPARTA/COMPASS. Path also used by AmbiHead, but it expects normalised values - will not implement.
      oscEulerInput.r = vals[0];
    } else if(add.matches("/hedrot/yaw")) {
      // Messages sent by Hedrot
      oscEulerInput.y = vals[0];
    } else if(add.matches("/hedrot/pitch")) {
      // Messages sent by Hedrot
      oscEulerInput.p = vals[0];
    } else if(add.matches("/hedrot/roll")) {
      // Messages sent by Hedrot
      oscEulerInput.r = vals[0];
    } else {
      return;
    }
    eulerToLatestQuat(EulerOrder::YPR);

  } else if(vals.size() == 3) {
    if(add.matches("/rotation")) {
      // Messages understood by Ambix
      oscEulerInput.p = vals[0];
      oscEulerInput.y = vals[1];
      oscEulerInput.r = vals[2];
      eulerToLatestQuat(EulerOrder::PYR);

    } else if(add.matches("/rendering/htrpy")) {
      // Messages understood by AudioLab SALTE
      oscEulerInput.r = vals[0];
      oscEulerInput.p = vals[1];
      oscEulerInput.y = vals[2];
      eulerToLatestQuat(EulerOrder::RPY);

    } else if(add.matches("/ypr")) {
      // Messages understood by SPARTA/COMPASS
      oscEulerInput.y = vals[0];
      oscEulerInput.p = vals[1];
      oscEulerInput.r = vals[2];
      eulerToLatestQuat(EulerOrder::YPR);

    } else {
      return;
    }

  } else if(vals.size() == 4) {
    if(add.matches("/quaternion")) {
      // Messages understood by Ambix
      latestQuat.w = vals[0];
      latestQuat.y = vals[1];
      latestQuat.x = -vals[2];
      latestQuat.z = vals[3];


    } else if(add.matches("/SceneRotator/quaternions")) {
      // Messages understood by IEM
      latestQuat.w = vals[0];
      latestQuat.x = vals[1];
      latestQuat.y = -vals[2];
      latestQuat.z = -vals[3];

    } else if(add.matches("/quaternions")) {
      // Messages understood by Unity plugin
      latestQuat.w = vals[0];
      latestQuat.x = -vals[1];
      latestQuat.z = -vals[2];
      latestQuat.y = -vals[3];

    } else {
      return;
    }

  } else if(vals.size() == 7) {
    if(add.matches("/head_pose")) {
      // Messages understood by Ambix
      oscEulerInput.p = vals[4];
      oscEulerInput.y = vals[5];
      oscEulerInput.r = vals[6];
      eulerToLatestQuat(EulerOrder::PYR);

    } else {
      return;
    }
  }

  processor_->setListenerOrientation(latestQuat.w, latestQuat.x, latestQuat.y, latestQuat.z);
}

juce::AudioProcessor::BusesProperties
EarBinauralMonitoringAudioProcessor::_getBusProperties() {
  return BusesProperties().withInput(
      "Input", AudioChannelSet::discreteChannels(64), true).withOutput("Left Ear", AudioChannelSet::mono(), true).withOutput("Right Ear", AudioChannelSet::mono(), true);
}

void EarBinauralMonitoringAudioProcessor::eulerToLatestQuat(EulerOrder order)
{
  float hr = deg2rad(oscEulerInput.r) / 2.0f;
  float hp = deg2rad(oscEulerInput.p) / 2.0f;
  float hy = deg2rad(oscEulerInput.y) / 2.0f;

  float cr = cos( hr );
  float cp = cos( hp );
  float cy = cos( hy );
  float sr = sin( hr );
  float sp = sin( hp );
  float sy = sin( hy );

  switch(order) {
    case YPR:
      latestQuat.x = sr * cp * cy - cr * sp * sy;
      latestQuat.y = cr * sp * cy + sr * cp * sy;
      latestQuat.z = cr * cp * sy - sr * sp * cy;
      latestQuat.w = cr * cp * cy + sr * sp * sy;
      break;

    case PYR:
      latestQuat.x = sr * cp * cy + cr * sp * sy;
      latestQuat.y = cr * sp * cy + sr * cp * sy;
      latestQuat.z = cr * cp * sy - sr * sp * cy;
      latestQuat.w = cr * cp * cy - sr * sp * sy;
      break;

    case RPY:
      latestQuat.x = sr * cp * cy + cr * sp * sy;
      latestQuat.y = cr * sp * cy - sr * cp * sy;
      latestQuat.z = cr * cp * sy + sr * sp * cy;
      latestQuat.w = cr * cp * cy - sr * sp * sy;
      break;

    case PRY:
      latestQuat.x = sr * cp * cy + cr * sp * sy;
      latestQuat.y = cr * sp * cy - sr * cp * sy;
      latestQuat.z = cr * cp * sy - sr * sp * cy;
      latestQuat.w = cr * cp * cy + sr * sp * sy;
      break;

    case YRP:
      latestQuat.x = sr * cp * cy - cr * sp * sy;
      latestQuat.y = cr * sp * cy + sr * cp * sy;
      latestQuat.z = cr * cp * sy + sr * sp * cy;
      latestQuat.w = cr * cp * cy - sr * sp * sy;
      break;

    case RYP:
      latestQuat.x = sr * cp * cy - cr * sp * sy;
      latestQuat.y = cr * sp * cy - sr * cp * sy;
      latestQuat.z = cr * cp * sy + sr * sp * cy;
      latestQuat.w = cr * cp * cy + sr * sp * sy;
      break;
  }

}

//==============================================================================
EarBinauralMonitoringAudioProcessor::EarBinauralMonitoringAudioProcessor()
    : AudioProcessor(_getBusProperties()) {
  backend_ = std::make_unique<ear::plugin::BinauralMonitoringBackend>(
    nullptr, 64);
  levelMeter_ = std::make_shared<ear::plugin::LevelMeterCalculator>(0, 0);

  auto vstPath = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentExecutableFile);
  vstPath = vstPath.getParentDirectory();
  vstPath = vstPath.getChildFile("default.tf");
  bearDataFilePath = vstPath.getFullPathName().toStdString();

  osc.addListener(this);
  oscConnected = osc.connect(oscPort);
}

EarBinauralMonitoringAudioProcessor::~EarBinauralMonitoringAudioProcessor() {
  osc.disconnect();
  oscConnected = false;
  osc.removeListener(this);
}

//==============================================================================
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
}

void EarBinauralMonitoringAudioProcessor::setStateInformation(const void* data,
                                                      int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new EarBinauralMonitoringAudioProcessor();
}
