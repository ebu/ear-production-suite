#include "scene_plugin_processor.hpp"
#include <sstream>

#include "scene_plugin_editor.hpp"
#include <adm/write.hpp>
#include <adm/parse.hpp>
#include "programme_store_adm_serializer.hpp"
#include "programme_store_adm_populator.hpp"
#include <future>
#include <programme_store.pb.h>
#include "scene_backend.hpp"
#include "scene_frontend_connector.hpp"
#include "metadata_event_dispatcher.hpp"

SceneAudioProcessor::SceneAudioProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", AudioChannelSet::discreteChannels(64), true)
              .withOutput("Output", AudioChannelSet::discreteChannels(64),
                          true)),
      metadata_(std::make_unique<ear::plugin::ui::UIEventDispatcher>(),
                std::make_unique<ear::plugin::MetadataEventDispatcher>(metadataThread_))
{
  connector_ = std::make_shared<ear::plugin::ui::JuceSceneFrontendConnector>(this);
  metadata_.addUIListener(connector_);
  backend_ = std::make_unique<ear::plugin::SceneBackend>(metadata_);

  try {
    backend_->setup();
  } catch (const std::exception& e) {
    if (connector_) {
      backendSetupTimer_ =
          std::make_unique<ear::plugin::BackendSetupTimer>(this);
      backendSetupTimer_->startTimer(1000);
    }
  }

  levelMeter_ =
      std::make_shared<ear::plugin::LevelMeterCalculator>(64, samplerate_);

  samplesSocket = new SamplesSender();
  samplesSocket->open();
  samplesSocket->listen();
  commandSocket = new CommandReceiver(std::bind(
      &SceneAudioProcessor::incomingMessage, this, std::placeholders::_1));
  commandSocket->open();
  commandSocket->listen();

  addParameter(commandPort = new ear::plugin::ui::ReadOnlyAudioParameterInt(
                   "commandPort",  // parameter ID
                   "Port that this plug-in is listening on for a COMMAND "
                   "connection",  // parameter name
                   0, 65535,
                   commandSocket->getPort()));  // range and default value

  addParameter(samplesPort = new ear::plugin::ui::ReadOnlyAudioParameterInt(
                   "samplesPort",  // parameter ID
                   "Port that this plug-in is listening on for a SAMPLES "
                   "connection",  // parameter name
                   0, 65535,
                   samplesSocket->getPort()));  // range and default value
}

SceneAudioProcessor::~SceneAudioProcessor() {
  sendSamplesToExtension = false;
  samplesSocket->close();
  delete samplesSocket;
}

const String SceneAudioProcessor::getName() const { return JucePlugin_Name; }

bool SceneAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool SceneAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool SceneAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double SceneAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int SceneAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int SceneAudioProcessor::getCurrentProgram() { return 0; }

void SceneAudioProcessor::setCurrentProgram(int index) {}

const String SceneAudioProcessor::getProgramName(int index) { return {}; }

void SceneAudioProcessor::changeProgramName(int index, const String& newName) {}

void SceneAudioProcessor::prepareToPlay(double sampleRate,
                                        int samplesPerBlock) {
  doSampleRateChecks();
}

void SceneAudioProcessor::releaseResources() {}

bool SceneAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
  if (layouts.getMainOutputChannelSet() ==
          AudioChannelSet::discreteChannels(64) &&
      layouts.getMainInputChannelSet() ==
          AudioChannelSet::discreteChannels(64)) {
    return true;
  }
  return false;
}

void SceneAudioProcessor::processBlock(AudioBuffer<float>& buffer,
                                       MidiBuffer& midiMessages) {
  auto sendSamples = sendSamplesToExtension;
  metadataThread_.post([this, sendSamples](){
    backend_->triggerMetadataSend(sendSamples);
  });
  doSampleRateChecks();

  if(!sendSamplesToExtension) {
    levelMeter_->process(buffer);
  } else {
    size_t sampleSize = sizeof(float);
    uint8_t numChannels = 64;
    size_t msg_size = buffer.getNumSamples() * numChannels * sampleSize;

    auto msg = std::make_shared<NngMsg>(msg_size);

    char* msgPosPtr = (char*)msg->getBufferPointer();
    int msgPosOffset = 0;
    float curSample = 0.0;

    int bufferChannels = buffer.getNumChannels();
    int bufferSamplesPerChannel = buffer.getNumSamples();

    for (int sample = 0; sample < bufferSamplesPerChannel; ++sample) {
      for (int channel = 0; channel < numChannels; ++channel) {
        curSample = buffer.getSample(channel, sample);
        assert(msgPosOffset + sampleSize <= msg_size);
        memcpy(msgPosPtr + msgPosOffset, &curSample, sampleSize);
        msgPosOffset += sampleSize;
      }
    }

    auto resSend = samplesSocket->sendBlock(msg, 0);
    assert(resSend == 0);
    if (resSend != 0) {
      // TODO: Log this error somewhere - this means we can't send our samples
      sendSamplesToExtension = false;
      return;
    }
  }
}

