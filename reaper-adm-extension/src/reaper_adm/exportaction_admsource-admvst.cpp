#include "exportaction_admsource-admvst.h"

#include "pluginregistry.h"
#include <version/eps_version.h>

#include <adm/adm.hpp>
#include <adm/utilities/id_assignment.hpp>
#include <adm/write.hpp>
#include <adm/common_definitions.hpp>

namespace {
    std::vector<adm::TypeDescriptor> getAdmTypeDefinitionsExcluding(adm::TypeDescriptor exclude) {
        std::vector<adm::TypeDescriptor> otherTypes;
        if(exclude != adm::TypeDefinition::OBJECTS) otherTypes.push_back(adm::TypeDefinition::OBJECTS);
        if(exclude != adm::TypeDefinition::DIRECT_SPEAKERS) otherTypes.push_back(adm::TypeDefinition::DIRECT_SPEAKERS);
        if(exclude != adm::TypeDefinition::HOA) otherTypes.push_back(adm::TypeDefinition::HOA);
        if(exclude != adm::TypeDefinition::MATRIX) otherTypes.push_back(adm::TypeDefinition::MATRIX);
        if(exclude != adm::TypeDefinition::BINAURAL) otherTypes.push_back(adm::TypeDefinition::BINAURAL);
        return otherTypes;
    }
}

AdmVstExportSources::AdmVstExportSources(ReaperAPI const & api) : IExportSources(api)
{
    admDocument = adm::Document::create();
    admProgramme = adm::AudioProgramme::create(adm::AudioProgrammeName("Programme"));
    admContent = adm::AudioContent::create(adm::AudioContentName("Content"));

    adm::addCommonDefinitionsTo(admDocument);
    admDocument->add(admProgramme);
    admProgramme->addReference(admContent);

    MediaTrack *trk;
    int numTracks = api.CountTracks(nullptr);

    for(int trackNum = 0; trackNum < numTracks; trackNum++) {

        trk = api.GetTrack(nullptr, trackNum);
        if(trk) {

            auto fxPosVec = AdmVst::trackAdmVstIndexes(api, trk);
            for(int fxPos : fxPosVec) {
                auto admExportVst = std::make_shared<AdmVst>(trk, fxPos, api);
                allAdmVsts.push_back(admExportVst);

                if(AdmVst::isCandidateForExport(admExportVst)) {

                    char aoName[100];
                    std::string aoNameStr = "Audio Object";
                    if(api.GetTrackName(trk, aoName, 100)) {
                        aoNameStr = aoName;
                    }

                    auto bounds = api.getTrackAudioBounds(trk, true); // True = ignore before zero - we don't do sub-zero bounds

                    auto audioObject = adm::AudioObject::create(adm::AudioObjectName(aoNameStr));
                    if(bounds.has_value()) {
                        audioObject->set(adm::Start{ toNs((*bounds).first) });
                        audioObject->set(adm::Duration{ toNs((*bounds).second - (*bounds).first) });
                    }

                    admContent->addReference(audioObject);
                    // Construction generates ADM
                    auto candidate = std::make_shared<AdmVstExporter>(admExportVst, admContent, audioObject, api);
                    candidatesForExport.push_back(candidate);
                }
            }
        }
    }

    adm::reassignIds(admDocument); // Must be done before allowing to generate AXML/CHNA so align ID's

    updateErrorsWarningsInfo(api);
}

