#include "exportaction_admsource-earvst.h"

#include "pluginregistry.h"
#include "pluginsuite_ear.h"
#include <version/eps_version.h>

#include <adm/write.hpp>
#include <adm/utilities/id_assignment.hpp>
#include <adm/parse.hpp>

#include <sstream>

using namespace admplug;

EarVstExportSources::EarVstExportSources(ReaperAPI const & api) : IExportSources(api)
{
    MediaTrack *trk;
    int numTracks = api.CountTracks(nullptr);

    for (int trackNum = 0; trackNum < numTracks; trackNum++) {

        trk = api.GetTrack(nullptr, trackNum);
        if (trk) {

            auto fxPosVec = EarSceneMasterVst::trackEarSceneMasterVstIndexes(api, trk);
            for(int fxPos : fxPosVec) {
                auto earSceneMasterVst = std::make_shared<EarSceneMasterVst>(trk, fxPos, api);
                auto comms = earSceneMasterVst->getCommunicator(true);
                if(comms) comms->updateInfo();

                allEarSceneMasterVsts.push_back(earSceneMasterVst);
                if(EarSceneMasterVst::isCandidateForExport(earSceneMasterVst)) {
                    if(!chosenCandidateForExport) {
                        chosenCandidateForExport = earSceneMasterVst;
                        generateAdmAndChna(api);
                    }
                    candidatesForExport.push_back(earSceneMasterVst);
                }
            }
        }
    }

    if(!chosenCandidateForExport) {
        errorStrings.push_back(std::string("No instance of EAR Scene found which is able to export!"));
    }
}

int EarVstExportSources::getSampleRate()
{
    if(chosenCandidateForExport){
        int sr = chosenCandidateForExport->getSampleRate();
        if(sr > 0) return sr;
    }
    for(auto &thisVst : allEarSceneMasterVsts) {
        int sr = thisVst->getSampleRate();
        if(sr > 0) return sr;
    }
    return 0;
}

int EarVstExportSources::getTotalExportChannels()
{
    if(chosenCandidateForExport) {
        return chosenCandidateForExport->getChannelCount();
    }
    return 0;
}

void EarVstExportSources::setRenderInProgress(bool state)
{
    if(chosenCandidateForExport){
        chosenCandidateForExport->setRenderInProgressState(true);
    }
}

bool EarVstExportSources::isFrameAvailable()
{
    if(!chosenCandidateForExport) return false;
    return chosenCandidateForExport->getCommunicator()->nextFrameAvailable();
}

bool EarVstExportSources::writeNextFrameTo(float * bufferWritePointer, bool skipFrameAvailableCheck)
{
    if(!skipFrameAvailableCheck && !isFrameAvailable()) return false;

    auto communicator = chosenCandidateForExport->getCommunicator();
    int channelsToWrite = communicator->getReportedChannelCount();
    if(channelsToWrite > 0) {
        if(!communicator->copyNextFrame(bufferWritePointer, true)) {
            throw std::runtime_error("copyNextFrame failed");
        }
        bufferWritePointer += channelsToWrite;
    }

    return true;
}

