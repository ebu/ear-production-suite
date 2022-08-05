#include "pluginsuite_visr.h"

#include "reaperapi.h"
#include "reaperapivalues.h"
#include "pluginregistry.h"
#include "track.h"
#include "plugin.h"
#include "parameter.h"
#include "admtraits.h"
#include "commontrackpool.h"
#include "admvstcontrol.h"

using namespace admplug;

#define OBJECT_ID_MIN 1
#define OBJECT_ID_MAX 64

namespace {

    enum class VisrObjectParameters {
        OBJECT_ID = 0,
        AZIMUTH,
        ELEVATION,
        DISTANCE,
        NUM_PARAMETERS
    };

    std::vector<std::unique_ptr<PluginParameter>> createAutomatedObjectPluginParameters()
    {
        std::vector<std::unique_ptr<PluginParameter>> parameters;

        auto azimuthRange = ParameterRange{ -180.0, 180.0 };
        auto azimuthMap = map::sequence({map::wrap(azimuthRange),
                                         map::normalise(azimuthRange)});
        parameters.push_back(createPluginParameter(static_cast<int>(VisrObjectParameters::AZIMUTH),
                                                   AdmParameter::OBJECT_AZIMUTH,
                                                   azimuthMap));
        parameters.push_back(createPluginParameter(static_cast<int>(VisrObjectParameters::AZIMUTH),
                                                   AdmParameter::SPEAKER_AZIMUTH,
                                                   azimuthMap));

        auto elevationRange = ParameterRange{-90.0, 90.0};
        auto elevationMap = map::sequence({map::normalise(elevationRange),
                                           map::clip(elevationRange)});
        parameters.push_back(createPluginParameter(static_cast<int>(VisrObjectParameters::ELEVATION),
                                                   AdmParameter::OBJECT_ELEVATION,
                                                   elevationMap));
        parameters.push_back(createPluginParameter(static_cast<int>(VisrObjectParameters::ELEVATION),
                                                   AdmParameter::SPEAKER_ELEVATION,
                                                   elevationMap));

        auto distanceRange = ParameterRange{0.01, 100.0};
        auto distanceMap = map::sequence({map::normalise(distanceRange),
                                          map::clip(distanceRange)});
        parameters.push_back(createPluginParameter(static_cast<int>(VisrObjectParameters::DISTANCE),
                                                   AdmParameter::OBJECT_DISTANCE,
                                                   elevationMap));
        parameters.push_back(createPluginParameter(static_cast<int>(VisrObjectParameters::DISTANCE),
                                                   AdmParameter::SPEAKER_DISTANCE,
                                                   elevationMap));

        return parameters;
    }

    std::vector<std::unique_ptr<TrackParameter>> createTrackParameters() {
        std::vector<std::unique_ptr<TrackParameter>> parameters;
        auto gainRange = ParameterRange{0.0, 4.0};
        parameters.push_back(createTrackParameter(TrackParameterType::VOLUME,
                                                  AdmParameter::OBJECT_GAIN,
                                                  map::clip(gainRange)));
        return parameters;
    }

    void configureAdmExportVst(const AutomationElement& element, const Track& track, const ReaperAPI &api) {
        auto packFormat = element.channel().packFormat();
        if (packFormat && AdmVst::isAvailable(api, false)) {
            auto vst = AdmVst(track.get(), api); // Will attempt to wrap existing plugin or create new if missing
            vst.setIncludeInRenderState(true);

            auto td = packFormat->get<adm::TypeDescriptor>();
            vst.setAdmTypeDefinition(td.get());

            // Set PackFormat and ChannelFormat. Only supporting CommonDefinitions. Set defaults if not CommonDefinition.
            auto packFormatId = packFormat->get<adm::AudioPackFormatId>().get<adm::AudioPackFormatIdValue>().get();
            vst.setAdmPackFormat(packFormatId > COMMONDEFINITIONS_MAX_ID ? ADM_VST_PACKFORMAT_UNSET_ID : packFormatId);
            vst.setAdmChannelFormat(ADM_VST_CHANNELFORMAT_ALLCHANNELS_ID);
        }
    }

    std::vector<int> determineUsedObjectId(PluginInstance& plugin) {
        auto param = createPluginParameter(static_cast<int>(VisrObjectParameters::OBJECT_ID), {OBJECT_ID_MIN, OBJECT_ID_MAX});
        auto objectId = plugin.getParameterWithConvertToInt(*(param.get()));
        assert(objectId.has_value());
        if(objectId.has_value()) {
            return std::vector<int>{ *objectId };
        }
        return std::vector<int>{};
    }

}
const char* VisrPluginSuite::OBJECT_METADATA_PLUGIN_NAME = "VISR ObjectEditor";
const char* VisrPluginSuite::SCENEMASTER_PLUGIN_NAME = "VISR SceneMaster";
const char* VisrPluginSuite::RENDERER_PLUGIN_NAME = "VISR LoudspeakerRenderer";
const int VisrPluginSuite::MAX_CHANNEL_COUNT = 64;

