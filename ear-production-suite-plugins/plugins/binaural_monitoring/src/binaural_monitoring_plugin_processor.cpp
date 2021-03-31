#include "binaural_monitoring_plugin_processor.hpp"
#include "binaural_monitoring_plugin_editor.hpp"
#include "variable_block_adapter.hpp"

//TODO - remove once OSC work done
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

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

  listenerOrientation = std::make_shared<ListenerOrientation>();
  listenerOrientation->setCoordinateUpdateHandler(
    [this]() {
      // TODO - inform processor_->setListenerOrientation(latestQuat.w, latestQuat.x, latestQuat.y, latestQuat.z);
      //   --- That should set a flag to let the proc method know to update bear listner orientation before DSPing
      // Need locks for OSC messgae tread vs audio proc thread here?
      if(processor_) {
        auto latestQuat = listenerOrientation->getQuaternion();
        processor_->setListenerOrientation(latestQuat.w, latestQuat.x, latestQuat.y, latestQuat.z);
      }
      // TODO - update editor - need to put on message queue or can do directly?
    }
  );

  oscReceiver.setListenerOrientationHandler(listenerOrientation);
  oscReceiver.setOnConnectionStatusChangeTextHandler(
    [this](std::string newStatus) {
      // TODO - update editor - need to put on message queue or can do directly?
    }
  );
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
      auto latestQuat = listenerOrientation->getQuaternion();
      processor_->setListenerOrientation(latestQuat.w, latestQuat.x, latestQuat.y, latestQuat.z);
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

ListenerOrientationOscReceiver::ListenerOrientationOscReceiver()
{
  osc.addListener(this);
}

ListenerOrientationOscReceiver::~ListenerOrientationOscReceiver()
{
  osc.disconnect();
  osc.removeListener(this);
}

void ListenerOrientationOscReceiver::setOnConnectionStatusChangeTextHandler(std::function<void(std::string newStatus)> callback)
{
  statusTextCallback = callback;
}

void ListenerOrientationOscReceiver::setListenerOrientationHandler(std::shared_ptr<ListenerOrientation>listenerOrientationInstance)
{
  listenerOrientation = listenerOrientationInstance;
}

void ListenerOrientationOscReceiver::listenForConnections(uint16_t port)
{
  disconnect();
  isListening = osc.connect(port);
  updateStatusText(std::string(isListening ? "OSC Ready." : "OSC Connection Error."));
}

void ListenerOrientationOscReceiver::disconnect()
{
  stopTimer();
  osc.disconnect();
  isListening = false;
  updateStatusText(std::string("OSC Closed."));
}