void EarVstExportSources::generateAdmAndChna(ReaperAPI const& api)
{
    using namespace adm;

    // Get ADM template in to admDocument
    std::istringstream sstr(chosenCandidateForExport->getAdmTemplateStr());
    try {
        admDocument = adm::parseXml(sstr, adm::xml::ParserOptions::recursive_node_search); // Shouldn't need recursive here, but doesn't harm to do it anyway
    } catch(std::exception &e) {
        std::string str = "Failed to parse ADM template: \"";
        str += e.what();
        str += "\"";
        errorStrings.push_back(str);
        return;
    }

    // This method creates a map which makes it quicker and easier to find relevant ADM Elements
    updateAdmElementsForAudioTrackUidValueMap();

    // Any track UID that won't be written should be removed
    auto docAudioTrackUids = admDocument->getElements<adm::AudioTrackUid>();
    std::vector<std::shared_ptr<adm::AudioTrackUid>> missingList;
    for(auto docAudioTrackUid : docAudioTrackUids) {
        uint32_t idValue = docAudioTrackUid->get<adm::AudioTrackUidId>().get<adm::AudioTrackUidIdValue>().get();
        if(admElementsForAudioTrackUidValue.find(idValue) == admElementsForAudioTrackUidValue.end()) {
            missingList.push_back(docAudioTrackUid);
        }
    }
    for(auto uid : missingList) {
        auto tf = uid->getReference<adm::AudioTrackFormat>();
        if(tf) {
            auto sf = tf->getReference<adm::AudioStreamFormat>();
            if(sf) {
                admDocument->remove(sf);
            }
            admDocument->remove(tf);
        }
        admDocument->remove(uid);
    }

    // Create CHNA chunk
    std::vector<bw64::AudioId> audioIds;
    for(auto &channelMapping : chosenCandidateForExport->getChannelMappings()) {
        auto admElements = getAdmElementsFor(channelMapping.audioTrackUidValue);
        assert(admElements.has_value());

        audioIds.push_back(bw64::AudioId(channelMapping.writtenChannelNumber + 1, //1-Indexed in CHNA!!!!!!!!!!!!!!!
                                         formatId((*admElements).audioTrackUid->get<AudioTrackUidId>()),
                                         formatId((*admElements).audioTrackFormat->get<AudioTrackFormatId>()),
                                         formatId((*admElements).audioPackFormat->get<AudioPackFormatId>())
        ));
    }

    chnaChunk = std::make_shared<bw64::ChnaChunk>(bw64::ChnaChunk(audioIds));

    //Populate rest of ADM template (admDocument) here with AudioBlockFormats

    std::shared_ptr<admplug::PluginSuite> pluginSuite = std::make_shared<EARPluginSuite>();

    for(auto &channelMapping : chosenCandidateForExport->getChannelMappings()) {
        auto admElements = getAdmElementsFor(channelMapping.audioTrackUidValue);
        assert(admElements.has_value());

        // Find the associated plugin

        auto feedingPlugins = getEarInputPluginsWithTrackMapping(channelMapping.originalChannelNumber, api);
        if(feedingPlugins.size() == 0) {
            std::string msg("Unable to find Input plugin feeding channel ");
            msg += std::to_string(channelMapping.originalChannelNumber + 1);
            errorStrings.push_back(msg);
            continue;
        } else if(feedingPlugins.size() > 1) {
            std::string msg("Multiple Input plugins found feeding channel ");
            msg += std::to_string(channelMapping.originalChannelNumber + 1);
            errorStrings.push_back(msg);
            continue;
        }
        auto pluginInst = feedingPlugins[0];

        // Set audioobject start/duration

        auto start = std::chrono::nanoseconds::zero();
        auto duration = toNs(api.GetProjectLength(nullptr));

        if(std::stof(api.GetAppVersion()) >= 6.01f)
        { 
            // Check if Tail option in rendering dialog is activated and add length to duration if so
            bool tailFlag = static_cast<size_t>(api.GetSetProjectInfo(nullptr, "RENDER_TAILFLAG", 0., false)) & 0x2;
            bool boundsFlag = static_cast<size_t>(api.GetSetProjectInfo(nullptr, "RENDER_BOUNDSFLAG", 0., false)) == 1;
            if (tailFlag && boundsFlag) {
                const double tailLengthMs = api.GetSetProjectInfo(nullptr, "RENDER_TAILMS", 0., false);
                duration += toNs(tailLengthMs / 1000.);
            }
        }

        if((*admElements).audioObject) {
            auto mediaTrack = pluginInst->getTrackInstance().get();
            auto bounds = api.getTrackAudioBounds(mediaTrack, true); // True = ignore before zero - we don't do sub-zero bounds
            if(bounds.has_value()) {
                start = toNs((*bounds).first);
                duration = toNs((*bounds).second - (*bounds).first);
                (*admElements).audioObject->set(adm::Start{ start });
                (*admElements).audioObject->set(adm::Duration{ duration });
            }
        }

        if(!(*admElements).isUsingCommonDefinition) {

            auto cumulatedPointData = CumulatedPointData(start, start + duration);

            // Get all values for all parameters, whether automated or not.
            for(int admParameterIndex = 0; admParameterIndex != (int)AdmParameter::NONE; admParameterIndex++) {
                auto admParameter = (AdmParameter)admParameterIndex;
                auto param = pluginSuite->getParameterFor(admParameter);
                auto env = getEnvelopeFor(pluginSuite, pluginInst.get(), admParameter, api);
                
                if (getEnvelopeBypassed(env, api)) { 
                    // We have an envelope, but it is bypassed
                    auto val = getValueFor(pluginSuite, pluginInst.get(), admParameter, api);
                    auto newErrors = cumulatedPointData.useConstantValueForParameter(admParameter, *val);
                    for (auto& newError : newErrors) {
                        warningStrings.push_back(newError.what());
                    }
                } else if(param && env) {
                    // We have an envelope for this ADM parameter
                    auto newErrors = cumulatedPointData.useEnvelopeDataForParameter(*env, *param, admParameter, api);
                    for(auto &newError : newErrors) {
                        warningStrings.push_back(newError.what());
                    }

                } else if(auto val = getValueFor(pluginSuite, pluginInst.get(), admParameter, api)) {
                    // We do not have an envelope for this ADM parameter but the plugin suite CAN provide a fixed value for it
                    // NOTE that this will include parameters NOT relevant to the current audioObject type, but these are ignored during block creation.
                    auto newErrors = cumulatedPointData.useConstantValueForParameter(admParameter, *val);
                    for(auto &newError : newErrors) {
                        warningStrings.push_back(newError.what());
                    }
                }
            }

            if((*admElements).typeDescriptor == adm::TypeDefinition::OBJECTS) {
                auto blocks = cumulatedPointData.generateAudioBlockFormatObjects(pluginSuite, pluginInst.get(), api);
                for(auto& block : *blocks) (*admElements).audioChannelFormat->add(*block);
            }
            else if((*admElements).typeDescriptor == adm::TypeDefinition::DIRECT_SPEAKERS) {
                //TODO
                warningStrings.push_back("Currently only supporting Common Defintions for non-Objects types");
            }
            else if((*admElements).typeDescriptor == adm::TypeDefinition::HOA) {
                //TODO
                warningStrings.push_back("Currently only supporting Common Defintions for non-Objects types");
            }
            else if((*admElements).typeDescriptor == adm::TypeDefinition::BINAURAL) {
                //TODO
                warningStrings.push_back("Currently only supporting Common Defintions for non-Objects types");
            }
            else if((*admElements).typeDescriptor == adm::TypeDefinition::MATRIX) {
                //TODO
                warningStrings.push_back("Currently only supporting Common Defintions for non-Objects types");
            }
        }
    }

    // Create AXML Chunk
    std::stringstream xmlStream;
    adm::writeXml(xmlStream, admDocument);
    xmlStream << "<!-- Produced using the EAR Production Suite (version ";
    xmlStream << (eps::versionInfoAvailable()? eps::currentVersion() : "unknown");
    xmlStream << "), from the EAR Scene plugin -->\n";
    axmlChunk = std::make_shared<bw64::AxmlChunk>(bw64::AxmlChunk(xmlStream.str()));

}