bool VisrPluginSuite::registered = PluginRegistry::getInstance()->registerSupportedPluginSuite("VISR", std::make_shared<VisrPluginSuite>());

VisrPluginSuite::VisrPluginSuite() : objectIdParameter{ createPluginParameter(static_cast<int>(VisrObjectParameters::OBJECT_ID), ParameterRange{OBJECT_ID_MIN, OBJECT_ID_MAX}) }
{
}

VisrPluginSuite::~VisrPluginSuite()
{
}

bool VisrPluginSuite::pluginSuiteUsable(ReaperAPI const& api)
{
    std::vector<std::string> requiredPlugins{
        OBJECT_METADATA_PLUGIN_NAME, // The VISR plugin names prior to v0.11 have VST appended in the cache file
        SCENEMASTER_PLUGIN_NAME,     // so will no longer work with this pluginSuite
        RENDERER_PLUGIN_NAME
    };
    return PluginRegistry::getInstance()->checkPluginsAvailable(requiredPlugins, api);
}

PluginParameter* admplug::VisrPluginSuite::getPluginParameterFor(AdmParameter admParameter)
{
    for(auto& param : automatedObjectPluginParameters()) {
        if(param->admParameter() == admParameter) {
            return param.get();
        }
    }
    return nullptr;
}

TrackParameter * admplug::VisrPluginSuite::getTrackParameterFor(AdmParameter admParameter)
{
    for(auto& param : trackParameters()) {
        if(param->admParameter() == admParameter) {
            return param.get();
        }
    }
    return nullptr;
}

Parameter* admplug::VisrPluginSuite::getParameterFor(AdmParameter admParameter)
{
    Parameter* param = getTrackParameterFor(admParameter);
    if(param) return param;
    return getPluginParameterFor(admParameter);
}

std::optional<std::string> admplug::VisrPluginSuite::getSpatialisationPluginNameForObjects()
{
    return std::optional<std::string>(VisrPluginSuite::OBJECT_METADATA_PLUGIN_NAME);
}

std::optional<std::string> admplug::VisrPluginSuite::getSpatialisationPluginNameForDirectSpeakers()
{
    return std::optional<std::string>(VisrPluginSuite::OBJECT_METADATA_PLUGIN_NAME);
}

void VisrPluginSuite::setupTrackWithMetadataPlugin(Track& track, ReaperAPI const& api)
{
    doGenericTrackSetup(track);

    assert(objectIdAssigner);
    auto objectId = objectIdAssigner->getNextAvailableValue();

    if(objectId.has_value()) {
        auto plugin = track.createPlugin(OBJECT_METADATA_PLUGIN_NAME);
        plugin->setParameter(*objectIdParameter, objectIdParameter->forwardMap(*objectId));
        track.routeTo(*busTrack3D, 1, 0, *objectId - 1); //Object IDs are 1-based, channels are 0-based

    } else {
        // TODO - need to warn user - no free object ID's available
    }

}

void VisrPluginSuite::onCreateObjectTrack(TrackElement &trackNode, ReaperAPI const& api)
{
    auto& track = *trackNode.getTrack();
    AdmVst(track.get(), api); // Creates ADM VST before spatialiser
    setupTrackWithMetadataPlugin(track, api);

    auto groups = trackNode.slaveOfGroups();
    for(auto& group : groups) {
        track.setAsVCASlave(group);
    }
}

void VisrPluginSuite::onCreateDirectTrack(TrackElement &trackNode, const ReaperAPI &)
{
    auto track = trackNode.getTrack();
    track->moveToBefore(trackInsertionIndex++);
    track->disableMasterSend();
    for(auto group : trackNode.slaveOfGroups()) {
        track->setAsVCASlave(group);
    }
}

void VisrPluginSuite::doGenericTrackSetup(Track& track)
{
    track.moveToBefore(trackInsertionIndex++);
    track.setChannelCount(1);
    track.disableMasterSend();
}

void VisrPluginSuite::onCreateGroup(TrackElement &trackNode, const ReaperAPI &)
{
    auto& track = *trackNode.getTrack();
    doGenericTrackSetup(track);
    auto groups = trackNode.slaveOfGroups();
    for (auto& group : groups) {
        track.setAsVCASlave(group);
    }
    track.setAsVCAMaster(trackNode.masterOfGroup());
}