void ListenerOrientationOscReceiver::oscMessageReceived(const OSCMessage & message)
{
  stopTimer();

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
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      if(listenerOrientation) listenerOrientation->setEuler(oscEulerInput);
    } else if(add.matches("/pitch")) {
      // Messages understood by SPARTA/COMPASS. Path also used by AmbiHead, but it expects normalised values - will not implement.
      oscEulerInput.p = vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      if(listenerOrientation) listenerOrientation->setEuler(oscEulerInput);
    } else if(add.matches("/roll")) {
      // Messages understood by SPARTA/COMPASS. Path also used by AmbiHead, but it expects normalised values - will not implement.
      oscEulerInput.r = vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      if(listenerOrientation) listenerOrientation->setEuler(oscEulerInput);
    } else if(add.matches("/hedrot/yaw")) {
      // Messages sent by Hedrot
      oscEulerInput.y = vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      if(listenerOrientation) listenerOrientation->setEuler(oscEulerInput);
    } else if(add.matches("/hedrot/pitch")) {
      // Messages sent by Hedrot
      oscEulerInput.p = vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      if(listenerOrientation) listenerOrientation->setEuler(oscEulerInput);
    } else if(add.matches("/hedrot/roll")) {
      // Messages sent by Hedrot
      oscEulerInput.r = vals[0];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      if(listenerOrientation) listenerOrientation->setEuler(oscEulerInput);

    } else {
      return;
    }

  } else if(vals.size() == 3) {
    if(add.matches("/rotation")) {
      // Messages understood by Ambix
      oscEulerInput.p = vals[0];
      oscEulerInput.y = vals[1];
      oscEulerInput.r = vals[2];
      oscEulerInput.order = ListenerOrientation::EulerOrder::PYR;
      if(listenerOrientation) listenerOrientation->setEuler(oscEulerInput);

    } else if(add.matches("/rendering/htrpy")) {
      // Messages understood by AudioLab SALTE
      oscEulerInput.r = vals[0];
      oscEulerInput.p = vals[1];
      oscEulerInput.y = vals[2];
      oscEulerInput.order = ListenerOrientation::EulerOrder::RPY;
      if(listenerOrientation) listenerOrientation->setEuler(oscEulerInput);

    } else if(add.matches("/ypr")) {
      // Messages understood by SPARTA/COMPASS
      oscEulerInput.y = vals[0];
      oscEulerInput.p = vals[1];
      oscEulerInput.r = vals[2];
      oscEulerInput.order = ListenerOrientation::EulerOrder::YPR;
      if(listenerOrientation) listenerOrientation->setEuler(oscEulerInput);

    } else {
      return;
    }

  } else if(vals.size() == 4) {

    if(add.matches("/quaternion")) {
      // Messages understood by Ambix
      if(listenerOrientation) {
        ListenerOrientation::Quaternion quat;
        quat.w = vals[0];
        quat.y = vals[1];
        quat.x = -vals[2];
        quat.z = vals[3];
        listenerOrientation->setQuaternion(quat);
      }

    } else if(add.matches("/SceneRotator/quaternions")) {
      // Messages understood by IEM
      if(listenerOrientation) {
        ListenerOrientation::Quaternion quat;
        quat.w = vals[0];
        quat.x = vals[1];
        quat.y = -vals[2];
        quat.z = -vals[3];
        listenerOrientation->setQuaternion(quat);
      }

    } else if(add.matches("/quaternions")) {
      // Messages understood by Unity plugin
      if(listenerOrientation) {
        ListenerOrientation::Quaternion quat;
        quat.w = vals[0];
        quat.x = -vals[1];
        quat.z = -vals[2];
        quat.y = -vals[3];
        listenerOrientation->setQuaternion(quat);
      }

    } else {
      return;
    }

  } else if(vals.size() == 7) {
    if(add.matches("/head_pose")) {
      // Messages understood by Ambix
      oscEulerInput.p = vals[4];
      oscEulerInput.y = vals[5];
      oscEulerInput.r = vals[6];
      oscEulerInput.order = ListenerOrientation::EulerOrder::PYR;
      if(listenerOrientation) listenerOrientation->setEuler(oscEulerInput);

    } else {
      return;
    }
  }

  updateStatusText(std::string("OSC Receiving..."));
  startTimer(500);
}

void ListenerOrientationOscReceiver::timerCallback()
{
  // "Receiving..." timer done
  stopTimer();
  updateStatusText(std::string(isListening ? "OSC Ready." : "OSC Closed."));
}

void ListenerOrientationOscReceiver::updateStatusText(std::string & newStatus)
{
  curStatusText = newStatus;
  updateStatusText();
}

void ListenerOrientationOscReceiver::updateStatusText()
{
  if(statusTextCallback) {
    statusTextCallback(curStatusText);
  }
}

ListenerOrientation::ListenerOrientation()
{
}

ListenerOrientation::~ListenerOrientation()
{
}

ListenerOrientation::Euler ListenerOrientation::getEuler()
{
  if(!eulerOutput.has_value()) {
    if(lastQuatInput.has_value()) {
      eulerOutput = toEuler(lastQuatInput.value(), YPR);
    } else {
      eulerOutput = Euler{ 0.0, 0.0, 0.0, YPR };
    }
  }
  return eulerOutput.value();
}

