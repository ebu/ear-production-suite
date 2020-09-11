#pragma once

class TrackEnvelope;

namespace admplug {

class PluginParameter;
class TrackParameter;
class ReaperAPI;

class Parametised {
public:
    virtual ~Parametised() = default;
    virtual TrackEnvelope* getEnvelope(PluginParameter const& parameter, ReaperAPI const& api) const = 0;
    virtual TrackEnvelope* getEnvelope(TrackParameter const& parameter, ReaperAPI const& api) const = 0;
    virtual void setParameter(PluginParameter const& parameter, double value, ReaperAPI const& api) const = 0;
    virtual void setParameter(TrackParameter const& parameter, double value, ReaperAPI const& api) const = 0;
};
}