void AdmVstExportSources::updateErrorsWarningsInfo(ReaperAPI const & api)
{
    std::string op;
    op.append(std::to_string(candidatesForExport.size()));
    op.append(" (of ");
    op.append(std::to_string(allAdmVsts.size()));
    op.append(") instances of ");
    op.append(getExportSourcesName());
    op.append(" will export.");
    infoStrings.push_back(op);

    int notIncluded = 0;
    int offlineOrBypassed = 0;
    int knownSampleRate = 0;
    bool sampleRatesMatch = true;

    for(auto admVst : allAdmVsts) {
        int vstSampleRate = admVst->getSampleRate();
        int vstChannelCount = admVst->getChannelCount();

        if(!admVst->getIncludeInRenderState()) notIncluded++;

        if(admVst->isBypassed() || admVst->isPluginOffline()) {
            offlineOrBypassed++;
        } else {
            if(admVst->getIncludeInRenderState()) {
                if(vstSampleRate == 0) {
                    std::string msg("VST on track '");
                    char buf[32];
                    api.GetTrackName(admVst->getTrackInstance().get(), buf, 32);
                    msg.append(buf);
                    msg.append("' is not reporting sample rate");
                    warningStrings.push_back(msg);
                }
                if(admVst->getAdmTypeDefinition() == 0) {
                    std::string msg("VST on track '");
                    char buf[32];
                    api.GetTrackName(admVst->getTrackInstance().get(), buf, 32);
                    msg.append(buf);
                    msg.append("' has TypeDefinition 'Undefined'");
                    warningStrings.push_back(msg);
                }
                if(vstChannelCount == 0) {
                    std::string msg("VST on track '");
                    char buf[32];
                    api.GetTrackName(admVst->getTrackInstance().get(), buf, 32);
                    msg.append(buf);
                    msg.append("' is reporting that it will export zero channels of audio");
                    warningStrings.push_back(msg);
                }
                if(vstChannelCount > admVst->getTrackInstance().getChannelCount()) {
                    std::string msg("Track '");
                    char buf[32];
                    api.GetTrackName(admVst->getTrackInstance().get(), buf, 32);
                    msg.append(buf);
                    msg.append("' has too few channels for ADM essence type specified by VST");
                    warningStrings.push_back(msg);
                }
            }
        }

        if(knownSampleRate == 0) {
            knownSampleRate = vstSampleRate;
        } else if(knownSampleRate != vstSampleRate) {
            sampleRatesMatch = false;
        }
    }

    if(notIncluded > 0) {
        op = std::to_string(notIncluded);
        op.append((notIncluded == 1)? " instance does not have 'Include in Render' set" : " instances do not have 'Include in Render' set");
        infoStrings.push_back(op);
    }

    if(offlineOrBypassed > 0) {
        op = std::to_string(offlineOrBypassed);
        op.append((offlineOrBypassed == 1)? " instance is bypassed or offline" : " instances are bypassed or offline");
        infoStrings.push_back(op);
    }

    if(knownSampleRate == 0) errorStrings.push_back("Unable to determine sample rate from VSTs");
    if(!sampleRatesMatch) errorStrings.push_back("VSTs are reporting conflicting sample rates");

    for(auto& candidate : candidatesForExport) {
        for(auto& admAuthoringError : *(candidate->getAdmAuthoringErrors())) {
            warningStrings.push_back(admAuthoringError.what());
        }
    }


}

int AdmVstExportSources::getSampleRate()
{
    for(auto &thisVst : allAdmVsts) {
        int sr = thisVst->getSampleRate();
        if(sr > 0) return sr;
    }
    return 0;
}

int AdmVstExportSources::getTotalExportChannels()
{
    int numChns = 0;
    for (auto &candidate : candidatesForExport) {
        numChns += candidate->getPlugin()->getChannelCount();
    }
    return numChns;
}

void AdmVstExportSources::setRenderInProgress(bool state)
{
    for(auto &candidate : candidatesForExport) {
        candidate->setRenderInProgressState(true);
    }
}

bool AdmVstExportSources::isFrameAvailable()
{
    for (auto &candidate : candidatesForExport) {
        auto communicator = candidate->getCommunicator();
        if (!communicator || !communicator->nextFrameAvailable()) {
            return false;
        }
    }
    return true;
}

bool AdmVstExportSources::writeNextFrameTo(float * bufferWritePointer, bool skipFrameAvailableCheck)
{
    if(!skipFrameAvailableCheck && !isFrameAvailable()) return false;

    for (auto &candidate : candidatesForExport) {
        auto communicator = candidate->getCommunicator();
        if(communicator->getReportedChannelCount() > 0) {
            if(!communicator->copyNextFrame(bufferWritePointer, true)) {
                throw std::runtime_error("copyNextFrame failed");
            }
            bufferWritePointer += communicator->getReportedChannelCount();
        }
    }
    return true;
}

