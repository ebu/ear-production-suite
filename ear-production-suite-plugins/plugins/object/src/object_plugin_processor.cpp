#include "object_plugin_processor.hpp"

#include "components/non_automatable_parameter.hpp"
#include "object_backend.hpp"
#include "object_frontend_connector.hpp"
#include "object_plugin_editor.hpp"
#include <plugin_info.hpp>

void registerPluginLoadSig(std::function<void(std::string const&)>);
uint32_t requestInputInstanceIdSig();

using namespace ear::plugin;

ObjectsAudioProcessor::ObjectsAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", AudioChannelSet::mono(), true)
                         .withOutput("Output", AudioChannelSet::mono(), true)),
      samplerate_(48000),
      levelMeter_(std::make_shared<LevelMeterCalculator>(1, samplerate_)) {

  /* clang-format off */
  auto pi = ObjectPluginInfo::pluginInfo();
  assert(pi.parameterCount() == 17); // Simple check that we've implemented everything
  // Ensure parameters are added in intended order
  for(int i = 0; i < pi.parameterCount(); ++i) {
    auto pn = pi.parameter(i)->name();
    if(pn == "ROUTING") {
      addParameter(routing_ = new ui::NonAutomatedParameter<AudioParameterInt>(
        pi.routing()->name(), pi.routing()->niceName(),
        pi.routing()->minVal(), pi.routing()->maxVal(), pi.routing()->defVal()));
    } else if(pn == "GAIN") {
      addParameter(gain_ = new AudioParameterFloat(
        pi.gain()->name(), pi.gain()->niceName(),
        NormalisableRange<float>{pi.gain()->minVal(), pi.gain()->maxVal()}, pi.gain()->defVal()));
    } else if(pn == "AZIMUTH") {
      addParameter(azimuth_ = new AudioParameterFloat(
        pi.azimuth()->name(), pi.azimuth()->niceName(),
        NormalisableRange<float>{pi.azimuth()->minVal(), pi.azimuth()->maxVal()}, pi.azimuth()->defVal()));
    } else if(pn == "ELEVATION") {
      addParameter(elevation_ = new AudioParameterFloat(
        pi.elevation()->name(), pi.elevation()->niceName(),
        NormalisableRange<float>{pi.elevation()->minVal(), pi.elevation()->maxVal()}, pi.elevation()->defVal()));
    } else if(pn == "DISTANCE") {
      addParameter(distance_ = new AudioParameterFloat(
        pi.distance()->name(), pi.distance()->niceName(),
        NormalisableRange<float>{pi.distance()->minVal(), pi.distance()->maxVal()}, pi.distance()->defVal()));
    } else if(pn == "LINKSIZE") {
      addParameter(linkSize_ = new AudioParameterBool(
        pi.linkSize()->name(), pi.linkSize()->niceName(), pi.linkSize()->defVal()));
    } else if(pn == "SIZE") {
      addParameter(size_ = new AudioParameterFloat(
        pi.size()->name(), pi.size()->niceName(),
        pi.size()->minVal(), pi.size()->maxVal(), pi.size()->defVal()));
    } else if(pn == "WIDTH") {
      addParameter(width_ = new AudioParameterFloat(
        pi.width()->name(), pi.width()->niceName(),
        pi.width()->minVal(), pi.width()->maxVal(), pi.width()->defVal()));
    } else if(pn == "HEIGHT") {
      addParameter(height_ = new AudioParameterFloat(
        pi.height()->name(), pi.height()->niceName(),
        pi.height()->minVal(), pi.height()->maxVal(), pi.height()->defVal()));
    } else if(pn == "DEPTH") {
      addParameter(depth_ = new AudioParameterFloat(
        pi.depth()->name(), pi.depth()->niceName(),
        pi.depth()->minVal(), pi.depth()->maxVal(), pi.depth()->defVal()));
    } else if(pn == "DIFFUSE") {
      addParameter(diffuse_ = new AudioParameterFloat(
        pi.diffuse()->name(), pi.diffuse()->niceName(),
        pi.diffuse()->minVal(), pi.diffuse()->maxVal(), pi.diffuse()->defVal()));
    } else if(pn == "DIVERGENCE") {
      addParameter(divergence_ = new AudioParameterBool(
        pi.divergence()->name(), pi.divergence()->niceName(), pi.divergence()->defVal()));
    } else if(pn == "FACTOR") {
      addParameter(factor_ = new AudioParameterFloat(
        pi.factor()->name(), pi.factor()->niceName(),
        pi.factor()->minVal(), pi.factor()->maxVal(), pi.factor()->defVal()));
    } else if(pn == "RANGE") {
      addParameter(range_ = new AudioParameterFloat(
        pi.range()->name(), pi.range()->niceName(),
        pi.range()->minVal(), pi.range()->maxVal(), pi.range()->defVal()));
    } else if(pn == "BYPASS") {
      addParameter(bypass_ = new AudioParameterBool(
        pi.bypass()->name(), pi.bypass()->niceName(), pi.bypass()->defVal()));
    } else if(pn == "USETRACKNAME") {
      addParameter(useTrackName_ = new AudioParameterBool(
        pi.useTrackName()->name(), pi.useTrackName()->niceName(), pi.useTrackName()->defVal()));
    } else if(pn == "INPUTINSTANCEID") {
      addParameter(inputInstanceId_ = new ReadOnlyAudioParameterInt(
        pi.inputInstanceId()->name(), pi.inputInstanceId()->niceName(),
        pi.inputInstanceId()->minVal(), pi.inputInstanceId()->maxVal(), pi.inputInstanceId()->defVal()));
    } else {
      assert(false); // Unimplemented parameter
    }
  }
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
  connector_->parameterValueChanged(14, useTrackName_->get());
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
  xml->setAttribute("use_track_name", (bool)*useTrackName_);
  xml->setAttribute("name", connector_->getActiveName());
  copyXmlToBinary(*xml, destData);
}

