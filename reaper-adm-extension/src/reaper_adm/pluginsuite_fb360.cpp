#include "pluginsuite_fb360.h"

#include <string>
#include <sstream>

#include <math.h>
#include "pluginsuite_fb360.h"
#include "pluginsuite.h"
#include "projectelements.h"
#include "reaperapi.h"
#include "pluginregistry.h"
#include "plugin.h"
#include "parameter.h"
#include "projectnode.h"
#include "color.h"
#include "admtraits.h"
#include "commontrackpool.h"
#include "admvstcontrol.h"
#include "mediatrackelement.h"

#include <memory>
#include <stdexcept>
using namespace admplug;

namespace {

    enum class FB360ObjectParameters {
        AZIMUTH = 0,
        ELEVATION,
        DISTANCE,
        NUM_PARAMETERS
    };

    std::vector<std::unique_ptr<PluginParameter>> createAutomatedObjectPluginParameters()
    {
        std::vector<std::unique_ptr<PluginParameter>> parameters;

        auto azimuthRange = ParameterRange{ -180.0, 180.0 };
        auto azimuthMap = map::sequence({ map::wrap(azimuthRange),
                                          map::invert(),
                                          map::normalise(azimuthRange) });
        parameters.push_back(createPluginParameter(static_cast<int>(FB360ObjectParameters::AZIMUTH),
                                                   AdmParameter::OBJECT_AZIMUTH,
                                                   azimuthMap));
        parameters.push_back(createPluginParameter(static_cast<int>(FB360ObjectParameters::AZIMUTH),
                                                   AdmParameter::SPEAKER_AZIMUTH,
                                                   azimuthMap));

        auto elevationRange = ParameterRange{ -90.0, 90.0 };
        auto elevationMap = map::sequence({ map::normalise(elevationRange),
                                            map::clip() });
        parameters.push_back(createPluginParameter(static_cast<int>(FB360ObjectParameters::ELEVATION),
                                                   AdmParameter::OBJECT_ELEVATION,
                                                   elevationMap));
        parameters.push_back(createPluginParameter(static_cast<int>(FB360ObjectParameters::ELEVATION),
                                                   AdmParameter::SPEAKER_ELEVATION,
                                                   elevationMap));

        auto distanceRange = ParameterRange{ 0.0, 60.0 };
        auto distanceMap = map::sequence({ map::normalise(distanceRange),
                                           map::clip() });
        parameters.push_back(createPluginParameter(static_cast<int>(FB360ObjectParameters::DISTANCE),
                                                   AdmParameter::OBJECT_DISTANCE,
                                                   distanceMap));
        parameters.push_back(createPluginParameter(static_cast<int>(FB360ObjectParameters::DISTANCE),
                                                   AdmParameter::SPEAKER_DISTANCE,
                                                   distanceMap));

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

    void configureAdmExportVst(const AutomationElement& element, const Track& track, const ReaperAPI &api, int packFormatIdValueOverride = 0) {
        auto packFormat = element.channel().packFormat();
        if (packFormat && AdmVst::isAvailable(api, false)) {
            auto vst = AdmVst(track.get(), api); // Will attempt to wrap existing plugin or create new if missing
            vst.setIncludeInRenderState(true);

            auto td = packFormat->get<adm::TypeDescriptor>();
            vst.setAdmTypeDefinition(td.get());
            auto packFormatIdValue = packFormatIdValueOverride;
            if(packFormatIdValueOverride <= 0) {
                // Not overridden - pull it out of the pack
                packFormatIdValue = packFormat->get<adm::AudioPackFormatId>().get<adm::AudioPackFormatIdValue>().get();
            }

            // Set PackFormat and ChannelFormat. Only supporting CommonDefinitions. Set defaults if not CommonDefinition.
            vst.setAdmPackFormat(packFormatIdValue > COMMONDEFINITIONS_MAX_ID ? ADM_VST_PACKFORMAT_UNSET_ID : packFormatIdValue);
            vst.setAdmChannelFormat(ADM_VST_CHANNELFORMAT_ALLCHANNELS_ID);
        }
    }

    int getHoaOrder(std::shared_ptr<adm::AudioPackFormat const> packFormat) {
        int maxOrder = -1;
        for(auto const& pf : packFormat->getReferences<adm::AudioPackFormat>()) {
            int pfMaxOrder = getHoaOrder(pf);
            if(pfMaxOrder > maxOrder) maxOrder = pfMaxOrder;
        }
        for(auto const& cf : packFormat->getReferences<adm::AudioChannelFormat>()) {
            for(auto const& bf : cf->getElements<adm::AudioBlockFormatHoa>()) {
                if(bf.has<adm::Order>()) {
                    int bfOrder = bf.get<adm::Order>().get();
                    if(bfOrder > maxOrder) maxOrder = bfOrder;
                }


            }
        }
        return maxOrder;
    }

}
const char* Facebook360PluginSuite::OBJECT_METADATA_PLUGIN_NAME = "FB360 Spatialiser (ambiX)";
const char* Facebook360PluginSuite::RENDERER_PLUGIN_NAME = "FB360 Control (ambiX)";

bool Facebook360PluginSuite::registered = PluginRegistry::getInstance()->registerSupportedPluginSuite("Facebook 360", std::make_shared<Facebook360PluginSuite>());

Facebook360PluginSuite::Facebook360PluginSuite() : busTrack3D{ nullptr }, controlTrack{ nullptr }
{
}

Facebook360PluginSuite::~Facebook360PluginSuite() = default;

bool Facebook360PluginSuite::pluginSuiteUsable(ReaperAPI const& api)
{
    std::vector<std::string> requiredPlugins{
        std::string(OBJECT_METADATA_PLUGIN_NAME),
        std::string(RENDERER_PLUGIN_NAME)
    };
    return PluginRegistry::getInstance()->checkPluginsAvailable(requiredPlugins, api);
}

void Facebook360PluginSuite::onCreateObjectTrack(const TrackElement & trackNode, ReaperAPI const& api)
{
   auto& track = *trackNode.getTrack();
   doGenericTrackSetup(track);
   AdmVst(track.get(), api); // Creates ADM VST before spatialiser
   setupMetadataPlugin(track, api);
   setAsGroupsSlave(trackNode, track);
}

void Facebook360PluginSuite::setupMetadataPlugin(Track& track, ReaperAPI const& api)
{
   track.createPlugin(OBJECT_METADATA_PLUGIN_NAME);
   track.routeTo(*busTrack3D, 16);
//   api.activateAndShowTrackVolumeEnvelope(track.get());
}

void Facebook360PluginSuite::setAsGroupsSlave(TrackElement const& element, Track& track)
{
    auto groups = element.slaveOfGroups();
    for (auto& group : groups) {
        track.setAsVCASlave(group);
    }
    if(!groups.empty()){
        track.setColor(groups.front().color()); // TODO - what if this belongs to more than one group?
    }
}

PluginParameter* admplug::Facebook360PluginSuite::getPluginParameterFor(AdmParameter admParameter)
{
    for(auto& param : automatedObjectPluginParameters()) {
        if(param->admParameter() == admParameter) {
            return param.get();
        }
    }
    return nullptr;
}

TrackParameter * admplug::Facebook360PluginSuite::getTrackParameterFor(AdmParameter admParameter)
{
    for(auto& param : trackParameters()) {
        if(param->admParameter() == admParameter) {
            return param.get();
        }
    }
    return nullptr;
}

Parameter* admplug::Facebook360PluginSuite::getParameterFor(AdmParameter admParameter)
{
    Parameter* param = getTrackParameterFor(admParameter);
    if(param) return param;
    return getPluginParameterFor(admParameter);
}

std::optional<std::string> admplug::Facebook360PluginSuite::getSpatialisationPluginNameForObjects()
{
    return std::optional<std::string>(Facebook360PluginSuite::OBJECT_METADATA_PLUGIN_NAME);
}

std::optional<std::string> admplug::Facebook360PluginSuite::getSpatialisationPluginNameForDirectSpeakers()
{
    return std::optional<std::string>(Facebook360PluginSuite::OBJECT_METADATA_PLUGIN_NAME);
}

std::optional<std::string> admplug::Facebook360PluginSuite::getSpatialisationPluginNameForHoa()
{
    return std::optional<std::string>(Facebook360PluginSuite::OBJECT_METADATA_PLUGIN_NAME);
}

void Facebook360PluginSuite::doGenericTrackSetup(Track& track) {
    track.moveToBefore(trackInsertionIndex++);
    track.setChannelCount(16);
    track.disableMasterSend();
}

void Facebook360PluginSuite::onCreateDirectTrack(const TrackElement &trackNode, const ReaperAPI &)
{
    auto& track = *trackNode.getTrack();
    track.moveToBefore(trackInsertionIndex++);
    track.disableMasterSend();
    setAsGroupsSlave(trackNode, track);
}

void Facebook360PluginSuite::onCreateGroup(const TrackElement &trackNode, const ReaperAPI &)
{
    auto& track = *trackNode.getTrack();
    doGenericTrackSetup(track);
    setAsGroupsSlave(trackNode, track);
    track.setAsVCAMaster(trackNode.masterOfGroup());
    track.setColor(trackNode.masterOfGroup().color());
}

void Facebook360PluginSuite::onCreateProject(const ProjectNode&, const ReaperAPI &api)
{
    if (!Track::trackPresent(controlTrack.get())) {
        controlTrack = getControlTrack(api);
    }

    if (!Track::trackPresent(controlTrack.get())) {
        controlTrack = api.createTrack();
        controlTrack->hideFromTrackControlPanel();
        controlTrack->setChannelCount(16);
        controlTrack->setName("3D MASTER");
        controlTrack->createPlugin(RENDERER_PLUGIN_NAME);
    }

    trackInsertionIndex = api.getTrackIndexOfSelectedMediaItem();
    if (trackInsertionIndex <= 0) trackInsertionIndex = 1;

    busTrack3D = api.createTrack();
    busTrack3D->moveToBefore(trackInsertionIndex++);
    busTrack3D->hideFromTrackControlPanel();
    busTrack3D->setChannelCount(16);
    busTrack3D->setName("3D Sub-Master");
    busTrack3D->disableMasterSend();
    busTrack3D->routeTo(*controlTrack, 16);

    if(!commonTracks) {
        commonTracks = std::make_unique<CommonTrackPool>(api);
    }
    commonTracks->removeDeletedTracks();
}

void Facebook360PluginSuite::onObjectAutomation(const ObjectAutomation &automationNode, const ReaperAPI &api)
{
    auto& track = *automationNode.getTrack();
    configureAdmExportVst(automationNode, track, api);
    auto plugin = track.getPlugin(OBJECT_METADATA_PLUGIN_NAME);
    if(plugin) {
        applyParameters(automationNode, track, *plugin);
    }
}

void Facebook360PluginSuite::onDirectSpeakersAutomation(const DirectSpeakersAutomation & directAutomation, const ReaperAPI &api)
{
    auto track = directAutomation.getTrack();
    auto take = directAutomation.parentTake();
    auto trackWidth = static_cast<int>(take->trackUidCount());
    track->setChannelCount(trackWidth);

    auto firstBlock = directAutomation.blocks().front();
    if(isCommonDefinition(firstBlock)) {
        int fxNum = api.TrackFX_AddByName(track->get(), AdmVst::getVstNameStr()->c_str(), false, TrackFXAddMode::QueryPresence);
        if(fxNum < 0) {
            // Not yet configured ADM Export VST
            configureAdmExportVst(directAutomation, *track, api);
        }
        if(directAutomation.channel().channelFormat()) {
            auto& busTrack = getCommonDefinitionTrack(directAutomation, api);
            track->routeTo(busTrack, 1, directAutomation.channelIndex(), 0);
        }

    } else {
        // TODO: Warn user - We don't support non-common-definition
    }
}

void Facebook360PluginSuite::onHoaAutomation(const HoaAutomation & hoaAutomation, const ReaperAPI &api){
    // NOTE: This is going to get called once for each channel in a HOA AudioObject! Make sure it doesn't duplicate actions

    auto track = hoaAutomation.getTrack();

    // Check whether we already processed this track - no point duplicating effort
    /// Need to check that the plugin hasn't already been removed (already processed and was unsuccessful)
    int fxNum = api.TrackFX_AddByName(track->get(), OBJECT_METADATA_PLUGIN_NAME, false, TrackFXAddMode::QueryPresence);
    if(fxNum < 0) return;
    /// Need to check that a preset hasn't already been applied (already processed and was successful)
    char presetname[255];
    if(api.TrackFX_GetPreset(track->get(), fxNum, presetname, 255)) {
        auto presetLen = std::string(presetname).length();
        if(presetLen > 0) return; // Already processed this track
    };

    if(applyFXPreset(hoaAutomation, api)) {

        // Configure plug-ins
        if(isCommonDefinition(hoaAutomation.blocks().front())) {
            configureAdmExportVst(hoaAutomation, *track, api);

        } else {
            // If it wasn't a common definition, we can't use the ADM Export VST directly on the source.
            // However the FB360 spat plugin outputs SN3D ACN, which are in common definitions!
            // We can use this but we need to move the ADM Export VST post-FB360.
            track->deletePlugin(AdmVst::getVstNameStr()->c_str(), true);

            // Find an appropriate common definition - FB360 does not upmix (even after adjusting pitch/yaw/roll), so use order according to incoming channel count;
            // PackIdValues 1 to 6 are SN3D ACN - matches FB360 plug-in output (intermediate format)
            int hoaOrder = getHoaOrder(hoaAutomation.channel().packFormat());
            if(hoaOrder < 1) hoaOrder = 1;
            if(hoaOrder > 3) hoaOrder = 3;
            configureAdmExportVst(hoaAutomation, *track, api, hoaOrder); // Creates a new instance
        }

    } else {
        // If we were unable to configure the spatialiser, should remove track sends and remove the plugins to avoid confusing the user
        // - this matches the behaviour of plugin suites which don't support HOA at all (e.g, VISR, not inserting plugins in the first place)
        track->deletePlugin(OBJECT_METADATA_PLUGIN_NAME, true);
        track->deletePlugin(AdmVst::getVstNameStr()->c_str(), true);
        while(api.GetTrackNumSends(track->get(), 0) > 0) {
            if(!api.RemoveTrackSend(track->get(), 0, 0)) break;
        }
    }
}

void Facebook360PluginSuite::onCreateHoaTrack(const TrackElement &trackNode, const ReaperAPI &api){
    auto& track = *trackNode.getTrack();
    doGenericTrackSetup(track);
    AdmVst(track.get(), api); // Creates ADM VST before spatialiser
    setupMetadataPlugin(track, api);
    setAsGroupsSlave(trackNode, track);
}
void Facebook360PluginSuite::applyParameters(ObjectAutomation const& element,
                                             Track const& track,
                                             Plugin const& plugin) const {
    for(auto& parameter : automatedObjectPluginParameters()) {
        element.apply(*parameter, plugin);
      }

    for(auto& parameter : trackParameters()) {
        element.apply(*parameter, track);
    }
}

std::vector<std::unique_ptr<PluginParameter>> const & admplug::Facebook360PluginSuite::automatedObjectPluginParameters() const
{
    auto static parameters = createAutomatedObjectPluginParameters();
    return parameters;
}

std::vector<std::unique_ptr<TrackParameter>> const & Facebook360PluginSuite::trackParameters() const {
    auto static parameters = createTrackParameters();
    return parameters;
}

Track& Facebook360PluginSuite::getCommonDefinitionTrack(const DirectSpeakersAutomation& element, const ReaperAPI &api)
{
    auto onCreate = [&element, this, &api] (Track& newTrack) {
        doGenericTrackSetup(newTrack);
        setupMetadataPlugin(newTrack, api);
        auto plugin = newTrack.getPlugin(OBJECT_METADATA_PLUGIN_NAME);
        assert(plugin);
        for(auto& parameter : automatedObjectPluginParameters()) {
            element.apply(*parameter, *plugin);
        }
    };

    return commonTracks->trackFor(element.channel(), onCreate);
}

std::unique_ptr<Track> Facebook360PluginSuite::getControlTrack(const ReaperAPI &api) {
    // We could cache this in a static, but that wouldn't work for saved projects that are reopened.
    return api.firstTrackWithPluginNamed(RENDERER_PLUGIN_NAME);
}

bool Facebook360PluginSuite::applyFXPreset(const HoaAutomation & hoaAutomation, const ReaperAPI &api){
    auto track = hoaAutomation.getTrack();
    auto hoaOrder = getHoaOrder(hoaAutomation.channel().packFormat());
    auto channelFormat = hoaAutomation.channel().channelFormat();
    if(!channelFormat) return false;
    auto blocks = channelFormat->getElements<adm::AudioBlockFormatHoa>();
    if(blocks.size() == 0) return false;
    auto normalization = blocks.front().get<adm::Normalization>();
    auto preset = "ambix1stOrder";  //Default according to BS.2076

    if(hoaOrder == 1 && normalization == "FuMa"){
        preset = "Fuma1stOrder";
    } else if(hoaOrder == 1 && (normalization == "SN3D" || normalization == "N3D")){
        preset = "ambix1stOrder";
    }  else if(hoaOrder == 2 && (normalization == "SN3D" || normalization == "N3D")){
        preset = "ambix2ndOrder";
    } else {
        // FB360 does not support other formats
        // TODO: Warn user
        assert(false);
        return false;
    }

    int fxNum = api.TrackFX_AddByName(track->get(), OBJECT_METADATA_PLUGIN_NAME, false, TrackFXAddMode::QueryPresence);
    std::vector<std::string> pathOptions;

#ifdef WIN32
    // UserPlugins could be in one of two locations on Windows!
    if(auto pathPrefix = api.GetResourcePath()) // This is normally something like "C:\Users\username\AppData\Roaming\REAPER"
        pathOptions.push_back(std::string(pathPrefix) + "\\UserPlugins\\ADMPresets\\FB360\\");
    if(auto pathPrefix = api.GetExePath()) // This is normally something like "C:\Program Files\REAPER (x64)"
        pathOptions.push_back(std::string(pathPrefix) + "\\Plugins\\ADMPresets\\FB360\\");
#else
    if(auto pathPrefix = api.GetResourcePath()) // This is something like "/Users/username/Library/Application Support/REAPER"
        pathOptions.push_back(std::string(pathPrefix) + "/UserPlugins/ADMPresets/FB360/");
#endif

    for(auto& pathOption : pathOptions) {
        std::string fullPath = pathOption + preset + ".vstpreset";
        if(api.TrackFX_SetPreset(track->get(), fxNum, fullPath.c_str())) {
            return true;
        }
    }

    // TODO: Warn user - Preset not applied
    assert(false);
    return false;
}