std::optional<EarVstExportSources::AdmElements> EarVstExportSources::getAdmElementsFor(uint32_t audioTrackUidValue)
{
    auto mapIt = admElementsForAudioTrackUidValue.find(audioTrackUidValue);
    if(mapIt == admElementsForAudioTrackUidValue.end()) return std::optional<AdmElements>();
    return std::optional<AdmElements>(mapIt->second);
}

void EarVstExportSources::updateAdmElementsForAudioTrackUidValueMap()
{
    admElementsForAudioTrackUidValue.clear();
    auto audioObjects = admDocument->getElements<adm::AudioObject>();

    for(auto &channelMapping : chosenCandidateForExport->getChannelMappings()) {
        std::string audioTrackUidStr = "ATU_";
        audioTrackUidStr += intToHex(channelMapping.audioTrackUidValue);
        adm::AudioTrackUidId audioTrackUidId;
        try {
            audioTrackUidId = adm::parseAudioTrackUidId(audioTrackUidStr);
        } catch(std::runtime_error &e) {
            warningStrings.push_back(std::string("Parsing Track UID: ") + e.what());
            continue;
        }

        auto audioTrackUid = admDocument->lookup(audioTrackUidId);
        assert(audioTrackUid);
        if(!audioTrackUid) {
            warningStrings.push_back(std::string("No Audio Track UID found for ") + audioTrackUidStr);
            continue;
        }

        // We make a lot of assumptions about the ADM structure here, but since we know what the EAR plugins will send us, this is probably safe

        auto audioPackFormat = audioTrackUid->getReference<adm::AudioPackFormat>();
        assert(audioPackFormat);
        if(!audioPackFormat) {
            warningStrings.push_back(std::string("No Pack Format referenced from ") + audioTrackUidStr);
            continue;
        }

        auto audioTrackFormat = audioTrackUid->getReference<adm::AudioTrackFormat>();
        assert(audioTrackFormat);
        if(!audioTrackFormat) {
            warningStrings.push_back(std::string("No Track Format referenced from ") + audioTrackUidStr);
            continue;
        }

        auto audioStreamFormat = audioTrackFormat->getReference<adm::AudioStreamFormat>();
        assert(audioStreamFormat);
        if(!audioStreamFormat) {
            warningStrings.push_back(std::string("No Stream Format referenced recursively from ") + audioTrackUidStr);
            continue;
        }

        auto audioChannelFormat = audioStreamFormat->getReference<adm::AudioChannelFormat>();
        assert(audioChannelFormat);
        if(!audioChannelFormat) {
            warningStrings.push_back(std::string("No Channel Format referenced recursively from ") + audioTrackUidStr);
            continue;
        }

        std::shared_ptr<adm::AudioObject> audioObject = nullptr;
        for(auto checkAudioObject : audioObjects) {
            auto audioTrackUids = checkAudioObject->getReferences<adm::AudioTrackUid>();
            for(auto checkAudioTrackUid : audioTrackUids) {
                if(checkAudioTrackUid == audioTrackUid) {
                    assert(audioObject == nullptr); // should only be one match due to way EAR plugins structure ADM
                    audioObject = checkAudioObject;
                }
            }
        }
        assert(audioObject != nullptr); // should be a match due to way EAR plugins structure ADM

        auto audioChannelFormatId = audioChannelFormat->get<adm::AudioChannelFormatId>().get<adm::AudioChannelFormatIdValue>().get();
        bool isCommonDefinition = (audioChannelFormatId <= COMMONDEFINITIONS_MAX_ID);

        adm::TypeDescriptor typeDescriptor = audioChannelFormat->get<adm::TypeDescriptor>();

        admElementsForAudioTrackUidValue.insert({
            channelMapping.audioTrackUidValue,
            AdmElements{
                isCommonDefinition,
                typeDescriptor,
                audioTrackUid,
                audioTrackFormat,
                audioPackFormat,
                audioChannelFormat,
                audioObject
        } });
    }
}

