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

juce::AudioProcessor::BusesProperties
EarMonitoringAudioProcessor::_getBusProperties() {
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
EarMonitoringAudioProcessor::EarMonitoringAudioProcessor()
    : AudioProcessor(_getBusProperties()) {
  backend_ = std::make_unique<ear::plugin::MonitoringBackend>(
      nullptr, ear::getLayout(SPEAKER_LAYOUT), 64);
  levelMeter_ = std::make_shared<ear::plugin::LevelMeterCalculator>(0, 0);
  ProcessorConfig newConfig{getTotalNumInputChannels(),
                            getTotalNumOutputChannels(), 512,
                            ear::getLayout(SPEAKER_LAYOUT)};
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
                            ear::getLayout(SPEAKER_LAYOUT)};
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
  auto layout = ear::getLayout(SPEAKER_LAYOUT);
  if (layouts.getMainOutputChannelSet() ==
          AudioChannelSet::discreteChannels(layout.channelNames().size()) &&
      layouts.getMainInputChannelSet() ==
          AudioChannelSet::discreteChannels(64)) {
    return true;
  }

  return false;
}

void EarMonitoringAudioProcessor::processBlock(AudioBuffer<float>& buffer,
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

void EarMonitoringAudioProcessor::speakerSetupChanged(std::string layout) {}

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
