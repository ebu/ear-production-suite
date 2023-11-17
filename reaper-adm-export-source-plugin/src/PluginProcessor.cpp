#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <fstream>
#include <daw_channel_count.h>

AdmStemPluginAudioProcessor::AdmStemPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
  : AudioProcessor (BusesProperties()
         .withInput("Input", AudioChannelSet::discreteChannels(MAX_DAW_CHANNELS), true)
         .withOutput("Output", AudioChannelSet::discreteChannels(MAX_DAW_CHANNELS), true)
     )
#endif
{
    CRT_SET
    sendSamples = false;

    // ORDERING OF addParameter IS IMPORTANT! IT SETS THE PARAMETER INDEX, WHICH IS USED DIRECTLY BY reaper_adm
    // DO NOT CHANGE THE CALL ORDER FOR EXISTING PARAMETERS. ONLY APPEND NEW PARAMETERS TO THE END.


    samplesSocket = new SamplesSender();
    samplesSocket->open();
    samplesSocket->listen();
    commandSocket = new CommandReceiver(std::bind(&AdmStemPluginAudioProcessor::incomingMessage, this, std::placeholders::_1));
    commandSocket->open();
    commandSocket->listen();

    addParameter(commandPort = new ReadOnlyAudioParameterInt("commandPort", // parameter ID
                                                             "Port that this plug-in is listening on for a COMMAND connection", // parameter name
                                                             0, 65535, commandSocket->getPort())); // range and default value

    addParameter(samplesPort = new ReadOnlyAudioParameterInt("samplesPort", // parameter ID
                                                             "Port that this plug-in is listening on for a SAMPLES connection", // parameter name
                                                             0, 65535, samplesSocket->getPort())); // range and default value

    addParameter(includeInAdmRender = new NonAutoAudioParameterBool("includeInAdmRender", // parameter ID
                                                                    "Include In ADM Render", // parameter name
                                                                    false)); // default value

    addParameter(sampleRateParam = new ReadOnlyAudioParameterInt("sampleRate", // parameter ID
                                                                 "Sample Rate detected from host", // parameter name
                                                                 0, 192000, 0)); // range and default value

    addParameter(numChnsParam = new ReadOnlyAudioParameterInt("numChns", // parameter ID
                                                              "Number of Channels defined by ADM essence type", // parameter name
                                                              0, MAX_DAW_CHANNELS, 0)); // range and default value

    addParameter(admTypeDefinitionParam = new NonAutoAudioParameterInt("admTypeDefinition", // parameter ID
                                                                       "ADM essence type (typeDefinition)", // parameter name
                                                                       0, 0xFFFF, 0x0000)); // range and default value

    addParameter(admPackFormatParam = new NonAutoAudioParameterInt("admPackFormat", // parameter ID
                                                                   "ADM pack format (audioPackFormatId)", // parameter name
                                                                   0, 0xFFFF, 0x0000)); // range and default value


    addParameter(admChannelFormatParam = new NonAutoAudioParameterInt("admChannelFormat", // parameter ID
                                                                      "ADM channel (audioChannelFormatId)", // parameter name
                                                                      0, 0xFFFF, 0x0000)); // range and default value

    includeInAdmRenderListener = new ParamChangeCallback(std::bind(&AdmStemPluginAudioProcessor::includeInAdmRenderValueChanged, this));
    includeInAdmRender->addListener(includeInAdmRenderListener);

    admTypeDefinitionListener = new ParamChangeCallback(std::bind(&AdmStemPluginAudioProcessor::admTypeDefinitionChanged, this));
    admTypeDefinitionParam->addListener(admTypeDefinitionListener);

    admPackFormatListener = new ParamChangeCallback(std::bind(&AdmStemPluginAudioProcessor::admPackFormatChanged, this));
    admPackFormatParam->addListener(admPackFormatListener);

    admChannelFormatListener = new ParamChangeCallback(std::bind(&AdmStemPluginAudioProcessor::admChannelFormatChanged, this));
    admChannelFormatParam->addListener(admChannelFormatListener);

    updateNumChnsParam(true);
}

