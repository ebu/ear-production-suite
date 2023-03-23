#pragma once
#include <memory>
#include <string>
#include <vector>
#include "reaperguid.h"
#include "reaperapi.h"
#include "color.h"

class MediaTrack;

namespace admplug {

class Plugin;
class PluginParameter;
class TrackParameter;
class ReaperAPI;
class EnvelopeCreator;
class AutomationEnvelope;
class TrackGroup;
class TrackInstance;

class Track {
public:
    virtual ~Track() = default;
    virtual std::unique_ptr<AutomationEnvelope> getEnvelope(TrackParameter const& parameter, EnvelopeCreator const& creator) const = 0;
    virtual std::unique_ptr<Plugin> createPlugin(std::string pluginName) = 0;
    virtual int deletePlugin(std::string pluginName, bool allInstances = true) = 0;
    virtual std::unique_ptr<Plugin> getPlugin(std::string pluginName) = 0; // TODO - get rid - this doesn't catch multiple instances, which we should always be checking for
    virtual std::vector<std::unique_ptr<Plugin>> getPlugins(std::string pluginName) = 0;
    virtual void setParameter(TrackParameter const& parameter, double value) const = 0;
    virtual void setAsVCASlave(TrackGroup const& group) = 0;
    virtual void setAsVCAMaster(const TrackGroup &group) = 0;
    virtual void setColor(const Color color) = 0;
    virtual MediaTrack* get() const = 0;
    virtual bool stillExists() const = 0;
    virtual void setChannelCount(int channelCount) = 0;
    virtual int getChannelCount() const = 0;
    virtual void moveToBefore(int index) = 0;
    virtual void disableMasterSend() = 0;
    virtual std::string getName() = 0;
    virtual void setName(std::string name) = 0;
    virtual void hideFromTrackControlPanel() = 0;
    virtual bool isPluginChainBypassed() const = 0;
    void routeTo(Track& other, int channelCount = 1, int firstSourceChannel = 0, int firstDestinationChannel = 0);
    static bool trackPresent(Track* track);
protected:
    virtual void route(Track& other, int channelCount, int firstSourceChannel, int firstDestinationChannel) = 0;
};

class TrackInstance : public Track {
public:
    explicit TrackInstance(MediaTrack* track, ReaperAPI const& api);
    std::unique_ptr<AutomationEnvelope> getEnvelope(TrackParameter const& parameter, EnvelopeCreator const& creator) const override;
    std::unique_ptr<Plugin> createPlugin(std::string pluginName) override;
    int deletePlugin(std::string pluginName, bool allInstances = true) override;
    std::unique_ptr<Plugin> getPlugin(std::string pluginName) override;
    std::vector<std::unique_ptr<Plugin>> getPlugins(std::string pluginName) override;
    void setParameter(TrackParameter const& parameter, double value) const override;
    void setAsVCASlave(TrackGroup const& group) override;
    void setAsVCAMaster(const TrackGroup &group) override;
    void setColor(const Color color) override;
    MediaTrack* get() const override;
    bool stillExists() const override;
    void setChannelCount(int channelCount) override;
    int getChannelCount() const override;
    void moveToBefore(int index) override;
    void disableMasterSend() override;
    std::string getName() override;
    void setName(std::string name) override;
    void hideFromTrackControlPanel() override;
    bool isPluginChainBypassed() const override;

protected:
    void route(Track &other, int channelCount, int firstSourceChannel, int firstDestinationChannel) override;

private:
    ReaperAPI const& api;
    MediaTrack* track;
    ReaperGUID guid;
};
}