void VisrPluginSuite::onCreateProject(const ProjectNode &, const ReaperAPI &api)
{

    objectIdAssigner = std::make_unique<UniqueValueAssigner>(UniqueValueAssigner::SearchCandidate{ OBJECT_METADATA_PLUGIN_NAME, determineUsedObjectId }, OBJECT_ID_MIN, OBJECT_ID_MAX, api);
    trackInsertionIndex = api.getTrackIndexOfSelectedMediaItem();
    if (trackInsertionIndex <= 0) trackInsertionIndex = 1;

    busTrack3D = std::make_unique<TrackInstance>(api.createTrackAtIndex(trackInsertionIndex++), api);
    busTrack3D->hideFromTrackControlPanel();
    busTrack3D->setChannelCount(MAX_CHANNEL_COUNT);
    busTrack3D->setName("3D MASTER");
    auto masterTrack = api.masterTrack();
    if(!masterTrack->getPlugin(SCENEMASTER_PLUGIN_NAME)) {
        masterTrack->createPlugin(SCENEMASTER_PLUGIN_NAME);
    }
    busTrack3D->createPlugin(RENDERER_PLUGIN_NAME);
    if(!commonTracks) {
        commonTracks = std::make_unique<CommonTrackPool>(api);
    }
    commonTracks->removeDeletedTracks();
}

Track& VisrPluginSuite::getCommonDefinitionTrack(const DirectSpeakersAutomation& element, const ReaperAPI &api)
{
    auto onCreate = [&element, this, &api] (Track& newTrack) {
        setupTrackWithMetadataPlugin(newTrack, api);
        auto plugin = newTrack.getPlugin(OBJECT_METADATA_PLUGIN_NAME);
        if(plugin) {
            for(auto& parameter : automatedObjectPluginParameters()) {
                element.apply(*parameter, *plugin);
            }
        }
    };
    return commonTracks->trackFor(element.channel(), onCreate);
}

void VisrPluginSuite::onObjectAutomation(const ObjectAutomation &automationNode, const ReaperAPI &api)
{
    auto& track = *automationNode.getTrack();
    configureAdmExportVst(automationNode, track, api);
    auto plugin = track.getPlugin(OBJECT_METADATA_PLUGIN_NAME);
    if(plugin) {
      for(auto& parameter : automatedObjectPluginParameters()) {
          automationNode.apply(*parameter, *plugin);
      }
      for(auto& parameter : trackParameters()) {
          automationNode.apply(*parameter, track);
      }
    }
}

void VisrPluginSuite::onDirectSpeakersAutomation(DirectSpeakersAutomation const& automationNode, ReaperAPI const& api) {
    auto track = automationNode.getTrack();
    auto take = automationNode.parentTake();
    auto trackWidth = static_cast<int>(take->trackUidCount());
    track->setChannelCount(trackWidth);
    configureAdmExportVst(automationNode, *track, api);
    auto firstBlock = automationNode.blocks().front();

    if(automationNode.channel().channelFormat()) {
        if(isCommonDefinition(firstBlock)) {
            auto& commonTrack = getCommonDefinitionTrack(automationNode, api);
            track->routeTo(commonTrack, 1, automationNode.channelIndex());

        } else {
            // Note: ADM VST currently has no support for non-common-def
            auto channel = automationNode.channel();
            auto channelTrack = api.createTrack();
            channelTrack->setName(channel.name());
            setupTrackWithMetadataPlugin(*channelTrack, api);
            auto plugin = track->getPlugin(OBJECT_METADATA_PLUGIN_NAME);
            if(plugin) {
                for(auto& parameter : automatedObjectPluginParameters()) {
                    automationNode.apply(*parameter, *plugin);
                }
                track->routeTo(*channelTrack, 1, automationNode.channelIndex());
            }
        }
    }
}

void VisrPluginSuite::onHoaAutomation(const HoaAutomation &, const ReaperAPI &api){

}

void VisrPluginSuite::onCreateHoaTrack(TrackElement &trackNode, const ReaperAPI &api){
    auto track = trackNode.getTrack();
    track->moveToBefore(trackInsertionIndex++);
    track->disableMasterSend();
    for(auto group : trackNode.slaveOfGroups()) {
        track->setAsVCASlave(group);
    }
}

std::vector<std::unique_ptr<PluginParameter>> const& VisrPluginSuite::automatedObjectPluginParameters()
{
    auto static parameters = createAutomatedObjectPluginParameters();
    return parameters;
}

std::vector<std::unique_ptr<TrackParameter>> const& VisrPluginSuite::trackParameters() {
    auto static parameters = createTrackParameters();
    return parameters;
}

bool VisrPluginSuite::applyFXPreset(const HoaAutomation &, const ReaperAPI &api) {
    return true;
}