AdmStemPluginAudioProcessor::~AdmStemPluginAudioProcessor()
{
    renderHasFinished(); // Just incase the plugin was somehow removed whilst rendering!

    // Tear down what we don't need anymore
    includeInAdmRender->removeListener(includeInAdmRenderListener);
    admTypeDefinitionParam->removeListener(admTypeDefinitionListener);
    admPackFormatParam->removeListener(admPackFormatListener);
    admChannelFormatParam->removeListener(admChannelFormatListener);
    delete includeInAdmRenderListener;
    delete admTypeDefinitionListener;
    delete admPackFormatListener;
    delete admChannelFormatListener;
    // Note that we do not need to delete Parameters (those added with addParameter) - Juce takes care of them!

    commandSocket->close();
    delete commandSocket;
    samplesSocket->close();
    delete samplesSocket;
}


//////////////////// NNG MESSAGES ////////////////////


void AdmStemPluginAudioProcessor::incomingMessage(std::shared_ptr<NngMsg> msg) {
    if (msg->getSize() == sizeof(uint8_t)) {
        uint8_t cmd;
        memcpy(&cmd, msg->getBufferPointer(), sizeof(uint8_t));

        if(cmd == commandSocket->Command::StartRender){
            OutputDebugString("\nincomingMessage - StartRender");
            renderIsStarting();
        } else if(cmd == commandSocket->Command::StopRender){
            OutputDebugString("\nincomingMessage - StopRender");
            renderHasFinished();
        }
        else if (cmd == commandSocket->Command::GetConfig) {
            OutputDebugString("\nincomingMessage - GetConfig");
        }
        uint8_t numChannels = numChnsParam->get();
        uint32_t sampleRate = sampleRateParam->get();
        commandSocket->sendInfo(numChannels, sampleRate, (uint16_t)admTypeDefinitionParam->get(), (uint16_t)admPackFormatParam->get(), (uint16_t)admChannelFormatParam->get());
    }

}


//////////////////// PARAMETER CHANGE LISTENERS ////////////////////


void AdmStemPluginAudioProcessor::setTypeDefinition(int val)
{
    desiredTypeDefinition = val;

    if(admTypeDefinitionParam->get() != desiredTypeDefinition){
        desiredPackFormat = PACKFORMAT_UNSET_ID;
        desiredChannelFormat = CHANNELFORMAT_ALLCHANNELS_ID;
    }

    updateAdmParameters();
}

void AdmStemPluginAudioProcessor::setPackFormat(int val)
{
    desiredPackFormat = val;
    updateAdmParameters();
}

void AdmStemPluginAudioProcessor::setChannelFormat(int val)
{
    desiredChannelFormat = val;
    updateAdmParameters();
}

void AdmStemPluginAudioProcessor::includeInAdmRenderValueChanged() {
    auto e = editor();
    if(e) e->syncEditorToProcessor();
}

void AdmStemPluginAudioProcessor::admTypeDefinitionChanged()
{
    auto prevDesiredPackFormat = desiredPackFormat;
    auto prevDesiredChannelFormat = desiredChannelFormat;

    if(desiredTypeDefinition != admTypeDefinitionParam->get()) {
        desiredTypeDefinition = admTypeDefinitionParam->get();
        desiredPackFormat = PACKFORMAT_UNSET_ID;
        desiredChannelFormat = CHANNELFORMAT_ALLCHANNELS_ID;
    }

    updateAdmParameters();
    updateNumChnsParam();

    if(admTypeDefinitionParam->wasChangeInvokedInternally() &&
       prevDesiredPackFormat == desiredPackFormat &&
       prevDesiredChannelFormat == desiredChannelFormat) {
        return;
    }

    auto e = editor();
    if(e) e->syncEditorToProcessor();
}

void AdmStemPluginAudioProcessor::admPackFormatChanged()
{
    desiredPackFormat = admPackFormatParam->get();
    updateAdmParameters();
    updateNumChnsParam();
    if(admPackFormatParam->wasChangeInvokedInternally()) return;
    auto e = editor();
    if(e) e->syncEditorToProcessor();
}

void AdmStemPluginAudioProcessor::admChannelFormatChanged()
{
    desiredChannelFormat = admChannelFormatParam->get();
    updateAdmParameters();
    updateNumChnsParam();
    if(admChannelFormatParam->wasChangeInvokedInternally()) return;
    auto e = editor();
    if(e) e->syncEditorToProcessor();
}

