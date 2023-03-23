#include <stdexcept>
#include <cassert>
#include <cmath>
#include "plugin.h"
#include "reaperapi.h"
#include "parameter.h"
#include "envelopecreator.h"
#include "automationenvelope.h"

using namespace admplug;

PluginInstance::PluginInstance(MediaTrack* mediaTrack, std::string pluginName, bool shouldInsert, ReaperAPI const& api) : track{mediaTrack, api}, name{pluginName}, api{api}
{
    auto index = api.TrackFX_AddByActualName(mediaTrack, pluginName.c_str(), false, shouldInsert? TrackFXAddMode::CreateNew : TrackFXAddMode::QueryPresence);
    if(index < 0) {
        throw std::runtime_error("Could not add to or get plugin from track");
    }
    guid = std::make_unique<ReaperGUID>(api.TrackFX_GetFXGUID(mediaTrack, index));
}

admplug::PluginInstance::PluginInstance(MediaTrack * mediaTrack, int pluginIndex, const ReaperAPI & api) : track{mediaTrack, api}, api{api}
{
    if(!api.TrackFX_GetActualFXName(mediaTrack, pluginIndex, name)) {
        throw std::runtime_error("Plugin index appears to be invalid");
    }
    guid = std::make_unique<ReaperGUID>(api.TrackFX_GetFXGUID(mediaTrack, pluginIndex));
}

admplug::PluginInstance::PluginInstance(MediaTrack* mediaTrack, const ReaperAPI & api) : track{ mediaTrack, api }, api{ api }
{
    // To be called only by child classes.
    // Child classes MUST set the following members in their constructor;
    //   guid
    //   name
}

std::unique_ptr<AutomationEnvelope> PluginInstance::getEnvelope(const PluginParameter &parameter, EnvelopeCreator const& creator) const
{
    auto trackEnvelope = api.getPluginEnvelope(track.get(), guid->get(), parameter.index());
    assert(trackEnvelope);
    return creator.create(trackEnvelope, api);
}

void PluginInstance::setParameter(const PluginParameter &parameter, double value) const
{
    auto index = getPluginIndex();
    if(!(index < 0)) {
      api.TrackFX_SetParam(track.get(), index, parameter.index(), value);
    }
}

void admplug::PluginInstance::setParameterWithConvert(PluginParameter const & parameter, double value) const
{
    setParameter(parameter, parameter.forwardMap(value));
}

std::optional<double> PluginInstance::getParameter(const PluginParameter &parameter) const {
    auto index = getPluginIndex();
    if(index < 0) {
        return std::optional<double>{};
    }
    auto value = api.TrackFX_GetParamNormalized(track.get(), index, parameter.index());
    if(value < 0) {
        return std::optional<double>{};
    }
    return value;
}

std::optional<double> admplug::PluginInstance::getParameterWithConvert(const PluginParameter & parameter) const
{
    auto raw = getParameter(parameter);
    if(!raw.has_value()) return raw;
    return std::optional<double>(parameter.reverseMap(*raw));
}

std::optional<int> admplug::PluginInstance::getParameterWithConvertToInt(const PluginParameter & parameter) const
{
    auto optVal = getParameterWithConvert(parameter);
    if(!optVal.has_value()) return optVal;
    int val = (int)std::round(*optVal);
    return std::optional<int>(val);
}

int admplug::PluginInstance::getPluginIndex() const
{
    assert(guid);
    int numFx = api.TrackFX_GetCount(track.get());
    for(int fxNum = 0; fxNum < numFx; fxNum++) {
        auto fxGuid = api.TrackFX_GetFXGUID(track.get(), fxNum);
        if(ReaperGUID(fxGuid) == *guid) {
            return fxNum;
        }
    }
    return -1;
}

std::optional<std::string> admplug::PluginInstance::getPluginName()
{
    std::string pluginName;
    if(api.TrackFX_GetActualFXName(getTrackInstance().get(), getPluginIndex(), pluginName))
        return pluginName;
    return std::optional<std::string>();
}

bool admplug::PluginInstance::stillExists() const
{
    return getPluginIndex() >= 0;
}

TrackInstance& admplug::PluginInstance::getTrackInstance()
{
    return track;
}

bool admplug::PluginInstance::isPluginBypassed() const
{
    auto index = getPluginIndex();
    assert(index >= 0);
    return !api.TrackFX_GetEnabled(track.get(), index);
}

bool admplug::PluginInstance::isPluginOffline() const
{
    auto index = getPluginIndex();
    assert(index >= 0);
    return api.TrackFX_GetOffline(track.get(), index);
}

bool admplug::PluginInstance::isBypassed() const
{
    return isPluginBypassed() || track.isPluginChainBypassed();
}
