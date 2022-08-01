#pragma once

#include <chrono>
#include <memory>
#include <vector>
#include <adm/elements/audio_block_format_objects.hpp>
#include <adm/elements/audio_channel_format.hpp>
#include <adm/element_variant.hpp>
#include "admchannel.h"
#include "color.h"

class PCM_source;
class MediaTrack;
class MediaItem;

namespace admplug {
class ReaperAPI;
class PluginSuite;
class PluginParameter;
class TrackParameter;
class Plugin;
class Track;

class ProjectElement {
public:
    virtual ~ProjectElement() = default;
    virtual void createProjectElements(PluginSuite& pluginSuite, ReaperAPI const& api) = 0;
    virtual bool hasAdmElement(adm::ElementConstVariant element) const;
    virtual bool hasAdmElements(std::vector<adm::ElementConstVariant> elements) const;
    virtual bool followsAdmElementSequence(std::vector<adm::ElementConstVariant> elements) const;
    virtual bool addParentProjectElement(std::shared_ptr<ProjectElement> newParentElement);

private:
    virtual std::vector<adm::ElementConstVariant> getAdmElements() const = 0;
};


class RootElement : public ProjectElement {
public:
    virtual ~RootElement() = default;
    virtual double startTime() const = 0;
    virtual double startOffset() const = 0;
};

class TrackGroup {
public:
    TrackGroup(int id);
    int id() const;
    Color color() const;
private:
    int idNum{0};
};

class TrackElement : public ProjectElement {
public:
    virtual ~TrackElement() = default;
    virtual std::shared_ptr<Track> getTrack() const = 0; // try and get rid?
    virtual MediaItem* addMediaItem(ReaperAPI const& api) = 0;
    virtual std::vector<TrackGroup> slaveOfGroups() const = 0;
    virtual TrackGroup masterOfGroup() const = 0;
    bool addParentProjectElement(std::shared_ptr<ProjectElement> newParentElement) override;
protected:
    std::vector<std::shared_ptr<TrackElement>> parentElements;
};

class TakeElement : public ProjectElement {
public:
    virtual ~TakeElement() = default;
    virtual void setSource(PCM_source*) = 0;
    virtual double startTime() const = 0;
    std::shared_ptr<TrackElement const> parentTrack() const { return parent; }
    virtual void addChannel(ADMChannel channel) = 0;
    virtual bool hasChannel(ADMChannel channel) = 0;
    virtual std::vector<ADMChannel> channels() const = 0;
    virtual void setChannels(std::vector<ADMChannel> channels) = 0;
    bool addParentProjectElement(std::shared_ptr<ProjectElement> newParentElement) override;
protected:
    std::shared_ptr<TrackElement> parent;
};

class AutomationElement : public ProjectElement {
public:
    virtual ~AutomationElement() = default;
    virtual double startTime() const = 0;
    virtual std::shared_ptr<Track> getTrack() const = 0;
    virtual std::shared_ptr<TakeElement const> parentTake() const { return parent; }
    virtual ADMChannel channel() const = 0;
    virtual void apply(PluginParameter const& parameter, Plugin const& plugin) const = 0;
    virtual void apply(TrackParameter const& parameter, Track const& track) const = 0;
protected:
    std::shared_ptr<TakeElement> parent;
};

class ObjectAutomation : public AutomationElement {
public:
    virtual ~ObjectAutomation() = default;
    virtual adm::BlockFormatsConstRange<adm::AudioBlockFormatObjects> blocks() const = 0;
    bool addParentProjectElement(std::shared_ptr<ProjectElement> newParentElement) override;
};

class DirectSpeakersAutomation : public AutomationElement {
public:
    virtual ~DirectSpeakersAutomation() = default;
    virtual adm::BlockFormatsConstRange<adm::AudioBlockFormatDirectSpeakers> blocks() const = 0;
    virtual std::vector<ADMChannel> takeChannels() const = 0;
    virtual int channelIndex() const = 0;
    bool addParentProjectElement(std::shared_ptr<ProjectElement> newParentElement) override;
};

class HoaAutomation : public AutomationElement {
public:
    virtual ~HoaAutomation() = default;
    virtual adm::BlockFormatsConstRange<adm::AudioBlockFormatHoa> blocks() const = 0;
    virtual std::vector<ADMChannel> takeChannels() const = 0;
    virtual int channelIndex() const = 0;
    bool addParentProjectElement(std::shared_ptr<ProjectElement> newParentElement) override;
};

}