TrackEnvelope* EarVstExportSources::getEnvelopeFor(std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance * pluginInst, AdmParameter admParameter, ReaperAPI const & api)
{
    MediaTrack* track = pluginInst->getTrackInstance().get();

    // Plugin Parameters
    if(auto param = pluginSuite->getPluginParameterFor(admParameter)) {
        return api.GetFXEnvelope(track, pluginInst->getPluginIndex(), param->index(), false);
    }

    return nullptr;
}

std::optional<double> EarVstExportSources::getValueFor(std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance * pluginInst, AdmParameter admParameter, ReaperAPI const & api)
{
    MediaTrack* track = pluginInst->getTrackInstance().get();

    // Plugin Parameters
    if(auto param = pluginSuite->getPluginParameterFor(admParameter)) {
        return pluginInst->getParameterWithConvert(*param);
    }

    return std::optional<double>();
}

bool EarVstExportSources::getEnvelopeBypassed(TrackEnvelope* env, ReaperAPI const& api)
{
    bool envBypassed = false;
    if(env) {
        char chunk[1024]; // For a plugin parameter (PARMENV) the ACT flag should always be within the first couple bytes of the state chunk
        bool getRes = api.GetEnvelopeStateChunk(env, chunk, 1024, false);
        if (getRes) {
            std::istringstream chunkSs(chunk);
            std::string line;
            while (std::getline(chunkSs, line)) {
                auto activePos = line.rfind("ACT ", 0);
                if ((activePos != std::string::npos) && (line.size() > (activePos + 4))) {
                    envBypassed = line.at(activePos + 4) == '0';
                    break;
                }
            }
        }
    }
    return envBypassed;
}

std::vector<std::shared_ptr<PluginInstance>> EarVstExportSources::getEarInputPluginsWithTrackMapping(int trackMapping, ReaperAPI const& api)
{
    std::vector<std::shared_ptr<PluginInstance>> insts;

    for(int trackNum = 0; trackNum < api.CountTracks(0); trackNum++) {
        auto trk = api.GetTrack(0, trackNum);
        for(int vstPos = 0; vstPos < api.TrackFX_GetCount(trk); vstPos++) {
            if(EarInputVst::isInputPlugin(api, trk, vstPos)) {
                auto pluginInst = std::make_shared<EarInputVst>(trk, vstPos, api);
                auto pluginTrackMapping = pluginInst->getTrackMapping();
                auto pluginWidth = pluginInst->getWidth();
                auto pluginStartCh = pluginTrackMapping;
                auto pluginEndCh = pluginTrackMapping + pluginWidth - 1;

                if(trackMapping >= pluginStartCh && trackMapping <= pluginEndCh) {
                    insts.push_back(std::make_shared<PluginInstance>(trk, vstPos, api));
                }
            }
        }
    }
    return insts;
}

//EarInputVst

std::string EarInputVst::directSpeakersVstName = admplug::EARPluginSuite::DIRECTSPEAKERS_METADATA_PLUGIN_NAME;
std::string EarInputVst::directSpeakersVstCompName = "";
size_t EarInputVst::directSpeakersVstCompNameLen = 0;
const char* EarInputVst::directSpeakersVstCompNameCStr = nullptr;

const char* EarInputVst::getDirectSpeakersVstCompName() {
    if (directSpeakersVstCompNameCStr == nullptr) {
        directSpeakersVstCompName = "VST3: ";
        directSpeakersVstCompName.append(directSpeakersVstName);
        directSpeakersVstCompName.append(" (");
        directSpeakersVstCompNameLen = directSpeakersVstCompName.length();
        directSpeakersVstCompNameCStr = directSpeakersVstCompName.c_str();
    }
    return directSpeakersVstCompNameCStr;
}

const std::string* EarInputVst::getDirectSpeakersVstNameStr()
{
    return &directSpeakersVstName;
}

bool EarInputVst::isDirectSpeakersVstAvailable(ReaperAPI const& api, bool doRescan)
{
    return PluginRegistry::getInstance()->checkPluginAvailable(directSpeakersVstName, api, doRescan);
}

int EarInputVst::trackDirectSpeakersVstIndex(ReaperAPI const& api, MediaTrack *trk)
{
    return api.TrackFX_AddByName(trk, directSpeakersVstName.c_str(), false, TrackFXAddMode::QueryPresence);
}