std::shared_ptr<bw64::AxmlChunk> AdmVstExportSources::getAxmlChunk()
{
    std::stringstream xmlStream;
    adm::writeXml(xmlStream, admDocument);
    xmlStream << "<!-- Produced using the EAR Production Suite (version ";
    xmlStream << (eps::versionInfoAvailable()? eps::currentVersion() : "unknown");
    xmlStream << "), from ADM Export Source plugins -->\n";
    return std::make_shared<bw64::AxmlChunk>(bw64::AxmlChunk(xmlStream.str()));
}

std::shared_ptr<bw64::ChnaChunk> AdmVstExportSources::getChnaChunk()
{
    using namespace adm;
    std::vector<bw64::AudioId> audioIds;

    // audioIds for CHNA chunk (must be done AFTER reassigning ADM IDs)
    int trackNumCounter = 1; // Tracks are 1-indexed!!!
    for (auto &metadata : candidatesForExport) {
        if(auto audioPackFormat = metadata->getAudioPackFormat()) {
            auto audioPackFormatIdStr = formatId(audioPackFormat->get<AudioPackFormatId>());
            for(auto& admTrack : *metadata->getAdmSubgraphs()) {
                audioIds.push_back(bw64::AudioId(trackNumCounter++,
                                                 formatId(admTrack->audioTrackUid->get<AudioTrackUidId>()),
                                                 formatId(admTrack->audioTrackFormat->get<AudioTrackFormatId>()),
                                                 audioPackFormatIdStr
                ));
            }
        }
    }

    return std::make_shared<bw64::ChnaChunk>(bw64::ChnaChunk(audioIds));
}




AdmVstExporter::AdmVstExporter(std::shared_ptr<AdmVst> admExportVst, std::shared_ptr<adm::AudioContent> parentContent, std::shared_ptr<adm::AudioObject> audioObject, ReaperAPI const& api) : admExportVst{ admExportVst }, audioObject{ audioObject }, parentContent{ parentContent }, parentDocument{ parentContent->getParent().lock() }
{
    assert(parentDocument);
    assignAdmMetadata(api);
}

AdmVstExporter::~AdmVstExporter()
{
}

bool AdmVstExporter::isCandidateForExport()
{
    // Does NOT check for correct spat plugins or spat plugin location at this stage!
    // - this is entirely an ADM Export Source VST check.
    // Spat plugin checks come during ADM metadata generation (assignAdmMetadata onwards)
    return AdmVst::isCandidateForExport(admExportVst);
}

