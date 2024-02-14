#include "admvstcontrol.h"

#include "pluginregistry.h"
#include <helper/adm_preset_definitions_helper.h>

using namespace admplug;

std::string AdmVst::vstName = ADM_VST_NAME;

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
    isCandidate &= (possibleCandidate->getCommandSocketPort() > 0);
    isCandidate &= (possibleCandidate->getSamplesSocketPort() > 0);
    return isCandidate;
}

std::vector<int> AdmVst::trackAdmVstIndexes(ReaperAPI const& api, MediaTrack *trk)
{
    std::vector<int> admVstPos;

    auto trackVstNames = api.TrackFX_GetActualFXNames(trk);
    for (int i = 0; i < trackVstNames.size(); ++i) {
        api.CleanFXName(trackVstNames[i]);
        if (trackVstNames[i] == vstName) {
            admVstPos.push_back(i);
        }
    }

    return admVstPos;
}

// ADMEXPORTVST

AdmVst::AdmVst(MediaTrack* mediaTrack, ReaperAPI const& api) : PluginInstance(mediaTrack, api) {
    auto index = api.TrackFX_AddByActualName(mediaTrack, ADM_VST_NAME, false, TrackFXAddMode::CreateIfMissing);
    if(index < 0) {
        throw std::runtime_error("Could not add to or get plugin from track");
    }
    name = ADM_VST_NAME;
    guid = std::make_unique<ReaperGUID>(api.TrackFX_GetFXGUID(mediaTrack, index));
}

AdmVst::AdmVst(MediaTrack * mediaTrack, int fxIndex, ReaperAPI const & api) : PluginInstance(mediaTrack, api) {
    std::string trackVstName;
    bool getNameSuccess = api.TrackFX_GetActualFXName(mediaTrack, fxIndex, trackVstName);
    if (getNameSuccess) {
        api.CleanFXName(trackVstName);
    }
    if(!getNameSuccess || trackVstName != ADM_VST_NAME) {
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

bool AdmVst::isUsingPresetDefinition()
{
    auto td = getAdmTypeDefinition();
    auto pfId = getAdmPackFormat();
    auto pfData = AdmPresetDefinitionsHelper::getSingleton().getPackFormatData(td, pfId);
    return pfData != nullptr;
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