std::vector<int> EarInputVst::trackDirectSpeakersVstIndexes(ReaperAPI const& api, MediaTrack *trk)
{
    int totalVsts = api.TrackFX_GetCount(trk);
    std::vector<int> vstPos;

    for (int i = 0; i < totalVsts; i++) {
        if (vstPosIsDirectSpeakersVst(api, trk, i)) vstPos.push_back(i);
    }

    return vstPos;
}

bool EarInputVst::vstPosIsDirectSpeakersVst(ReaperAPI const& api, MediaTrack *trk, int vstPos)
{
    getDirectSpeakersVstCompName(); // Need to generate it to get length
    char name[100];
    if (!api.TrackFX_GetFXName(trk, vstPos, name, (int)directSpeakersVstCompNameLen + 1)) return false;
    return (strcmp(name, getDirectSpeakersVstCompName()) == 0);
}

std::string EarInputVst::objectVstName = admplug::EARPluginSuite::OBJECT_METADATA_PLUGIN_NAME;
std::string EarInputVst::objectVstCompName = "";
size_t EarInputVst::objectVstCompNameLen = 0;
const char* EarInputVst::objectVstCompNameCStr = nullptr;

const char* EarInputVst::getObjectVstCompName() {
    if (objectVstCompNameCStr == nullptr) {
        objectVstCompName = "VST3: ";
        objectVstCompName.append(objectVstName);
        objectVstCompName.append(" (");
        objectVstCompNameLen = objectVstCompName.length();
        objectVstCompNameCStr = objectVstCompName.c_str();
    }
    return objectVstCompNameCStr;
}

const std::string* EarInputVst::getObjectVstNameStr()
{
    return &objectVstName;
}

bool EarInputVst::isObjectVstAvailable(ReaperAPI const& api, bool doRescan)
{
    return PluginRegistry::getInstance()->checkPluginAvailable(objectVstName, api, doRescan);
}

int EarInputVst::trackObjectVstIndex(ReaperAPI const& api, MediaTrack *trk)
{
    return api.TrackFX_AddByName(trk, objectVstName.c_str(), false, TrackFXAddMode::QueryPresence);
}

std::vector<int> EarInputVst::trackObjectVstIndexes(ReaperAPI const& api, MediaTrack *trk)
{
    int totalVsts = api.TrackFX_GetCount(trk);
    std::vector<int> vstPos;

    for (int i = 0; i < totalVsts; i++) {
        if (vstPosIsObjectVst(api, trk, i)) vstPos.push_back(i);
    }

    return vstPos;
}

bool EarInputVst::vstPosIsObjectVst(ReaperAPI const& api, MediaTrack *trk, int vstPos)
{
    getObjectVstCompName(); // Need to generate it to get length
    char name[100];
    if (!api.TrackFX_GetFXName(trk, vstPos, name, (int)objectVstCompNameLen + 1)) return false;
    return (strcmp(name, getObjectVstCompName()) == 0);
}

bool EarInputVst::isObjectPlugin(std::string vstNameStr)
{
    // Name only
    if(vstNameStr == objectVstName) return true;
    // Comparison name (includes VST3: and ends with company name and channel count)
    getObjectVstCompName(); // need to ensure generated
    if (strncmp (vstNameStr.c_str(), getObjectVstCompName(), objectVstCompNameLen) == 0) return true;
    return false;
}

bool EarInputVst::isDirectSpeakersPlugin(std::string vstNameStr)
{
    // Name only
    if(vstNameStr == directSpeakersVstName) return true;
    // Comparison name (includes VST3: and ends with company name and channel count)
    getDirectSpeakersVstCompName(); // need to ensure generated
    if (strncmp (vstNameStr.c_str(), getDirectSpeakersVstCompName(), directSpeakersVstCompNameLen) == 0) return true;
    return false;
}

bool EarInputVst::isInputPlugin(char * vstName)
{
    auto vstNameStr = std::string(vstName);
    return isObjectPlugin(vstNameStr) || isDirectSpeakersPlugin(vstNameStr) || isHoaPlugin(vstNameStr);
}

bool EarInputVst::isInputPlugin(ReaperAPI const& api, MediaTrack *trk, int vstPos) {
    if(vstPosIsObjectVst(api, trk, vstPos)) return true;
    if(vstPosIsDirectSpeakersVst(api, trk, vstPos)) return true;
    if (vstPosIsHoaVst(api, trk, vstPos)) return true;
    return false;
}

EarInputVst::EarInputVst(MediaTrack * mediaTrack, char* vstName, ReaperAPI const & api) : PluginInstance(mediaTrack, api) {
    if(!isInputPlugin(vstName)) {
        throw std::runtime_error("VST Name is not a known EAR Input VST");
    }
    auto index = api.TrackFX_AddByName(mediaTrack, vstName, false, TrackFXAddMode::CreateIfMissing);
    if(index < 0) {
        throw std::runtime_error("Could not add to or get plugin from track");
    }
    name = vstName;
    guid = std::make_unique<ReaperGUID>(api.TrackFX_GetFXGUID(mediaTrack, index));
}