void AdmVstExporter::assignAdmMetadata(ReaperAPI const & api)
{
    admAuthoringErrors.clear();

    if(admExportVst->getAdmTypeDefinition() == adm::TypeDefinition::UNDEFINED.get()) {
        // We can't include this in the render, because the type was Undefined in the ADM VST and we rely on the packformat to determine how many channels to write out
        std::string errorMessage{ "The " };
        errorMessage += *AdmVst::getVstNameStr();
        errorMessage += " plugin on track '";
        errorMessage += admExportVst->getTrackInstance().getName();
        errorMessage += "' is set to type 'Undefined'.";
        admAuthoringErrors.push_back(AdmAuthoringError(errorMessage));
        return;
    }

    auto thisType = admExportVst->getAdmType();
    auto otherTypes = getAdmTypeDefinitionsExcluding(thisType);

    auto supportingPlugins = getTypeSupportingPlugins(api, thisType);
    auto unsupportingPlugins = getTypeSupportingPlugins(api, otherTypes, &supportingPlugins);

    auto trackInst = admExportVst->getTrackInstance();
    std::pair<std::shared_ptr<PluginSuite>, std::shared_ptr<PluginInstance>>* chosenPlugin = nullptr;
    if(supportingPlugins.size() > 0) {
        chosenPlugin = &(supportingPlugins.begin()->second);
    }

    if(admExportVst->isUsingCommonDefinition()) {
        // Common definitions (directspeakers, hoa) may not need spatialising so it's acceptable to not provide a plugin

        if(chosenPlugin) {
            newAdmCommonDefinitionReference(api, chosenPlugin->first, chosenPlugin->second.get());
        } else {
            newAdmCommonDefinitionReference(api, nullptr, nullptr);
        }

    } else if(chosenPlugin){
        newAdmSubgraph(api, chosenPlugin->first, chosenPlugin->second.get());

    } else {
        // ERROR - Require a spatialisation plugin and none were present
        std::string errorMessage{ "Track '" };
        errorMessage += admExportVst->getTrackInstance().getName();
        errorMessage += "' has no supported spatialisation plugin for object type specified by ";
        errorMessage += *AdmVst::getVstNameStr();
        errorMessage += " plugin.";
        admAuthoringErrors.push_back(AdmAuthoringError(errorMessage));
        // We can still generate ADM - just no blocks - default initial block will be created
        newAdmSubgraph(api, nullptr, nullptr);
    }

    if(supportingPlugins.size() > 1) {
        // WARNING - Multiple spatalisation plugins on track - can not determine which to use! (we used first one)
        std::optional<std::string> pluginName = chosenPlugin ? chosenPlugin->second->getPluginName() : std::optional<std::string>();

        std::string errorMessage{ "Track '" };
        errorMessage += trackInst.getName();
        errorMessage += "' has multiple supported spatialisation plugins for object type specified by ";
        errorMessage += *AdmVst::getVstNameStr();
        errorMessage += " plugin. Using parameter data from first instance";
        if(pluginName.has_value()) {
            errorMessage += " of '";
            errorMessage += *pluginName;
            errorMessage += "'";
        }
        errorMessage += ".";
        admAuthoringErrors.push_back(AdmAuthoringError(errorMessage));
    }

    if(unsupportingPlugins.size() > 0) {
        // WARNING - Spatalisation plugins on track suitable for different objectType to that specified in ADM VST. They are ignored.
        std::string errorMessage{ "Track '" };
        errorMessage += admExportVst->getTrackInstance().getName();
        errorMessage += "' contains spatialisation plugins for a different object type to that specified by ";
        errorMessage += *AdmVst::getVstNameStr();
        errorMessage += " plugin.";
        admAuthoringErrors.push_back(AdmAuthoringError(errorMessage));
    }
}

std::shared_ptr<AdmSubgraphElements> AdmVstExporter::newAdmSubgraph(ReaperAPI const &api, std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance* spatPlugin, std::string suffix, adm::FormatDescriptor format)
{
    if (suffix.length() == 0) suffix = std::to_string(admSubgraphs.size());
    suffix.insert(0, "_");

    auto objectName = audioObject->get<adm::AudioObjectName>().get();
    auto objectType = admExportVst->getAdmType();

    if(!audioPackFormat) {
        // create packformat for this audioObject
        audioPackFormat = adm::AudioPackFormat::create(adm::AudioPackFormatName(objectName), admExportVst->getAdmType());
        audioObject->addReference(audioPackFormat);
    }

    auto subgraph = std::make_shared<AdmSubgraphElements>();
    admSubgraphs.push_back(subgraph);

    subgraph->audioChannelFormat = adm::AudioChannelFormat::create(adm::AudioChannelFormatName(objectName + suffix), objectType);
    subgraph->audioStreamFormat = adm::AudioStreamFormat::create(adm::AudioStreamFormatName(objectName + suffix), format);
    subgraph->audioTrackFormat = adm::AudioTrackFormat::create(adm::AudioTrackFormatName(objectName + suffix), format);
    subgraph->audioTrackUid = adm::AudioTrackUid::create();
    // References
    audioPackFormat->addReference(subgraph->audioChannelFormat);
    subgraph->audioStreamFormat->setReference(subgraph->audioChannelFormat);
    subgraph->audioTrackFormat->setReference(subgraph->audioStreamFormat);
    audioObject->addReference(subgraph->audioTrackUid);
    subgraph->audioTrackUid->setReference(subgraph->audioTrackFormat);
    subgraph->audioTrackUid->setReference(audioPackFormat);

    if(pluginSuite && spatPlugin) {
        checkPluginPositions(pluginSuite, spatPlugin);
    }
    createAndAddAudioBlocks(objectType, pluginSuite, spatPlugin, subgraph, api);

    return subgraph;
}