void ObjectsAudioProcessor::setStateInformation(const void* data,
                                                int sizeInBytes) {
  std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
  if(xmlState) setStateInformation(xmlState.get());
}

void ObjectsAudioProcessor::setStateInformation(XmlElement * xmlState, bool useDefaultsIfUnspecified)
{
  if (xmlState->hasTagName("ObjectsPlugin")) {
    if(useDefaultsIfUnspecified || xmlState->hasAttribute("connection_id")) {
      connectionId_ = communication::ConnectionId{
        xmlState
        ->getStringAttribute("connection_id",
                             "00000000-0000-0000-0000-000000000000")
        .toStdString() };
      backend_->setConnectionId(connectionId_);
    }

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("routing"))
      *routing_ = xmlState->getIntAttribute("routing", -1);

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("gain"))
      *gain_ = xmlState->getDoubleAttribute("gain", 0.0);

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("azimuth"))
      *azimuth_ = xmlState->getDoubleAttribute("azimuth", 0.0);

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("elevation"))
      *elevation_ = xmlState->getDoubleAttribute("elevation", 0.0);

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("distance"))
      *distance_ = xmlState->getDoubleAttribute("distance", 1.0);

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("link_size"))
      *linkSize_ = xmlState->getBoolAttribute("link_size", true);

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("size"))
      *size_ = xmlState->getDoubleAttribute("size", 0.0);

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("width"))
      *width_ = xmlState->getDoubleAttribute("width", 0.0);

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("height"))
      *height_ = xmlState->getDoubleAttribute("height", 0.0);

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("depth"))
      *depth_ = xmlState->getDoubleAttribute("depth", 0.0);

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("diffuse"))
      *diffuse_ = xmlState->getDoubleAttribute("diffuse", 0.0);

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("divergence"))
      *divergence_ = xmlState->getBoolAttribute("divergence", false);

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("factor"))
      *factor_ = xmlState->getDoubleAttribute("factor", 0.0);

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("range"))
      *range_ = xmlState->getDoubleAttribute("range", 0.0);

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("use_track_name"))
      *useTrackName_ = xmlState->getBoolAttribute("use_track_name", true);

    if(useDefaultsIfUnspecified || xmlState->hasAttribute("name"))
      connector_->setName(xmlState->getStringAttribute("name", "No Name").toStdString());

    if(xmlState->hasAttribute("imported_ao_id")) {
      backend_->setImportedAudioObjectId(xmlState->getIntAttribute("imported_ao_id", 0));
    }

    if(xmlState->hasAttribute("imported_atu_id")) {
      backend_->setImportedAudioTrackUidId(xmlState->getIntAttribute("imported_atu_id", 0));
    }
  }
}

void ObjectsAudioProcessor::updateTrackProperties(
    const TrackProperties& properties) {
  connector_->trackPropertiesChanged(properties);
}

void ObjectsAudioProcessor::setIHostApplication(Steinberg::FUnknown * unknown)
{
  reaperHost = dynamic_cast<IReaperHostApplication*>(unknown);
  VST3ClientExtensions::setIHostApplication(unknown);

  auto requestInputInstanceIdPtr = reaperHost->getReaperApi("requestInputInstanceId");
  if(requestInputInstanceIdPtr) {
    auto requestInputInstanceId = reinterpret_cast<decltype(&requestInputInstanceIdSig)>(requestInputInstanceIdPtr);
    uint32_t inputInstanceId = requestInputInstanceId();
    inputInstanceId_->internalSetIntAndNotifyHost(inputInstanceId);
  }

  auto registerPluginLoadPtr = reaperHost->getReaperApi("registerPluginLoad");
  if(registerPluginLoadPtr) {
    auto registerPluginLoad = reinterpret_cast<decltype(&registerPluginLoadSig)>(registerPluginLoadPtr);
    registerPluginLoad([this](std::string const& xmlState) {
      this->extensionSetState(xmlState);
    });
  }
}

void ObjectsAudioProcessor::extensionSetState(std::string const & xmlStateStr)
{
  auto doc = XmlDocument(xmlStateStr);
  auto xmlState = doc.getDocumentElement();
  setStateInformation(xmlState.get(), false);
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new ObjectsAudioProcessor();
}
