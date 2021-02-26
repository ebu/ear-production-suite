#include "binaural_monitoring_plugin_processor.hpp"
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
};

}  // namespace plugin
}  // namespace ear

#include <ear/bs2051.hpp>
#include "monitoring_audio_processor.hpp"
#include "monitoring_backend.hpp"

juce::AudioProcessor::BusesProperties
EarBinauralMonitoringAudioProcessor::_getBusProperties() {
  auto layout = ear::getLayout(SPEAKER_LAYOUT);
  channels_ = layout.channelNames().size();
  auto ret = BusesProperties().withInput(
      "Input", AudioChannelSet::discreteChannels(64), true);
  for (const std::string& name : layout.channelNames()) {
    ret = ret.withOutput(name, AudioChannelSet::mono(), true);
  }
  return ret;
}

//==============================================================================
EarBinauralMonitoringAudioProcessor::EarBinauralMonitoringAudioProcessor()
    : AudioProcessor(_getBusProperties()) {
  backend_ = std::make_unique<ear::plugin::MonitoringBackend>(
      nullptr, ear::getLayout(SPEAKER_LAYOUT), 64);
  levelMeter_ = std::make_shared<ear::plugin::LevelMeterCalculator>(0, 0);
  ProcessorConfig newConfig{getTotalNumInputChannels(),
                            getTotalNumOutputChannels(), 512,
                            ear::getLayout(SPEAKER_LAYOUT)};
  configureProcessor(newConfig);
}

EarBinauralMonitoringAudioProcessor::~EarBinauralMonitoringAudioProcessor() {}

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
  ProcessorConfig newConfig{getTotalNumInputChannels(),
                            getTotalNumOutputChannels(), samplesPerBlock,
                            ear::getLayout(SPEAKER_LAYOUT)};
  configureProcessor(newConfig);
  samplerate_ = sampleRate;
  levelMeter_->setup(newConfig.layout.channels().size(), sampleRate);
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
  auto gains = backend_->currentGains();

  // Make sure to reset the state if your inner loop is processing
  // the samples and the outer loop is handling the channels.
  // Alternatively, you can process the samples with the channels
  // interleaved by keeping the same state.
  if (processor_) {
    processor_->process(buffer, buffer, gains.direct, gains.diffuse);
    // Check enough channels - prevents crash if track/buffer too narrow
    if (buffer.getNumChannels() >= levelMeter_->channels()) {
      levelMeter_->process(buffer);
    }
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

void EarBinauralMonitoringAudioProcessor::configureProcessor(
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
  return new EarBinauralMonitoringAudioProcessor();
}