void AdmVstExporter::newAdmCommonDefinitionReference(ReaperAPI const & api, std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance* spatPlugin)
{
    auto typeDefinition = admExportVst->getAdmType();
    int typeDefinitionId = typeDefinition.get();
    int packFormatId = admExportVst->getAdmPackFormat();
    int channelFormatId = admExportVst->getAdmChannelFormat();

    assert(packFormatId != ADM_VST_PACKFORMAT_UNSET_ID);
    assert(packFormatId <= COMMONDEFINITIONS_MAX_ID);

    if(!audioPackFormat) {
        // Lookup packformat for this commondefinition
        auto allPackFormats = parentDocument->getElements<adm::AudioPackFormat>();
        for(auto packFormat : allPackFormats) {
            if(packFormat->get<adm::TypeDescriptor>() == typeDefinition &&
               packFormat->get<adm::AudioPackFormatId>().get<adm::AudioPackFormatIdValue>().get() == packFormatId) {
                audioPackFormat = packFormat;
                audioObject->addReference(audioPackFormat);
                break;
            }
        }
    }

    assert(audioPackFormat);

    if(audioPackFormat) {
        if(channelFormatId == ADM_VST_CHANNELFORMAT_ALLCHANNELS_ID) {
            // ALL channels for this packformat, in the order they are referenced in the packformat
            auto allTrackFormats = parentDocument->getElements<adm::AudioTrackFormat>();

            // We use the commonDefinitionHelper to get not only the Pack Formats immediate Channel Formats,
            //  but also those it may inherit if it references another Pack Format (which may also inherit another)
            auto pfData = AdmCommonDefinitionHelper::getSingleton()->getPackFormatData(typeDefinitionId, packFormatId);

            for(auto cfData : pfData->relatedChannelFormats) {

                auto cfId = adm::AudioChannelFormatId(typeDefinition, adm::AudioChannelFormatIdValue(cfData->id));
                auto channelFormat = parentDocument->lookup(cfId);
                assert(channelFormat);

                auto subgraph = std::make_shared<AdmSubgraphElements>();
                subgraph->audioChannelFormat = channelFormat;

                for(auto trackFormat : allTrackFormats) {
                    auto referencedStreamFormat = trackFormat->getReference<adm::AudioStreamFormat>();
                    if(referencedStreamFormat->getReference<adm::AudioChannelFormat>() == channelFormat) {
                        subgraph->audioStreamFormat = referencedStreamFormat;
                        subgraph->audioTrackFormat = trackFormat;
                        subgraph->audioTrackUid = adm::AudioTrackUid::create();
                        subgraph->audioTrackUid->setReference(subgraph->audioTrackFormat);
                        subgraph->audioTrackUid->setReference(audioPackFormat);
                        audioObject->addReference(subgraph->audioTrackUid);
                    }
                }

                admSubgraphs.push_back(subgraph);
            }

        } else {
            // Single channel - find it
            auto subgraph = std::make_shared<AdmSubgraphElements>();

            auto allTrackFormats = parentDocument->getElements<adm::AudioTrackFormat>();
            for(auto trackFormat : allTrackFormats) {
                auto referencedStreamFormat = trackFormat->getReference<adm::AudioStreamFormat>();
                auto referencedChannelFormat = referencedStreamFormat->getReference<adm::AudioChannelFormat>();

                if(referencedChannelFormat->get<adm::TypeDescriptor>() == typeDefinition &&
                   referencedChannelFormat->get<adm::AudioChannelFormatId>().get<adm::AudioChannelFormatIdValue>().get() == channelFormatId) {
                    subgraph->audioChannelFormat = referencedChannelFormat;
                    subgraph->audioStreamFormat = referencedStreamFormat;
                    subgraph->audioTrackFormat = trackFormat;
                    break;
                }
            }

            if(subgraph->audioChannelFormat) {
                subgraph->audioTrackUid = adm::AudioTrackUid::create();
                subgraph->audioTrackUid->setReference(subgraph->audioTrackFormat);
                subgraph->audioTrackUid->setReference(audioPackFormat);
                audioObject->addReference(subgraph->audioTrackUid);

                admSubgraphs.push_back(subgraph);
            }
        }
    }

    if(pluginSuite && spatPlugin) {
        // TODO checkPluginParamsMatchCommonDefinition(chosenPlugin, commondef);
        checkPluginPositions(pluginSuite, spatPlugin);
    }
}