bool SceneAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SceneAudioProcessor::createEditor() {
  return new SceneAudioProcessorEditor(this);
}

void SceneAudioProcessor::getStateInformation(MemoryBlock& destData) {
  auto [programmes, _] = metadata_.stores();
  destData.setSize(programmes.ByteSizeLong());
  programmes.SerializeToArray(destData.getData(), destData.getSize());
}

void SceneAudioProcessor::setStateInformation(const void* data,
                                              int sizeInBytes) {
  ear::plugin::proto::ProgrammeStore store;
  store.ParseFromArray(data, sizeInBytes);
  metadata_.setStore(store);
}

ear::plugin::ui::JuceSceneFrontendConnector*
SceneAudioProcessor::getFrontendConnector() {
  return connector_.get();
}

void SceneAudioProcessor::doSampleRateChecks() {
  auto curSampleRate = static_cast<int>(getSampleRate());
  if (samplerate_ != curSampleRate) {
    samplerate_ = curSampleRate;
    levelMeter_->setup(64, samplerate_);
  }
}

void SceneAudioProcessor::sendAdmMetadata() {
  // This is going to fail if two uids reference the same channel. (the second
  // will overwrite the first) Need to change the way channelMapping is sent if
  // we want to handle that case
  ear::plugin::ProgrammeStoreAdmSerializer serializer{};
  auto [adm, chna] =
      serializer.serialize(metadata_.stores());

  std::stringstream ss;
  adm::writeXml(ss, adm);

  // Use ATU_00000000 for UID's not to be stored in the file, as per BS.2076
  std::vector<uint32_t> channelMapping(64u, 0u);

  for (auto& id : chna.audioIds()) {
    if (id.trackIndex() > 0) {
      assert(id.uid().size() == 12);
      auto idNumString = id.uid().substr(4, 8);
      auto idNum = static_cast<uint32_t>(std::stoul(idNumString, nullptr, 16));
      // chna track indices are 1-based, vector indices are 0-based
      channelMapping.at(id.trackIndex() - 1) = idNum;
    }
  }

  commandSocket->sendAdmAndMappings(ss.str(), std::move(channelMapping));
}

void SceneAudioProcessor::recvAdmMetadata(std::string admStr,
                                          std::vector<uint32_t> mappings) {
  auto iss = std::istringstream{std::move(admStr)};
  auto doc = adm::parseXml(iss, adm::xml::ParserOptions::recursive_node_search);
  pendingElements_ =
      ear::plugin::populateStoreFromAdm(*doc, pendingStore_, mappings);
}

void SceneAudioProcessor::incomingMessage(std::shared_ptr<NngMsg> msg) {
  uint8_t cmd;
  memcpy(&cmd, msg->getBufferPointer(), sizeof(uint8_t));

  if (cmd == commandSocket->Command::StartRender) {
    sendSamplesToExtension = true;
    commandSocket->sendResp(cmd);

  } else if (cmd == commandSocket->Command::StopRender) {
    sendSamplesToExtension = false;
    commandSocket->sendResp(cmd);

  } else if (cmd == commandSocket->Command::GetAdmAndMappings) {
    auto future =
        std::async(std::launch::async, [this]() { sendAdmMetadata(); });
    future.get();

  } else if (cmd == commandSocket->Command::GetConfig) {
    uint8_t numChannels = 64;
    uint32_t sampleRate = samplerate_;
    commandSocket->sendInfo(numChannels, samplerate_);

  } else if (cmd == commandSocket->Command::SetAdmAndMappings) {
    uint32_t mapSz = (64 * 4);
    uint32_t admSz = msg->getSize() - mapSz - 1;
    std::vector<uint32_t> mappings(64, 0x00000000);
    std::string admStr(admSz, 0);
    char* bufPtr = (char*)msg->getBufferPointer();
    int bufOffset = 1;  // Skip first "command" byte
    memcpy(mappings.data(), bufPtr + bufOffset, mapSz);
    bufOffset += mapSz;
    memcpy(admStr.data(), bufPtr + bufOffset, admSz);
    auto future = std::async(std::launch::async, [this, admStr, mappings]() {
      recvAdmMetadata(admStr, mappings);
    });
    future.get();
    commandSocket->sendResp(cmd);

  } else {
    assert(false);  // If we got here, we received an unsupported command. Just
                    // ignore for now by sending a response anyway.
    commandSocket->sendResp(cmd);
  }
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new SceneAudioProcessor();
}

void SceneAudioProcessor::setupBackend() {
  if (backend_) {
    backend_->setup();
  }
}