//////////////////// HELPERS ////////////////////

int AdmStemPluginAudioProcessor::calcNumChannels()
{
    auto selTd = admTypeDefinitionParam->get();

    if(selTd == adm::TypeDefinition::UNDEFINED.get())  return 0;
    if(selTd == adm::TypeDefinition::OBJECTS.get())  return 1;

    auto selPf = admPackFormatParam->get();
    auto selCf = admChannelFormatParam->get();

    auto typeDefinitionData = admPresetDefinitions.getTypeDefinitionData(selTd);

    auto packFormatData = admPresetDefinitions.getPackFormatData(selTd, selPf);
    if(!packFormatData) return 0;

    if(selCf == CHANNELFORMAT_ALLCHANNELS_ID) return packFormatData->relatedChannelFormats.size();

    auto channelFormatData = admPresetDefinitions.getChannelFormatData(selTd, selPf, selCf);
    if(!channelFormatData) return 0;

    return 1;
}

void AdmStemPluginAudioProcessor::updateNumChnsParam(bool force)
{
    auto newNumChns = calcNumChannels();
    if(force || numChnsParam->get() != newNumChns) {
        numChnsParam->internalSetIntAndNotifyHost(0);
        numChnsParam->internalSetIntAndNotifyHost(newNumChns);
    }
}

void AdmStemPluginAudioProcessor::updateAdmParameters(bool force)
{
    // These are the values we will actually set the params to. Set defaults for now;
    int valTypeDefinition = desiredTypeDefinition;
    int valPackFormat = PACKFORMAT_UNSET_ID;
    int valChannelFormat = CHANNELFORMAT_ALLCHANNELS_ID;

    if(admPresetDefinitions.getPackFormatData(valTypeDefinition, desiredPackFormat)) {
        valPackFormat = desiredPackFormat;
    }

    if(admPresetDefinitions.getChannelFormatData(valTypeDefinition, desiredPackFormat, desiredChannelFormat)) {
        valChannelFormat = desiredChannelFormat;
    }

    // Now do the setting of params

#ifdef DEBUG_PARAMS
    OutputDebugString("\n[updateAdmParameters] force = ");
    OutputDebugString(force ? "TRUE" : "false");
    OutputDebugString("\n                    desired = ");
    OutputDebugString(std::to_string(desiredTypeDefinition).c_str());
    OutputDebugString(" ");
    OutputDebugString(std::to_string(desiredPackFormat).c_str());
    OutputDebugString(" ");
    OutputDebugString(std::to_string(desiredChannelFormat).c_str());
    OutputDebugString("\n                     actual = ");
    OutputDebugString(std::to_string(valTypeDefinition).c_str());
    OutputDebugString(" ");
    OutputDebugString(std::to_string(valPackFormat).c_str());
    OutputDebugString(" ");
    OutputDebugString(std::to_string(valChannelFormat).c_str());
#endif

    if(force || admTypeDefinitionParam->get() != valTypeDefinition) {
        admTypeDefinitionParam->internalSetIntAndNotifyHost(valTypeDefinition);
    }

    if(force || admPackFormatParam->get() != valPackFormat) {
        admPackFormatParam->internalSetIntAndNotifyHost(valPackFormat);
    }

    if(force || admChannelFormatParam->get() != valChannelFormat) {
        admChannelFormatParam->internalSetIntAndNotifyHost(valChannelFormat);
    }
}

void AdmStemPluginAudioProcessor::renderIsStarting() {
    OutputDebugString("\n[Render START]");
    sendSamples = includeInAdmRender->get();
}

void AdmStemPluginAudioProcessor::renderHasFinished() {
    OutputDebugString("\n[Render FINISH]");
    sendSamples = false;
}


//////////////////// COMMON VST METHODS ////////////////////


void AdmStemPluginAudioProcessor::prepareToPlay (double vstSampleRate, int samplesPerBlock)
{
    OutputDebugString("\n(prepareToPlay)");
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    sampleRateParam->internalSetIntAndNotifyHost(vstSampleRate);
}

