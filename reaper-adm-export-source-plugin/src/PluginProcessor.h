/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "helper/nng_wrappers.h"
#include "JuceHeader_Wrapper.h"
#include "PluginProcessorUtils.h"
#include <functional>
#include <memory>

#define TRACK_STATE 0
#define DEBUG_PARAMS 0

#if defined(WIN32) && defined(DEBUG)
#include <windows.h>
#else
#define OutputDebugString(...)
#endif

class AdmStemPluginAudioProcessorEditor;

class AdmStemPluginAudioProcessor  : public AudioProcessor
{
public:
    AdmStemPluginAudioProcessor();
    ~AdmStemPluginAudioProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;
    void numChannelsChanged() override;

    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    bool getIncludeInAdmRender() { return includeInAdmRender->get(); }
    void setIncludeInAdmRender(bool state) { includeInAdmRender->setValueNotifyingHost(state? 1.0 : 0.0); }

    int getTypeDefinition() { return desiredTypeDefinition; }
    int getPackFormat() { return desiredPackFormat; }
    int getChannelFormat() { return desiredChannelFormat; }
    void setTypeDefinition(int val);
    void setPackFormat(int val);
    void setChannelFormat(int val);

    void includeInAdmRenderValueChanged();
    void admTypeDefinitionChanged();
    void admPackFormatChanged();
    void admChannelFormatChanged();

    void incomingMessage(std::shared_ptr<NngMsg> msg);

private:
    AdmStemPluginAudioProcessorEditor* editor();

    int calcNumChannels();
    void updateNumChnsParam(bool force = false);
    void updateAdmParameters(bool force = false);

    int desiredTypeDefinition{ 0 }; //Undefined
    int desiredPackFormat{ PACKFORMAT_UNSET_ID };
    int desiredChannelFormat{ CHANNELFORMAT_ALLCHANNELS_ID };

    bool sendSamples;

    void renderIsStarting();
    void renderHasFinished();

    NonAutoAudioParameterBool* includeInAdmRender;
    ParamChangeCallback* includeInAdmRenderListener;
    ReadOnlyAudioParameterInt* sampleRateParam;
    ReadOnlyAudioParameterInt* numChnsParam;
    ReadOnlyAudioParameterInt* commandPort;
    ReadOnlyAudioParameterInt* samplesPort;
    NonAutoAudioParameterInt* admTypeDefinitionParam;
    ParamChangeCallback* admTypeDefinitionListener;
    NonAutoAudioParameterInt* admPackFormatParam;
    ParamChangeCallback* admPackFormatListener;
    NonAutoAudioParameterInt* admChannelFormatParam;
    ParamChangeCallback* admChannelFormatListener;

    CommandReceiver *commandSocket;
    SamplesSender *samplesSocket;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AdmStemPluginAudioProcessor)
};

