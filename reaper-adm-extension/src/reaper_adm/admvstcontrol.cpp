#include "admvstcontrol.h"

#include "pluginregistry.h"

using namespace admplug;

std::string AdmVst::vstName = ADM_VST_NAME;
std::string AdmVst::vstCompName = "";
size_t AdmVst::vstCompNameLen = 0;
const char* AdmVst::vstCompNameCStr = nullptr;

const char* AdmVst::getVstCompName() {
    if (vstCompNameCStr == nullptr) {
        vstCompName = "VST3: ";
        vstCompName.append(vstName);
        vstCompName.append(" (");
        vstCompNameLen = vstCompName.length();
        vstCompNameCStr = vstCompName.c_str();
    }
    return vstCompNameCStr;
}

const std::string* AdmVst::getVstNameStr()
{
    return &vstName;
}

bool AdmVst::isAvailable(ReaperAPI const& api, bool doRescan)
{
    return PluginRegistry::getInstance()->checkPluginAvailable(vstName, api, doRescan);
}

bool AdmVst::isCandidateForExport(std::shared_ptr<AdmVst> possibleCandidate)
{
    assert(possibleCandidate);
    bool isCandidate = possibleCandidate->getIncludeInRenderState();
    isCandidate &= !possibleCandidate->isBypassed();
    isCandidate &= !possibleCandidate->isPluginOffline();
    isCandidate &= (possibleCandidate->getSampleRate() > 0);
    isCandidate &= (possibleCandidate->getChannelCount() > 0);
    return isCandidate;
}

int AdmVst::trackAdmVstIndex(ReaperAPI const& api, MediaTrack *trk)
{
    return api.TrackFX_AddByName(trk, vstName.c_str(), false, TrackFXAddMode::QueryPresence);
}

std::vector<int> AdmVst::trackAdmVstIndexes(ReaperAPI const& api, MediaTrack *trk)
{
    int totalVsts = api.TrackFX_GetCount(trk);
    std::vector<int> admVstPos;

    for (int i = 0; i < totalVsts; i++) {
        if (vstPosIsAdmVst(api, trk, i)) admVstPos.push_back(i);
    }

    return admVstPos;
}

bool AdmVst::vstPosIsAdmVst(ReaperAPI const& api, MediaTrack *trk, int vstPos)
{
    getVstCompName(); // Need to generate it to get length
    char name[100];
    if (!api.TrackFX_GetFXName(trk, vstPos, name, (int)vstCompNameLen + 1)) return false;
    return (strcmp(name, getVstCompName()) == 0);
}

// ADMEXPORTVST

AdmVst::AdmVst(MediaTrack* mediaTrack, ReaperAPI const& api) : PluginInstance(mediaTrack, api) {
    auto index = api.TrackFX_AddByName(mediaTrack, ADM_VST_NAME, false, TrackFXAddMode::CreateIfMissing);
    if(index < 0) {
        throw std::runtime_error("Could not add to or get plugin from track");
    }
    name = ADM_VST_NAME;
    guid = std::make_unique<ReaperGUID>(api.TrackFX_GetFXGUID(mediaTrack, index));
}

AdmVst::AdmVst(MediaTrack * mediaTrack, int fxIndex, ReaperAPI const & api) : PluginInstance(mediaTrack, api) {
    if(!vstPosIsAdmVst(api, mediaTrack, fxIndex)) {
        throw std::runtime_error("Plugin is not an ADM export plugin");
    }
    name = ADM_VST_NAME;
    guid = std::make_unique<ReaperGUID>(api.TrackFX_GetFXGUID(mediaTrack, fxIndex));
}

AdmVst::~AdmVst()
{
}

bool AdmVst::getIncludeInRenderState() {
    auto optVal = getParameter(*paramIncludeInRender);
    assert(optVal.has_value()); // Check for not returned
    assert(*optVal == 0.0 || *optVal == 1.0); // Check for ambiguity
    return *optVal == 1.0;
}

void AdmVst::setIncludeInRenderState(bool state) {
    setParameter(*paramIncludeInRender, state ? 1.0 : 0.0);
}

adm::TypeDescriptor AdmVst::getAdmType() {
    auto optVal = getParameterWithConvertToInt(*paramAdmTypeDefinition);
    assert(optVal.has_value());
    return (adm::TypeDescriptor)*optVal;
}

void AdmVst::setAdmType(adm::TypeDescriptor admType) {
    int admTypeInt = std::stoi(formatTypeLabel(admType), nullptr, 16);
    double admTypeParamVal = paramAdmTypeDefinition->forwardMap((double)admTypeInt);
    setParameter(*paramAdmTypeDefinition, admTypeParamVal);
}

int AdmVst::getSampleRate() {
    auto optVal = getParameterWithConvertToInt(*paramSampleRate);
    assert(optVal.has_value());
    return *optVal;
}

int AdmVst::getChannelCount() {
    auto optVal = getParameterWithConvertToInt(*paramChannelCount);
    assert(optVal.has_value());
    return *optVal;
}

int AdmVst::getAdmTypeDefinition()
{
    auto optVal = getParameterWithConvertToInt(*paramAdmTypeDefinition);
    assert(optVal.has_value());
    return *optVal;
}

void AdmVst::setAdmTypeDefinition(int typeDefinition)
{
    setParameterWithConvert(*paramAdmTypeDefinition, (double)typeDefinition);
}

int AdmVst::getAdmPackFormat()
{
    auto optVal = getParameterWithConvertToInt(*paramAdmPackFormat);
    assert(optVal.has_value());
    return *optVal;
}

void AdmVst::setAdmPackFormat(int packFormat)
{
    setParameterWithConvert(*paramAdmPackFormat, (double)packFormat);
}

int AdmVst::getAdmChannelFormat()
{
    auto optVal = getParameterWithConvertToInt(*paramAdmChannelFormat);
    assert(optVal.has_value());
    return *optVal;
}

void AdmVst::setAdmChannelFormat(int channelFormat)
{
    setParameterWithConvert(*paramAdmChannelFormat, (double)channelFormat);
}

bool AdmVst::isUsingCommonDefinition()
{
    auto packFormat = getAdmPackFormat();
    if(packFormat == ADM_VST_PACKFORMAT_UNSET_ID) return false;
    bool packIsCommonDef = (packFormat <= COMMONDEFINITIONS_MAX_ID);
    bool channelIsCommonDef = (getAdmChannelFormat() <= COMMONDEFINITIONS_MAX_ID);
    assert(packIsCommonDef == channelIsCommonDef); // Should be
    return (packIsCommonDef && channelIsCommonDef);
}

int AdmVst::getSamplesSocketPort()
{
    auto optVal = getParameterWithConvertToInt(*paramSamplesPort);
    assert(optVal.has_value());
    return *optVal;
}

int AdmVst::getCommandSocketPort()
{
    auto optVal = getParameterWithConvertToInt(*paramCommandPort);
    assert(optVal.has_value());
    return *optVal;
}


