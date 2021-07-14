#include "object_plugin_processor.hpp"

#include "../../shared/components/non_automatable_parameter.hpp"
#include "object_backend.hpp"
#include "object_frontend_connector.hpp"
#include "object_plugin_editor.hpp"

using namespace ear::plugin;

ObjectsAudioProcessor::ObjectsAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", AudioChannelSet::mono(), true)
                         .withOutput("Output", AudioChannelSet::mono(), true)),
      samplerate_(48000),
      levelMeter_(std::make_shared<LevelMeterCalculator>(1, samplerate_)) {
  /* clang-format off */
  addParameter(routing_ = new ui::NonAutomatedParameter<AudioParameterInt>("routing", "Routing", -1, 63, -1));
  addParameter(gain_ = new AudioParameterFloat("gain", "Gain", NormalisableRange<float>{-100.f, 6.f}, 0.f));
  addParameter(azimuth_ = new AudioParameterFloat("azimuth", "Azimuth", NormalisableRange<float>{-180.f, 180.f}, 0.f));
  addParameter(elevation_ = new AudioParameterFloat("elevation", "Elevation", NormalisableRange<float>{-90.f, 90.f}, 0.f));
  addParameter(distance_ = new AudioParameterFloat("distance", "Distance", NormalisableRange<float>{0.f, 1.f}, 1.f));
  addParameter(linkSize_ = new AudioParameterBool("linkSize", "Link Size", false));
  addParameter(size_ = new AudioParameterFloat("size", "Size", 0.f, 1.f, 0.f));
  addParameter(width_ = new AudioParameterFloat("width", "Width", 0.f, 1.f, 0.f));
  addParameter(height_ = new AudioParameterFloat("height", "Height", 0.f, 1.f, 0.f));
  addParameter(depth_ = new AudioParameterFloat("depth","Depth", 0.f, 1.f, 0.f));
  addParameter(diffuse_ = new AudioParameterFloat("diffuse", "Diffuse", 0.f, 1.f, 0.f));
  addParameter(divergence_ = new AudioParameterBool("divergence", "Divergence", false));
  addParameter(factor_ = new AudioParameterFloat("factor", "Factor", 0.f, 1.f, 0.f));
  addParameter(range_ = new AudioParameterFloat("range", "Range", 0.f, 180.f, 45.f));
  addParameter(bypass_ = new AudioParameterBool("byps", "Bypass", false));
  /* clang-format on */

  static_cast<ui::NonAutomatedParameter<AudioParameterInt>*>(routing_)->markPluginStateAsDirty = [this]() {
    bypass_->setValueNotifyingHost(bypass_->get());
  };

  connector_ = std::make_unique<ui::ObjectsJuceFrontendConnector>(this);
  backend_ = std::make_unique<ObjectBackend>(connector_.get());

  connector_->parameterValueChanged(0, routing_->get());
  connector_->parameterValueChanged(1, gain_->get());
  connector_->parameterValueChanged(2, azimuth_->get());
  connector_->parameterValueChanged(3, elevation_->get());
  connector_->parameterValueChanged(4, distance_->get());
  connector_->parameterValueChanged(5, linkSize_->get());
  connector_->parameterValueChanged(6, size_->get());
  connector_->parameterValueChanged(7, width_->get());
  connector_->parameterValueChanged(8, height_->get());
  connector_->parameterValueChanged(9, depth_->get());
  connector_->parameterValueChanged(10, diffuse_->get());
  connector_->parameterValueChanged(11, divergence_->get());
  connector_->parameterValueChanged(12, factor_->get());
  connector_->parameterValueChanged(13, range_->get());
}

ObjectsAudioProcessor::~ObjectsAudioProcessor() {}

const String ObjectsAudioProcessor::getName() const { return JucePlugin_Name; }

bool ObjectsAudioProcessor::acceptsMidi() const { return false; }

bool ObjectsAudioProcessor::producesMidi() const { return false; }

bool ObjectsAudioProcessor::isMidiEffect() const { return false; }

double ObjectsAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int ObjectsAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int ObjectsAudioProcessor::getCurrentProgram() { return 0; }

