#pragma once
#include <string>
#include <optional>
#include "track.h"

namespace admplug {

class AutomationEnvelope;
class EnvelopeCreator;

class Plugin {
public:
    virtual ~Plugin() = default;
    virtual std::unique_ptr<AutomationEnvelope> getEnvelope(PluginParameter const& parameter, EnvelopeCreator const& creator) const = 0;
    virtual void setParameter(PluginParameter const& parameter, double value) const = 0;
    virtual std::optional<double> getParameter(const PluginParameter &parameter) const = 0;
};

class PluginInstance : public Plugin {
public:
    PluginInstance(MediaTrack *mediaTrack, std::string pluginName, bool shouldInsert, const ReaperAPI &api);
    PluginInstance(MediaTrack *mediaTrack, int pluginIndex, const ReaperAPI &api);
    std::unique_ptr<AutomationEnvelope> getEnvelope(PluginParameter const& parameter, EnvelopeCreator const& creator) const;
    void setParameter(PluginParameter const& parameter, double value) const;
    void setParameterWithConvert(PluginParameter const& parameter, double value) const;
    std::optional<double> getParameter(const PluginParameter &parameter) const;
    std::optional<double> getParameterWithConvert(const PluginParameter &parameter) const;
    std::optional<int> getParameterWithConvertToInt(const PluginParameter &parameter) const;
    int getPluginIndex() const;
    std::optional<std::string> getPluginName();
    bool stillExists() const;
    TrackInstance& getTrackInstance();
    bool isPluginBypassed() const;
    bool isPluginOffline() const;
    bool isBypassed() const;

protected:
    PluginInstance(MediaTrack *mediaTrack, const ReaperAPI &api);
    TrackInstance track;
    std::string name;
    ReaperAPI const& api;
    std::unique_ptr<ReaperGUID> guid;

};
}