TrackEnvelope * AdmVstExporter::getEnvelopeFor(std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance * pluginInst, AdmParameter admParameter, ReaperAPI const & api)
{
    MediaTrack* track = pluginInst->getTrackInstance().get();

    // Plugin Parameters
    if(auto param = pluginSuite->getPluginParameterFor(admParameter)) {
        return api.GetFXEnvelope(track, pluginInst->getPluginIndex(), param->index(), false);
    }

    // Track Parameters
    switch(admParameter) {
        case AdmParameter::OBJECT_GAIN:
            return api.GetTrackEnvelopeByName(track, "Volume");
            break;
    }

    return nullptr;
}

std::optional<double> AdmVstExporter::getValueFor(std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance * pluginInst, AdmParameter admParameter, ReaperAPI const & api)
{
    MediaTrack* track = pluginInst->getTrackInstance().get();

    // Plugin Parameters
    if(auto param = pluginSuite->getPluginParameterFor(admParameter)) {
        return pluginInst->getParameterWithConvert(*param);
    }

    // Track Parameters
    double val;
    switch(admParameter) {
        case AdmParameter::OBJECT_GAIN:
            if(api.GetTrackUIVolPan(track, &val, nullptr)) {
                return std::optional<double>(val);
            }
            break;
    }
    return std::optional<double>();
}

bool AdmVstExporter::checkPluginPositions(std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance * pluginInst)
{
    auto spatialisationPluginName = pluginInst->getPluginName();
    assert(spatialisationPluginName);

    if(spatialisationPluginName) {
        if(pluginSuite->pluginSuiteRequiresAdmExportVst() == PluginSuite::RequiresAdmExportVst::NONE) {
            std::string errorMessage{ "Plugin '" };
            errorMessage += *spatialisationPluginName;
            errorMessage += "' on track '";
            errorMessage += admExportVst->getTrackInstance().getName();
            errorMessage += "' does not require an instance of  ";
            errorMessage += *AdmVst::getVstNameStr();
            errorMessage += " plugin.";
            admAuthoringErrors.push_back(AdmAuthoringError(errorMessage));
            return false;
        } else if(pluginSuite->pluginSuiteRequiresAdmExportVst() == PluginSuite::RequiresAdmExportVst::PRE_SPATIALISATION &&
                  pluginInst->getPluginIndex() < admExportVst->getPluginIndex()) {
            std::string errorMessage{ "The " };
            errorMessage += *AdmVst::getVstNameStr();
            errorMessage += " plugin on track '";
            errorMessage += admExportVst->getTrackInstance().getName();
            errorMessage += "' should be placed BEFORE '";
            errorMessage += *spatialisationPluginName;
            errorMessage += "'.";
            admAuthoringErrors.push_back(AdmAuthoringError(errorMessage));
            return false;
        } else if(pluginSuite->pluginSuiteRequiresAdmExportVst() == PluginSuite::RequiresAdmExportVst::POST_SPATIALISATION &&
                  pluginInst->getPluginIndex() > admExportVst->getPluginIndex()) {
            std::string errorMessage{ "The " };
            errorMessage += *AdmVst::getVstNameStr();
            errorMessage += "' plugin on track '";
            errorMessage += admExportVst->getTrackInstance().getName();
            errorMessage += "' should be placed AFTER '";
            errorMessage += *spatialisationPluginName;
            errorMessage += "'.";
            admAuthoringErrors.push_back(AdmAuthoringError(errorMessage));
            return false;
        }
        return true;
    }
    else {
        std::string errorMessage{ "Spatialisation plugin on track '" };
        errorMessage += admExportVst->getTrackInstance().getName();
        errorMessage += "' could not be determined.";
        admAuthoringErrors.push_back(AdmAuthoringError(errorMessage));
        return false;
    }
}