EarInputVst::EarInputVst(MediaTrack * mediaTrack, int fxIndex, ReaperAPI const & api) : PluginInstance(mediaTrack, api) {
    char nameAtFxIndex[100];
    if(!api.TrackFX_GetFXName(mediaTrack, fxIndex, nameAtFxIndex, 100)) {
        throw std::runtime_error("Cannot get VST Name");
    }
    if(!isInputPlugin(nameAtFxIndex)) {
        throw std::runtime_error("VST Name is not a known EAR Input VST");
    }
    name = nameAtFxIndex;
    guid = std::make_unique<ReaperGUID>(api.TrackFX_GetFXGUID(mediaTrack, fxIndex));
}

int EarInputVst::getTrackMapping()
{
    assert(paramTrackMapping);
    auto optVal = getParameterWithConvertToInt(*paramTrackMapping);
    assert(optVal.has_value());
    return *optVal;
}

int EarInputVst::getWidth()
{
    if (isObjectPlugin(name)) { return 1; }
    else if (isDirectSpeakersPlugin(name)) {
        assert(paramSpeakerLayout);
        //auto SL = *paramSpeakerLayout.get();
        auto speakerLayout = getParameterWithConvertToInt(*paramSpeakerLayout);
        assert(speakerLayout.has_value());

        int trackWidth = speakerLayout.has_value() ? EARPluginSuite::countChannelsInSpeakerLayout(*speakerLayout) : 0;
        return trackWidth;
    }
    else if (isHoaPlugin(name)) {
        std::optional<int> packFormatId = getParameterWithConvertToInt(*paramPackFormatId);
        assert(packFormatId.has_value());
        int trackWidth = packFormatId.has_value() ? EARPluginSuite::countChannelsInHoaPackFormat(*packFormatId) : 0;
        return trackWidth;
    }
    else { return 0; }

}

// EarSceneMasterVst

std::string EarSceneMasterVst::vstName = admplug::EARPluginSuite::SCENEMASTER_PLUGIN_NAME;
std::string EarSceneMasterVst::vstCompName = "";
size_t EarSceneMasterVst::vstCompNameLen = 0;
const char* EarSceneMasterVst::vstCompNameCStr = nullptr;

const char* EarSceneMasterVst::getVstCompName() {
    if (vstCompNameCStr == nullptr) {
        vstCompName = "VST3: ";
        vstCompName.append(vstName);
        vstCompName.append(" (");
        vstCompNameLen = vstCompName.length();
        vstCompNameCStr = vstCompName.c_str();
    }
    return vstCompNameCStr;
}

const std::string* EarSceneMasterVst::getVstNameStr()
{
    return &vstName;
}

bool EarSceneMasterVst::isAvailable(ReaperAPI const& api, bool doRescan)
{
    return PluginRegistry::getInstance()->checkPluginAvailable(vstName, api, doRescan);
}

bool EarSceneMasterVst::isCandidateForExport(std::shared_ptr<EarSceneMasterVst> possibleCandidate)
{
    assert(possibleCandidate);
    bool isCandidate = true;
    isCandidate &= !possibleCandidate->isBypassed();
    isCandidate &= !possibleCandidate->isPluginOffline();
    isCandidate &= (possibleCandidate->getSampleRate() > 0);
    isCandidate &= (possibleCandidate->getCommandSocketPort() > 0);
    isCandidate &= (possibleCandidate->getSamplesSocketPort() > 0);
    return isCandidate;
}

int EarSceneMasterVst::trackEarSceneMasterVstIndex(ReaperAPI const& api, MediaTrack *trk)
{
    return api.TrackFX_AddByName(trk, vstName.c_str(), false, TrackFXAddMode::QueryPresence);
}

std::vector<int> EarSceneMasterVst::trackEarSceneMasterVstIndexes(ReaperAPI const& api, MediaTrack *trk)
{
    int totalVsts = api.TrackFX_GetCount(trk);
    std::vector<int> vstPos;

    for (int i = 0; i < totalVsts; i++) {
        if (vstPosIsEarSceneMasterVst(api, trk, i)) vstPos.push_back(i);
    }

    return vstPos;
}

bool EarSceneMasterVst::vstPosIsEarSceneMasterVst(ReaperAPI const& api, MediaTrack *trk, int vstPos)
{
    getVstCompName(); // Need to generate it to get length
    char name[100];
    if (!api.TrackFX_GetFXName(trk, vstPos, name, (int)vstCompNameLen + 1)) return false;
    return (strcmp(name, getVstCompName()) == 0);
}

bool EarSceneMasterVst::isCommunicatorPresent()
{
    return (bool)communicator;
}

