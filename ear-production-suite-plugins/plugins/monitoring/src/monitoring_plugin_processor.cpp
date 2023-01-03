#include "monitoring_plugin_processor.hpp"
#include "monitoring_plugin_editor.hpp"
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
};

}  // namespace plugin
}  // namespace ear

#include <ear/bs2051.hpp>
#include "monitoring_audio_processor.hpp"
#include "monitoring_backend.hpp"

namespace {
  ear::Layout getLayoutImpl(std::string const& layout) {
  if (layout == "2+7+0") {
    auto channels_4_9_0 = ear::getLayout("4+9+0").channels();
    // 2+7+0 has the same first 8 speakers as 4_9_0
    std::vector<ear::Channel> channels_2_7_0(channels_4_9_0.begin(),
                                             channels_4_9_0.begin() + 8);
    auto channels_9_10_3 = ear::getLayout("9+10+3").channels();
    // The last two are U+090 and U-090, which we take from 9_10_3 at 11/12
    channels_2_7_0.push_back(channels_9_10_3.at(12));
    channels_2_7_0.push_back(channels_9_10_3.at(13));
    return {"2+7+0", channels_2_7_0};
  } else {
    return ear::getLayout(layout);
  }
}

ear::Layout const& layout() {
    static ear::Layout LAYOUT{getLayoutImpl(SPEAKER_LAYOUT)};
    return LAYOUT;
  }
}

juce::AudioProcessor::BusesProperties
EarMonitoringAudioProcessor::_getBusProperties() {
  numOutputChannels_ = layout().channelNames().size();
  auto ret = BusesProperties().withInput(
      "Input", AudioChannelSet::discreteChannels(64), true);
  for (const std::string& name : layout().channelNames()) {
    ret = ret.withOutput(name, AudioChannelSet::mono(), true);
  }
  ret = ret.withOutput("(Unused)", AudioChannelSet::discreteChannels(64 - numOutputChannels_), true);
  return ret;
}

//==============================================================================
EarMonitoringAudioProcessor::EarMonitoringAudioProcessor()
    : AudioProcessor(_getBusProperties()) {
  auto speakerLayout = layout();
  backend_ = std::make_unique<ear::plugin::MonitoringBackend>(
      nullptr, speakerLayout, 64);
  levelMeter_ = std::make_shared<ear::plugin::LevelMeterCalculator>(0, 0);
  ProcessorConfig newConfig{getTotalNumInputChannels(),
                            getTotalNumOutputChannels(), 512,
                            speakerLayout};
  configureProcessor(newConfig);
}

EarMonitoringAudioProcessor::~EarMonitoringAudioProcessor() {}

//==============================================================================
const String EarMonitoringAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool EarMonitoringAudioProcessor::acceptsMidi() const { return false; }

bool EarMonitoringAudioProcessor::producesMidi() const { return false; }

bool EarMonitoringAudioProcessor::isMidiEffect() const { return false; }

double EarMonitoringAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int EarMonitoringAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int EarMonitoringAudioProcessor::getCurrentProgram() { return 0; }

void EarMonitoringAudioProcessor::setCurrentProgram(int index) {}

const String EarMonitoringAudioProcessor::getProgramName(int index) {
  return {};
}

void EarMonitoringAudioProcessor::changeProgramName(int index,
                                                    const String& newName) {}

//==============================================================================
void EarMonitoringAudioProcessor::prepareToPlay(double sampleRate,
                                                int samplesPerBlock) {
  ProcessorConfig newConfig{getTotalNumInputChannels(),
                            getTotalNumOutputChannels(), samplesPerBlock,
                            layout()};
  configureProcessor(newConfig);
  samplerate_ = sampleRate;
  levelMeter_->setup(newConfig.layout.channels().size(), sampleRate);
}

void EarMonitoringAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

bool EarMonitoringAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
  if (layouts.getMainOutputChannelSet() ==
          AudioChannelSet::discreteChannels(64) &&
      layouts.getMainInputChannelSet() ==
          AudioChannelSet::discreteChannels(64)) {
    return true;
  }

  return false;
}

void EarMonitoringAudioProcessor::processBlock(AudioBuffer<float>& buffer,
                                               MidiBuffer&) {
  ScopedNoDenormals noDenormals;

  if(backend_->isExporting()) {
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
          buffer.clear(destChn, 0, buffer.getNumSamples());
        }
      }
    }
    return;
  }

  // Do EAR render
  auto gains = backend_->currentGains();
  if (processor_) {
    processor_->process(buffer, buffer, gains.direct, gains.diffuse);
    levelMeter_->process(buffer);
  }

  // Clear unused output channels
  for(int outChn = numOutputChannels_; outChn < buffer.getNumChannels(); ++outChn) {
    buffer.clear(outChn, 0, buffer.getNumSamples());
  }

}

//==============================================================================
bool EarMonitoringAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* EarMonitoringAudioProcessor::createEditor() {
  return new EarMonitoringAudioProcessorEditor(this);
}

//==============================================================================
void EarMonitoringAudioProcessor::getStateInformation(MemoryBlock& destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void EarMonitoringAudioProcessor::setStateInformation(const void* data,
                                                      int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
}

void EarMonitoringAudioProcessor::configureProcessor(
    const ProcessorConfig& config) {
  if (!processor_ || config != processorConfig_) {
    processor_ = std::make_unique<ear::plugin::MonitoringAudioProcessor>(
        config.inputChannels, config.layout, config.blockSize);
    processorConfig_ = config;
  }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new EarMonitoringAudioProcessor();
}