void AdmVstExporter::createAndAddAudioBlocks(adm::TypeDescriptor typeDescriptor, std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance * pluginInst, std::shared_ptr<AdmSubgraphElements> subgraph, ReaperAPI const & api)
{
    auto start = std::chrono::nanoseconds::zero();
    auto duration = toNs(api.GetProjectLength(nullptr));

    if(audioObject->has<adm::Start>()) {
        start = audioObject->get<adm::Start>().get().asNanoseconds();
        duration -= start;
    }
    if(audioObject->has<adm::Duration>()) {
        duration = audioObject->get<adm::Duration>().get().asNanoseconds();
    }

    auto cumulatedPointData = CumulatedPointData(start, start + duration);

    if(pluginSuite && pluginInst) {
        // Get all values for all parameters, whether automated or not.
        for(int admParameterIndex = 0; admParameterIndex != (int)AdmParameter::NONE; admParameterIndex++) {
            auto admParameter = (AdmParameter)admParameterIndex;
            auto param = pluginSuite->getParameterFor(admParameter);
            auto env = getEnvelopeFor(pluginSuite, pluginInst, admParameter, api);

            if(param && env) {
                // We have an envelope for this ADM parameter
                auto newErrors = cumulatedPointData.useEnvelopeDataForParameter(*env, *param, admParameter, api);
                admAuthoringErrors.insert(admAuthoringErrors.end(), newErrors.begin(), newErrors.end());

            } else if(auto val = getValueFor(pluginSuite, pluginInst, admParameter, api)) {
                // We do not have an envelope for this ADM parameter but the plugin suite CAN provide a fixed value for it
                // NOTE that this will include parameters NOT relevant to the current audioObject type, but these are ignored during block creation.
                auto newErrors = cumulatedPointData.useConstantValueForParameter(admParameter, *val);
                admAuthoringErrors.insert(admAuthoringErrors.end(), newErrors.begin(), newErrors.end());
            }
        }
    }

    if(typeDescriptor == adm::TypeDefinition::OBJECTS) {
        addBlockFormatsToChannelFormat(cumulatedPointData.generateAudioBlockFormatObjects(pluginSuite, pluginInst, api), subgraph);
    }
    else if(typeDescriptor == adm::TypeDefinition::DIRECT_SPEAKERS) {
        // TODO - no need to support just yet as we only support common definitions and objects in the ADM VST
    }
    else if(typeDescriptor == adm::TypeDefinition::HOA) {
        // TODO - no need to support just yet as we only support common definitions and objects in the ADM VST
    }
    else if(typeDescriptor == adm::TypeDefinition::BINAURAL) {
        // TODO - no need to support just yet as we only support common definitions and objects in the ADM VST
    }
    else if(typeDescriptor == adm::TypeDefinition::MATRIX) {
        // TODO - no need to support just yet as we only support common definitions and objects in the ADM VST
    }

}

template<typename AudioBlockFormatType>
void AdmVstExporter::addBlockFormatsToChannelFormat(std::optional<std::vector<std::shared_ptr<AudioBlockFormatType>>> blocks, std::shared_ptr<AdmSubgraphElements> subgraph) {
    assert(blocks.has_value());
    blockCount = (*blocks).size();
    for(auto& block : *blocks) {
        subgraph->audioChannelFormat->add(*block);
    }
}