bool EarSceneMasterVst::obtainCommunicator()
{
    if(isBypassed() || isPluginOffline()) return false; // Do not create if Offline - it isn't running and therefore won't connect!
    auto samplesPort = getSamplesSocketPort();
    auto commandPort = getCommandSocketPort();
    if(commandPort == 0) return false; // If the vst failed to load, it can still appear in the proj as online and not bypassed - commandPort will be zero though.
    if(isCommunicatorPresent()) releaseCommunicator();
    communicator = CommunicatorRegistry::getCommunicator<EarVstCommunicator>(samplesPort, commandPort);
    return true;
}

void EarSceneMasterVst::releaseCommunicator()
{
    communicator.reset();
}

EarSceneMasterVst::EarSceneMasterVst(MediaTrack * mediaTrack, ReaperAPI const & api) : PluginInstance(mediaTrack, api)
{
    auto index = api.TrackFX_AddByName(mediaTrack, EARPluginSuite::SCENEMASTER_PLUGIN_NAME, false, TrackFXAddMode::CreateIfMissing);
    if(index < 0) {
        throw std::runtime_error("Could not add to or get plugin from track");
    }
    name = EARPluginSuite::SCENEMASTER_PLUGIN_NAME;
    guid = std::make_unique<ReaperGUID>(api.TrackFX_GetFXGUID(mediaTrack, index));
}

EarSceneMasterVst::EarSceneMasterVst(MediaTrack * mediaTrack, int fxIndex, ReaperAPI const & api) : PluginInstance(mediaTrack, api)
{
    if(!vstPosIsEarSceneMasterVst(api, mediaTrack, fxIndex)) {
        throw std::runtime_error("Plugin is not an Ear Scene Master plugin");
    }
    name = EARPluginSuite::SCENEMASTER_PLUGIN_NAME;
    guid = std::make_unique<ReaperGUID>(api.TrackFX_GetFXGUID(mediaTrack, fxIndex));
}

EarSceneMasterVst::~EarSceneMasterVst()
{
}

int EarSceneMasterVst::getSampleRate()
{
    if(!isCommunicatorPresent() && !obtainCommunicator()) return 0;
    return communicator->getReportedSampleRate();
}

int EarSceneMasterVst::getChannelCount()
{
    if(!isCommunicatorPresent() && !obtainCommunicator()) return 0;
    return communicator->getReportedChannelCount();
}

std::string EarSceneMasterVst::getAdmTemplateStr()
{
    if(!isCommunicatorPresent() && !obtainCommunicator()) return "";
    return communicator->getAdmTemplateStr();
}

std::vector<EarVstCommunicator::ChannelMapping> EarSceneMasterVst::getChannelMappings()
{
    if(!isCommunicatorPresent() && !obtainCommunicator()) return std::vector<EarVstCommunicator::ChannelMapping>();
    return communicator->getChannelMappings();
}

bool EarSceneMasterVst::getRenderInProgressState()
{
    if(!isCommunicatorPresent()) return false;
    return communicator->getRenderingState();
}

void EarSceneMasterVst::setRenderInProgressState(bool state)
{
    if(state == true && !isCommunicatorPresent()) obtainCommunicator();
    if(isCommunicatorPresent()) communicator->setRenderingState(state);
}

EarVstCommunicator * EarSceneMasterVst::getCommunicator(bool mustExist)
{
    if(mustExist && !isCommunicatorPresent()) obtainCommunicator();
    return communicator.get();
}

int EarSceneMasterVst::getSamplesSocketPort()
{
    auto optVal = getParameterWithConvertToInt(*paramSamplesPort);
    assert(optVal.has_value());
    return *optVal;
}

int EarSceneMasterVst::getCommandSocketPort()
{
    auto optVal = getParameterWithConvertToInt(*paramCommandPort);
    assert(optVal.has_value());
    return *optVal;
}



EarVstCommunicator::EarVstCommunicator(int samplesPort, int commandPort) : CommunicatorBase(samplesPort, commandPort)
{
}

void EarVstCommunicator::updateInfo()
{
    infoExchange();
    admAndMappingExchange();
}

bool EarVstCommunicator::copyNextFrame(float * buf, bool bypassAvailabilityCheck)
{
    if(!bypassAvailabilityCheck && !nextFrameAvailable()) return false;

    // Need to pick and select data out of buffer to build frame.
    bool successful = true;
    int currentReadPosChannelNumber = 0;

    for(auto &channelMapping : channelMappings) {
        int advanceAmount = channelMapping.originalChannelNumber - currentReadPosChannelNumber;

        // We know that channelMappings is sorted according by originalChannel, but sanity check
        assert(advanceAmount >= 0);
        if(advanceAmount > 0) {
            latestBlockMessage->advanceSeqReadPos(advanceAmount);
            currentReadPosChannelNumber += advanceAmount;
        }

        successful = latestBlockMessage->seqReadAndPut(buf + channelMapping.writtenChannelNumber, 1);
        // False = Error copying frame - frame might not have been ready.
        // This should never occur.
        // If you bypassAvailabilityCheck, you should have called nextFrameAvailable yourself prior to this call.
        assert(successful);
        if(!successful) break;

        currentReadPosChannelNumber++;
    }

    // Advance to the end of this data
    int advanceAmount = 64 - currentReadPosChannelNumber;
    // Sanity check; Make sure we haven't already exceeded the end of the message
    assert(advanceAmount >= 0);
    if(advanceAmount > 0) {
        latestBlockMessage->advanceSeqReadPos(advanceAmount);
    }

    return successful;
}