void ListenerOrientation::setEuler(Euler e)
{
  lastQuatInput.reset();
  if(lastEulerInput.has_value()) {
    auto& lE = lastEulerInput.value();
    if(lE.y == e.y && lE.p == e.p && lE.r == e.r && lE.order == e.order) return;
  }
  lastEulerInput = e;
  eulerOutput = e;
  // Other output need reconvert
  quatOutput.reset();
  if(coordinateUpdateCallback) coordinateUpdateCallback();
}

ListenerOrientation::Quaternion ListenerOrientation::getQuaternion()
{
  if(!quatOutput.has_value()) {
    if(lastEulerInput.has_value()) {
      quatOutput = toQuaternion(lastEulerInput.value());
    } else {
      quatOutput = toQuaternion(Euler{ 0.0, 0.0, 0.0, YPR });
    }
  }
  return quatOutput.value();
}

void ListenerOrientation::setQuaternion(Quaternion q)
{
  lastEulerInput.reset();
  if(lastQuatInput.has_value()) {
    auto& lQ = lastQuatInput.value();
    if(lQ.w == q.w && lQ.x == q.x && lQ.y == q.y && lQ.z == q.z) return;
  }
  lastQuatInput = q;
  quatOutput = q;
  // Other output need reconvert
  eulerOutput.reset();
  if(coordinateUpdateCallback) coordinateUpdateCallback();

  //TODO - remove once OSC work done
#ifdef _WIN32
  auto latestEuler = getEuler();
  std::string msg{ "Euler Y P R: " };
  msg += std::to_string(latestEuler.y) + "   ";
  msg += std::to_string(latestEuler.p) + "   ";
  msg += std::to_string(latestEuler.r) + "\n";
  OutputDebugString(msg.c_str());
#endif
}

void ListenerOrientation::setCoordinateUpdateHandler(std::function<void()> callback)
{
  coordinateUpdateCallback = callback;
}

ListenerOrientation::Euler ListenerOrientation::toEuler(Quaternion q, EulerOrder order)
{
  double eRadX, eRadY, eRadZ;

  auto x2 = q.x + q.x, y2 = q.y + q.y, z2 = q.z + q.z;
  auto xx = q.x * x2, xy = q.x * y2, xz = q.x * z2;
  auto yy = q.y * y2, yz = q.y * z2, zz = q.z * z2;
  auto wx = q.w * x2, wy = q.w * y2, wz = q.w * z2;

  double m11 = (1.0 - (yy + zz));
  double m12 = (xy - wz);
  double m13 = (xz + wy);
  double m21 = (xy + wz);
  double m22 = (1 - (xx + zz));
  double m23 = (yz - wx);
  double m31 = (xz - wy);
  double m32 = (yz + wx);
  double m33 = (1.0 - (xx + yy));

  switch(order) {

    case RPY: //XYZ:
      eRadY = asin(clamp(m13, -1.0, 1.0));
      if(abs(m13) < 0.9999999) {
        eRadX = atan2(-m23, m33);
        eRadZ = atan2(-m12, m11);
      } else {
        eRadX = atan2(m32, m22);
        eRadZ = 0.0;
      }
      break;

    case PRY: //YXZ:
      eRadX = asin(-clamp(m23, -1.0, 1.0));
      if(abs(m23) < 0.9999999) {
        eRadY = atan2(m13, m33);
        eRadZ = atan2(m21, m22);
      } else {
        eRadY = atan2(-m31, m11);
        eRadZ = 0.0;
      }
      break;

    case YRP: //ZXY:
      eRadX = asin(clamp(m32, -1.0, 1.0));
      if(abs(m32) < 0.9999999) {
        eRadY = atan2(-m31, m33);
        eRadZ = atan2(-m12, m22);
      } else {
        eRadY = 0.0;
        eRadZ = atan2(m21, m11);
      }
      break;

    case YPR: //ZYX:
      eRadY = asin(-clamp(m31, -1.0, 1.0));
      if(abs(m31) < 0.9999999) {
        eRadX = atan2(m32, m33);
        eRadZ = atan2(m21, m11);
      } else {
        eRadX = 0.0;
        eRadZ = atan2(-m12, m22);
      }
      break;

    case PYR: //YZX:
      eRadZ = asin(clamp(m21, -1.0, 1.0));
      if(abs(m21) < 0.9999999) {
        eRadX = atan2(-m23, m22);
        eRadY = atan2(-m31, m11);
      } else {
        eRadX = 0.0;
        eRadY = atan2(m13, m33);
      }
      break;

    case RYP: //XZY:
      eRadZ = asin(-clamp(m12, -1.0, 1.0));
      if(abs(m12) < 0.9999999) {
        eRadX = atan2(m32, m22);
        eRadY = atan2(m13, m11);
      } else {
        eRadX = atan2(-m23, m33);
        eRadY = 0.0;
      }
      break;

    default:
      throw std::runtime_error("setFromRotationMatrix() encountered an unknown order");
  }

  return Euler{ radiansToDegrees(eRadZ), radiansToDegrees(eRadY), radiansToDegrees(eRadX), order };
}

