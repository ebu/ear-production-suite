#pragma once

#include "reaperapi.h"
#include "plugin.h"
#include "parameter.h"
#include <adm/adm.hpp>

#include <memory>
#include <string>
#include <sstream>
#include <iomanip>

#define COMMONDEFINITIONS_MAX_ID 0x0FFF

using namespace admplug;

#define ADM_VST_NAME "ADM Export Source"

#define ADM_VST_COMMANDPORTPARAM 0
#define ADM_VST_SAMPLESPORTPARAM 1
#define ADM_VST_PORTPARAM_NORMLIMIT 65535.0

#define ADM_VST_INCLUDEINADMPARAM 2

#define ADM_VST_SAMPLERATEPARAM 3
#define ADM_VST_SAMPLERATEPARAM_NORMLIMIT 192000.0

#define ADM_VST_NUMCHNSPARAM 4
#define ADM_VST_NUMCHNSPARAM_NORMLIMIT 64.0

#define ADM_VST_ADMTYPEDEFINITIONPARAM 5
#define ADM_VST_ADMPACKFORMATPARAM 6
#define ADM_VST_ADMCHANNELFORMATPARAM 7
#define ADM_VST_ADMPARAM_NORMLIMIT 65535.0 //0xFFFF

#define ADM_VST_CHANNELFORMAT_ALLCHANNELS_ID 0
#define ADM_VST_PACKFORMAT_UNSET_ID 0

class AdmVst : public PluginInstance
{
public:
    AdmVst(MediaTrack* mediaTrack, ReaperAPI const& api);
    AdmVst(MediaTrack* mediaTrack, int fxIndex, ReaperAPI const& api);
    ~AdmVst();

    bool getIncludeInRenderState();
    void setIncludeInRenderState(bool state);
    adm::TypeDescriptor getAdmType();
    void setAdmType(adm::TypeDescriptor admType);
    int getSampleRate();
    int getChannelCount();
    int getAdmTypeDefinition();
    void setAdmTypeDefinition(int typeDefinition);
    int getAdmPackFormat();
    void setAdmPackFormat(int packFormat);
    int getAdmChannelFormat();
    void setAdmChannelFormat(int channelFormat);

    bool isUsingCommonDefinition();

    int getSamplesSocketPort();
    int getCommandSocketPort();

    // Statics
    static const std::string* getVstNameStr();
    static bool isAvailable(ReaperAPI const& api, bool doRescan = true);
    static bool isCandidateForExport(std::shared_ptr<AdmVst> possibleCandidate);
    static std::vector<int> trackAdmVstIndexes(ReaperAPI const& api, MediaTrack *trk);

private:
    std::unique_ptr<PluginParameter> paramIncludeInRender   { createPluginParameter(ADM_VST_INCLUDEINADMPARAM,
                                                                                    ParameterRange{ 0.0, 1.0 }) };
    std::unique_ptr<PluginParameter> paramAdmTypeDefinition { createPluginParameter(ADM_VST_ADMTYPEDEFINITIONPARAM,
                                                                                    ParameterRange{ 0.0, ADM_VST_ADMPARAM_NORMLIMIT }) };
    std::unique_ptr<PluginParameter> paramAdmPackFormat     { createPluginParameter(ADM_VST_ADMPACKFORMATPARAM,
                                                                                    ParameterRange{ 0.0, ADM_VST_ADMPARAM_NORMLIMIT }) };
    std::unique_ptr<PluginParameter> paramAdmChannelFormat  { createPluginParameter(ADM_VST_ADMCHANNELFORMATPARAM,
                                                                                    ParameterRange{ 0.0, ADM_VST_ADMPARAM_NORMLIMIT }) };
    std::unique_ptr<PluginParameter> paramSampleRate        { createPluginParameter(ADM_VST_SAMPLERATEPARAM,
                                                                                    ParameterRange{ 0.0, ADM_VST_SAMPLERATEPARAM_NORMLIMIT }) };
    std::unique_ptr<PluginParameter> paramChannelCount      { createPluginParameter(ADM_VST_NUMCHNSPARAM,
                                                                                    ParameterRange{ 0.0, ADM_VST_NUMCHNSPARAM_NORMLIMIT }) };
    std::unique_ptr<PluginParameter> paramSamplesPort       { createPluginParameter(ADM_VST_SAMPLESPORTPARAM,
                                                                                    ParameterRange{ 0.0, ADM_VST_PORTPARAM_NORMLIMIT }) };
    std::unique_ptr<PluginParameter> paramCommandPort       { createPluginParameter(ADM_VST_COMMANDPORTPARAM,
                                                                                    ParameterRange{ 0.0, ADM_VST_PORTPARAM_NORMLIMIT }) };

    // Statics
    static std::string vstName; // Human-readable name
};