std::map<int, std::pair<std::shared_ptr<PluginSuite>, std::shared_ptr<PluginInstance>>> AdmVstExporter::getTypeSupportingPlugins(const ReaperAPI &api, adm::TypeDescriptor typeDef, std::map<int, std::pair<std::shared_ptr<PluginSuite>, std::shared_ptr<PluginInstance>>>* ignorePlugins)
{
    std::map<int, std::pair<std::shared_ptr<PluginSuite>, std::shared_ptr<PluginInstance>>> pluginIndices;
    auto pluginSuites = PluginRegistry::getInstance()->getPluginSuites();

    std::map<std::string, std::shared_ptr<admplug::PluginSuite>>::iterator it;
    for(it = pluginSuites->begin(); it != pluginSuites->end(); it++) {

        std::shared_ptr<admplug::PluginSuite> pluginSuite = it->second;
        auto spatialisationPluginNameForType = pluginSuite->getSpatialisationPluginNameFor(typeDef);

        auto pluginMatches = admExportVst->getTrackInstance().getPlugins(*spatialisationPluginNameForType);
        for(auto& plugin : pluginMatches) {
            if(auto pluginInst = dynamic_cast<PluginInstance*>(plugin.get())) {
                int index = pluginInst->getPluginIndex();
                bool shouldIgnore = ignorePlugins ? (ignorePlugins->find(index) != ignorePlugins->end()) : false;
                if(!shouldIgnore) {
                    auto pluginInstShare = std::make_shared<PluginInstance>(pluginInst->getTrackInstance().get(), pluginInst->getPluginIndex(), api);
                    auto suitePluginPair = std::make_pair(pluginSuite, pluginInstShare);
                    pluginIndices.insert(std::make_pair(index, suitePluginPair));
                }
            }
        }
    }

    return pluginIndices;
}

std::map<int, std::pair<std::shared_ptr<PluginSuite>, std::shared_ptr<PluginInstance>>> AdmVstExporter::getTypeSupportingPlugins(const ReaperAPI &api, std::vector<adm::TypeDescriptor> typeDefs, std::map<int, std::pair<std::shared_ptr<PluginSuite>, std::shared_ptr<PluginInstance>>>* ignorePlugins)
{
    std::map<int, std::pair<std::shared_ptr<PluginSuite>, std::shared_ptr<PluginInstance>>> resultsList{};

    for(auto& typeDef : typeDefs) {
        auto newResults = getTypeSupportingPlugins(api, typeDef, ignorePlugins);
        resultsList.insert(newResults.begin(), newResults.end());
    }

    return resultsList;
}

AdmVstCommunicator* AdmVstExporter::getCommunicator(bool mustExist) {
    if(mustExist && !isCommunicatorPresent()) obtainCommunicator();
    return communicator.get();
}

bool AdmVstExporter::isCommunicatorPresent() {
    return (bool)communicator;
}

void AdmVstExporter::obtainCommunicator() {
    auto samplesPort = admExportVst->getSamplesSocketPort();
    auto commandPort = admExportVst->getCommandSocketPort();
    if(isCommunicatorPresent()) releaseCommunicator();
    communicator = CommunicatorRegistry::getCommunicator<AdmVstCommunicator>(samplesPort, commandPort);
}

void AdmVstExporter::releaseCommunicator() {
    communicator.reset();
}

bool AdmVstExporter::getRenderInProgressState() {
    if(!isCommunicatorPresent()) return false;
    return communicator->getRenderingState();
}

void AdmVstExporter::setRenderInProgressState(bool state) {
    if(state == true && !isCommunicatorPresent()) obtainCommunicator();
    if(isCommunicatorPresent()) communicator->setRenderingState(state);
}

AdmVstCommunicator::AdmVstCommunicator(int samplesPort, int commandPort) : CommunicatorBase(samplesPort, commandPort)
{
    updateInfo();
}

void AdmVstCommunicator::updateInfo()
{
    infoExchange();
}

void AdmVstCommunicator::infoExchange()
{
    auto resp = commandSocket.doCommand(commandSocket.Command::GetConfig);
    assert(resp->success());

    memcpy(&channelCount, (char*)resp->getBufferPointer() + 0, 1);
    memcpy(&sampleRate, (char*)resp->getBufferPointer() + 1, 4);
    memcpy(&admTypeDefinition, (char*)resp->getBufferPointer() + 5, 2);
    memcpy(&admPackFormatId, (char*)resp->getBufferPointer() + 7, 2);
    memcpy(&admChannelFormatId, (char*)resp->getBufferPointer() + 9, 2);

    infoReceived = true;
}