void ObjectsAudioProcessor::setCurrentProgram(int index) {}

const String ObjectsAudioProcessor::getProgramName(int index) { return {}; }

void ObjectsAudioProcessor::changeProgramName(int index,
                                              const String& newName) {}

void ObjectsAudioProcessor::prepareToPlay(double samplerate,
                                          int samplesPerBlock) {
  if (samplerate_ != static_cast<int>(samplerate)) {
    samplerate_ = static_cast<int>(samplerate);
    levelMeter_->setup(1, samplerate_);
  }
}

void ObjectsAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

bool ObjectsAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
  // This is the place where you check if the layout is supported.
  if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()) {
    return false;
  }

  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) {
    return false;
  }
  return true;
}

void ObjectsAudioProcessor::processBlock(AudioBuffer<float>& buffer,
                                         MidiBuffer& midiMessages) {
  if(! bypass_->get()) {
    levelMeter_->process(buffer);
    backend_->triggerMetadataSend();
  }
}

bool ObjectsAudioProcessor::hasEditor() const { return true; }

AudioProcessorEditor* ObjectsAudioProcessor::createEditor() {
  return new ObjectAudioProcessorEditor(this);
}

void ObjectsAudioProcessor::getStateInformation(MemoryBlock& destData) {
  std::unique_ptr<XmlElement> xml(new XmlElement("ObjectsPlugin"));
  connectionId_ = backend_->getConnectionId();
  xml->setAttribute("connection_id", connectionId_.string());
  xml->setAttribute("routing", (int)*routing_);
  xml->setAttribute("gain", (double)*gain_);
  xml->setAttribute("azimuth", (double)*azimuth_);
  xml->setAttribute("elevation", (double)*elevation_);
  xml->setAttribute("distance", (double)*distance_);
  xml->setAttribute("link_size", (bool)*linkSize_);
  xml->setAttribute("size", (double)*size_);
  xml->setAttribute("width", (double)*width_);
  xml->setAttribute("height", (double)*height_);
  xml->setAttribute("depth", (double)*depth_);
  xml->setAttribute("diffuse", (double)*diffuse_);
  xml->setAttribute("divergence", (bool)*divergence_);
  xml->setAttribute("factor", (double)*factor_);
  xml->setAttribute("range", (double)*range_);
  copyXmlToBinary(*xml, destData);
}

void ObjectsAudioProcessor::setStateInformation(const void* data,
                                                int sizeInBytes) {
  std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
  if (xmlState.get() != nullptr) {
    if (xmlState->hasTagName("ObjectsPlugin")) {
      connectionId_ = communication::ConnectionId{
          xmlState
              ->getStringAttribute("connection_id",
                                   "00000000-0000-0000-0000-000000000000")
              .toStdString()};
      backend_->setConnectionId(connectionId_);
      *routing_ = xmlState->getIntAttribute("routing", -1);
      *gain_ = xmlState->getDoubleAttribute("gain", 0.0);
      *azimuth_ = xmlState->getDoubleAttribute("azimuth", 0.0);
      *elevation_ = xmlState->getDoubleAttribute("elevation", 0.0);
      *distance_ = xmlState->getDoubleAttribute("distance", 1.0);
      *linkSize_ = xmlState->getBoolAttribute("link_size", true);
      *size_ = xmlState->getDoubleAttribute("size", 0.0);
      *width_ = xmlState->getDoubleAttribute("width", 0.0);
      *height_ = xmlState->getDoubleAttribute("height", 0.0);
      *depth_ = xmlState->getDoubleAttribute("depth", 0.0);
      *diffuse_ = xmlState->getDoubleAttribute("diffuse", 0.0);
      *divergence_ = xmlState->getBoolAttribute("divergence", false);
      *factor_ = xmlState->getDoubleAttribute("factor", 0.0);
      *range_ = xmlState->getDoubleAttribute("range", 0.0);
    }
  }
}

void ObjectsAudioProcessor::updateTrackProperties(
    const TrackProperties& properties) {
  connector_->trackPropertiesChanged(properties);
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new ObjectsAudioProcessor();
}