ListenerOrientation::Quaternion ListenerOrientation::toQuaternion(Euler e)
{
  Quaternion q;

  double eRadX = degreesToRadians(e.r);
  double eRadY = degreesToRadians(e.p);
  double eRadZ = degreesToRadians(e.y);

  double c1 = cos(eRadX / 2.0);
  double c2 = cos(eRadY / 2.0);
  double c3 = cos(eRadZ / 2.0);

  double s1 = sin(eRadX / 2.0);
  double s2 = sin(eRadY / 2.0);
  double s3 = sin(eRadZ / 2.0);

  switch(e.order) {

    case RPY: //XYZ:
      q.x = s1 * c2 * c3 + c1 * s2 * s3;
      q.y = c1 * s2 * c3 - s1 * c2 * s3;
      q.z = c1 * c2 * s3 + s1 * s2 * c3;
      q.w = c1 * c2 * c3 - s1 * s2 * s3;
      break;

    case PRY: //YXZ:
      q.x = s1 * c2 * c3 + c1 * s2 * s3;
      q.y = c1 * s2 * c3 - s1 * c2 * s3;
      q.z = c1 * c2 * s3 - s1 * s2 * c3;
      q.w = c1 * c2 * c3 + s1 * s2 * s3;
      break;

    case YRP: //ZXY:
      q.x = s1 * c2 * c3 - c1 * s2 * s3;
      q.y = c1 * s2 * c3 + s1 * c2 * s3;
      q.z = c1 * c2 * s3 + s1 * s2 * c3;
      q.w = c1 * c2 * c3 - s1 * s2 * s3;
      break;

    case YPR: //ZYX:
      q.x = s1 * c2 * c3 - c1 * s2 * s3;
      q.y = c1 * s2 * c3 + s1 * c2 * s3;
      q.z = c1 * c2 * s3 - s1 * s2 * c3;
      q.w = c1 * c2 * c3 + s1 * s2 * s3;
      break;

    case PYR: //YZX:
      q.x = s1 * c2 * c3 + c1 * s2 * s3;
      q.y = c1 * s2 * c3 + s1 * c2 * s3;
      q.z = c1 * c2 * s3 - s1 * s2 * c3;
      q.w = c1 * c2 * c3 - s1 * s2 * s3;
      break;

    case RYP: //XZY:
      q.x = s1 * c2 * c3 - c1 * s2 * s3;
      q.y = c1 * s2 * c3 - s1 * c2 * s3;
      q.z = c1 * c2 * s3 + s1 * s2 * c3;
      q.w = c1 * c2 * c3 + s1 * s2 * s3;
      break;

    default:
      throw std::runtime_error("setFromEuler() encountered an unknown order");
  }

  return q;
}