void AdmStemPluginAudioProcessor::releaseResources()
{
    OutputDebugString("\n(releaseResources)");
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void AdmStemPluginAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{

#if TRACK_STATE
    std::string op = "{";
    op += this->isNonRealtime()? "r/":"R/";
    op += this->isSuspended()? "S/":"s/";

    auto tempPh = getPlayHead(); // Not all hosts will return this

    juce::AudioPlayHead::CurrentPositionInfo tempCpi;
    bool success = tempPh->getCurrentPosition(tempCpi);
    if (success) {
        op += std::to_string(tempCpi.timeInSamples);
        op += tempCpi.isPlaying? "/P}": "/S}";
    }
    else {
        op += "-/-}";
    }
    OutputDebugString(op.c_str());
#endif

    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...

    if (sendSamples) {
        size_t sampleSize = sizeof(float);
        uint8_t numChannels = numChnsParam->get();
        size_t msg_size = buffer.getNumSamples() * numChannels * sampleSize; // Samples
        auto msg = std::make_shared<NngMsg>(msg_size);

        char* msgPosPtr = (char*)msg->getBufferPointer();
        int msgPosOffset = 0;
        float curSample = 0.0;

        int bufferChannels = buffer.getNumChannels();
        int bufferSamplesPerChannel = buffer.getNumSamples();

        for(int sample = 0; sample < bufferSamplesPerChannel; ++sample)
        {
            for (int channel = 0; channel < numChannels; ++channel)
            {
                curSample = (channel < totalNumInputChannels)? buffer.getSample(channel, sample) : 0.0;
                assert(msgPosOffset + sampleSize <= msg_size);
                memcpy(msgPosPtr + msgPosOffset, &curSample, sampleSize);
                msgPosOffset += sampleSize;
            }
        }

        auto resSend = samplesSocket->sendBlock(msg, 0);
        assert(resSend == 0);
        if (resSend != 0) {
            // TODO: Log this error somewhere - this means we can't send our samples
            sendSamples = false;
            return;
        }
    }

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AdmStemPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() == AudioChannelSet::discreteChannels(MAX_DAW_CHANNELS) &&
        layouts.getMainInputChannelSet() == AudioChannelSet::discreteChannels(MAX_DAW_CHANNELS)) {
        return true;
    }
    return false;
}
#endif

void AdmStemPluginAudioProcessor::numChannelsChanged(){
    //auto busIn = this->getBus(true, 0)->getCurrentLayout().getDescription();
    auto busChIn = this->getBus(true, 0)->getNumberOfChannels();
    //auto busOut = this->getBus(false, 0)->getCurrentLayout().getDescription();
    auto busChOut = this->getBus(false, 0)->getNumberOfChannels();

    // TODO - do we actually need to do anything here? We used to warn if being fed less than the expected width of the ADM essense - would be a nice feature.
}

AdmStemPluginAudioProcessorEditor* AdmStemPluginAudioProcessor::editor(){
    auto e = getActiveEditor();
    if(e) return reinterpret_cast<AdmStemPluginAudioProcessorEditor*>(e);
    return nullptr;
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    // This creates new instances of the plugin..
    return new AdmStemPluginAudioProcessor();
}

bool AdmStemPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* AdmStemPluginAudioProcessor::createEditor()
{
    return new AdmStemPluginAudioProcessorEditor(*this);
}

void AdmStemPluginAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    MemoryOutputStream stream(destData, true);
    auto boolVal = includeInAdmRender->get();
    stream.writeBool(boolVal);
    stream.writeInt(desiredTypeDefinition);
    stream.writeInt(desiredPackFormat);
    stream.writeInt(desiredChannelFormat);
}

void AdmStemPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    MemoryInputStream stream(data, static_cast<size_t> (sizeInBytes), false);
    auto boolVal = stream.readBool();
    includeInAdmRender->setValueNotifyingHost(boolVal);
    desiredTypeDefinition = stream.readInt();
    desiredPackFormat = stream.readInt();
    desiredChannelFormat = stream.readInt();
    updateAdmParameters();
    updateNumChnsParam();

    auto e = editor();
    if(e) e->syncEditorToProcessor();
}


const String AdmStemPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AdmStemPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AdmStemPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AdmStemPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AdmStemPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AdmStemPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AdmStemPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AdmStemPluginAudioProcessor::setCurrentProgram (int index)
{
}

const String AdmStemPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void AdmStemPluginAudioProcessor::changeProgramName (int index, const String& newName)
{
}