void EarVstCommunicator::sendAdmAndTrackMappings(std::string originalAdmStr, std::vector<uint32_t> trackMappingToAtu)
{
    auto resp = commandSocket.sendAdmAndMappings(originalAdmStr, trackMappingToAtu);
}

void EarVstCommunicator::infoExchange()
{
    auto resp = commandSocket.doCommand(commandSocket.Command::GetConfig);
    assert(resp->success());

    memcpy(&channelCount, (char*)resp->getBufferPointer() + 0, 1);
    memcpy(&sampleRate, (char*)resp->getBufferPointer() + 1, 4);
}

void EarVstCommunicator::admAndMappingExchange()
{
    auto resp = commandSocket.doCommand(commandSocket.Command::GetAdmAndMappings);
    assert(resp->success());

    auto bufPtr = (char*)resp->getBufferPointer();

    uint32_t mapSz = (64 * 4);
    uint32_t admSz = resp->getSize() - mapSz;

    auto mappings = std::vector<uint32_t>(64, 0x00000000);
    memcpy(mappings.data(), bufPtr, mapSz);
    auto admStrRecv = std::string(admSz, 0);
    memcpy(admStrRecv.data(), bufPtr + mapSz, admSz);

    auto latestChannelMappings = std::vector<ChannelMapping>();

    uint32_t atuVal = 0x00000000;
    uint8_t writtenChannelNum = 0;
    for(uint8_t originalChannelNum = 0; originalChannelNum < channelCount; originalChannelNum++) {
        atuVal = mappings[originalChannelNum];
        if(atuVal != 0x00000000) {
            latestChannelMappings.push_back(ChannelMapping{ originalChannelNum, writtenChannelNum, atuVal });
            writtenChannelNum++;
        }
    }

    channelMappings = latestChannelMappings;
    admStr = admStrRecv;
}

//HOA functions

std::string EarInputVst::hoaVstName = admplug::EARPluginSuite::HOA_METADATA_PLUGIN_NAME;
std::string EarInputVst::hoaVstCompName = "";
size_t EarInputVst::hoaVstCompNameLen = 0;
const char* EarInputVst::hoaVstCompNameCStr = nullptr;

const char* EarInputVst::getHoaVstCompName() {
    if (hoaVstCompNameCStr == nullptr) {
        hoaVstCompName = "VST3: ";
        hoaVstCompName.append(hoaVstName);
        hoaVstCompName.append(" (");
        hoaVstCompNameLen = hoaVstCompName.length();
        hoaVstCompNameCStr = hoaVstCompName.c_str();
    }
    return hoaVstCompNameCStr;
}

const std::string* EarInputVst::getHoaVstNameStr()
{
    return &hoaVstName;
}

bool EarInputVst::isHoaVstAvailable(ReaperAPI const& api, bool doRescan)
{
    return PluginRegistry::getInstance()->checkPluginAvailable(hoaVstName, api, doRescan);
}

int EarInputVst::trackHoaVstIndex(ReaperAPI const& api, MediaTrack* trk)
{
    return api.TrackFX_AddByName(trk, hoaVstName.c_str(), false, TrackFXAddMode::QueryPresence);
}

std::vector<int> EarInputVst::trackHoaVstIndexes(ReaperAPI const& api, MediaTrack* trk)
{
    int totalVsts = api.TrackFX_GetCount(trk);
    std::vector<int> vstPos;

    for (int i = 0; i < totalVsts; i++) {
        if (vstPosIsHoaVst(api, trk, i)) vstPos.push_back(i);
    }

    return vstPos;
}

bool EarInputVst::vstPosIsHoaVst(ReaperAPI const& api, MediaTrack* trk, int vstPos)
{
    getHoaVstCompName(); // Need to generate it to get length
    char name[100];
    if (!api.TrackFX_GetFXName(trk, vstPos, name, (int)hoaVstCompNameLen + 1)) return false;
    return (strcmp(name, getHoaVstCompName()) == 0);
}

bool EarInputVst::isHoaPlugin(std::string vstNameStr)
{
    // Name only
    if (vstNameStr == hoaVstName) return true;
    // Comparison name (includes VST3: and ends with company name and channel count)
    getHoaVstCompName(); // need to ensure generated
    if (strncmp(vstNameStr.c_str(), getHoaVstCompName(), hoaVstCompNameLen) == 0) return true;
    return false;
}