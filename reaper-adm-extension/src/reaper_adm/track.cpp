#include "reaperapi.h"
#include "parameter.h"
#include "track.h"
#include "envelopecreator.h"
#include "automationenvelope.h"
#include "projectelements.h"
#include "plugin.h"

using namespace admplug;

TrackInstance::TrackInstance(MediaTrack *track, ReaperAPI const& api) : api{api}, track{track}, guid{api.GetTrackGUID(track)}
{
    if(!track || !api.ValidatePtr(track, "MediaTrack*")) {
        throw std::runtime_error("Invalid MediaTrack pointer");
    }
}

std::unique_ptr<AutomationEnvelope> TrackInstance::getEnvelope(const TrackParameter &parameter, EnvelopeCreator const& creator) const
{
    if(parameter.type() == TrackParameterType::VOLUME) {
        api.activateAndShowTrackVolumeEnvelope(track);
        auto trackEnvelope = api.GetTrackEnvelopeByName(track, "Volume");
        // Need to force "Amplitude" scaling, rather than "Fader" scaling (since ADM uses linear amplitude interpolation, not log)
        bool res = api.forceAmplitudeScaling(trackEnvelope);
        assert(res);
        return creator.create(trackEnvelope, api);
    }
    return nullptr;
}

std::unique_ptr<Plugin> TrackInstance::createPlugin(std::string pluginName)
{
    return std::make_unique<PluginInstance>(get(), pluginName, true, api);
}

int admplug::TrackInstance::deletePlugin(std::string pluginName, bool allInstances)
{
    int deleteCount = 0;
    while(true){
        int fxNum = api.TrackFX_AddByName(get(), pluginName.c_str(), false, TrackFXAddMode::QueryPresence);
        if(fxNum < 0) break;
        auto deleteSuccess = api.TrackFX_Delete(get(), fxNum);
        if(!deleteSuccess) break;
        deleteCount++;
        if(!allInstances) break;
    }
    return deleteCount;
}

std::unique_ptr<Plugin> TrackInstance::getPlugin(std::string pluginName)
{
    auto index = api.TrackFX_AddByName(get(), pluginName.c_str(), false, TrackFXAddMode::QueryPresence);
    if(index < 0) {
        return nullptr;
    }
    return std::make_unique<PluginInstance>(get(), pluginName, false, api);
}

std::unique_ptr<Plugin> admplug::TrackInstance::getPlugin(int index)
{
    if(index < 0 || index >= api.TrackFX_GetCount(track)) return std::unique_ptr<Plugin>();
    return std::make_unique<PluginInstance>(track, index, api);
}

std::vector<std::unique_ptr<Plugin>> admplug::TrackInstance::getPlugins(std::string pluginName)
{
    std::vector<std::unique_ptr<Plugin>> pluginMatches;
    // We know the name is followed by a space and opening bracket for the company - means we can do a stricter comparison
    pluginName += " (";

    auto pluginCount = api.TrackFX_GetCount(get());
    for(int pluginIndex = 0; pluginIndex < pluginCount; pluginIndex++) {
        char cName[100];
        if(api.TrackFX_GetFXName(get(), pluginIndex, cName, 100)) {
            // Remove "VST3:/ VST:" from start of plugin name
            std::string cNameStr{ cName };
            if(cNameStr.rfind("VST3: ", 0) == 0) { // Starts with
                cNameStr.erase(0, 6);
            } else if(cNameStr.rfind("VST: ", 0) == 0) { // Starts with
                cNameStr.erase(0, 5);
            }
            if(cNameStr.rfind(pluginName, 0) == 0) { // Starts with
                pluginMatches.push_back(std::make_unique<PluginInstance>(get(), pluginIndex, api));
            }
        }
    }
    return std::move(pluginMatches);
}

std::vector<std::unique_ptr<Plugin>> admplug::TrackInstance::getPlugins()
{
    std::vector<std::unique_ptr<Plugin>> pluginMatches;

    auto pluginCount = api.TrackFX_GetCount(get());
    for(int pluginIndex = 0; pluginIndex < pluginCount; pluginIndex++) {
        pluginMatches.push_back(std::make_unique<PluginInstance>(get(), pluginIndex, api));
    }
    return std::move(pluginMatches);
}

void TrackInstance::setParameter(const TrackParameter &parameter, double value) const
{
    if(parameter.type() == TrackParameterType::VOLUME) {
        api.SetMediaTrackInfo_Value(track, "D_VOL", value);
    }
}

void TrackInstance::setAsVCASlave(const TrackGroup &group)
{
    api.setAsVCAGroupSlave(get(), group.id());
}

void TrackInstance::setAsVCAMaster(const TrackGroup &group)
{
    api.setAsVCAGroupMaster(get(), group.id());
}

void TrackInstance::setColor(const Color color) {
    api.SetTrackColor(get(), api.ColorToNative(color.red, color.green, color.blue));
}

MediaTrack *TrackInstance::get() const
{
    return track;
}

bool TrackInstance::stillExists() const
{
    if(!api.ValidatePtr(track, "MediaTrack*")) {
        return false;
    }

    auto currentGuid = ReaperGUID{api.GetTrackGUID(track)};
    return guid == currentGuid;
}

void TrackInstance::setChannelCount(int channelCount)
{
    api.setTrackChannelCount(get(), channelCount);
}

void TrackInstance::moveToBefore(int index)
{
    api.moveTrackToBeforeIndex(get(), index);
}

void TrackInstance::disableMasterSend()
{
    api.disableTrackMasterSend(get());
}

std::string admplug::TrackInstance::getName()
{
    char nameBuff[32];
    bool res = api.GetTrackName(track, nameBuff, 32);
    assert(res);
    return res? std::string(nameBuff) : std::string();
}

void TrackInstance::setName(std::string name)
{
    api.setTrackName(get(), name);
}

void TrackInstance::hideFromTrackControlPanel()
{
    api.SetMediaTrackInfo_Value(get(), "B_SHOWINTCP", 0.0);
}

bool admplug::TrackInstance::isPluginChainBypassed() const
{
    return (api.GetMediaTrackInfo_Value(track, "I_FXEN") == 0.0);
}

int admplug::TrackInstance::getChannelCount() const
{
    return (int)api.GetMediaTrackInfo_Value(track, "I_NCHAN");
}

void TrackInstance::route(Track &other, int channelCount, int firstSourceChannel, int firstDestinationChannel)
{
    api.RouteTrackToTrack(get(), firstSourceChannel, other.get(), firstDestinationChannel, channelCount, PostFaderPostFx);
}

void Track::routeTo(Track& other, int channelCount, int firstSourceChannel, int firstDestinationChannel) {
    this->route(other, channelCount, firstSourceChannel, firstDestinationChannel);
}

bool admplug::Track::trackPresent(Track * track)
{
    return (track && track->stillExists());
}
