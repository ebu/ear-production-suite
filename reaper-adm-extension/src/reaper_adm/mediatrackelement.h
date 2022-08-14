#pragma once
#include "projectelements.h"
#include <functional>
#include <adm/element_variant.hpp>
#include <array>

namespace admplug {

class MediaTrackElement : public TrackElement {
public:
    MediaTrackElement(std::vector<adm::ElementConstVariant> admElements, std::shared_ptr<TrackElement> parentTrack = nullptr, std::unique_ptr<TrackGroup> = nullptr);
    virtual void createProjectElements(PluginSuite& pluginSuite, ReaperAPI const& api) override;
    MediaItem* addMediaItem(ReaperAPI const& api) override;
    std::vector<TrackGroup> slaveOfGroups() const override;
    TrackGroup masterOfGroup() const override;
    virtual void firePluginSuiteCallback(PluginSuite& pluginSuite, ReaperAPI const& api) = 0;
    void nameTrackFromElementName();
    std::string getAppropriateName();
protected:
    std::vector<adm::ElementConstVariant> getAdmElements() const override;
    std::vector<adm::ElementConstVariant> elements;
    std::unique_ptr<TrackGroup> groupMaster;
};

class ObjectTrack : public MediaTrackElement {
public:
    ObjectTrack(std::vector<adm::ElementConstVariant> admElements, std::shared_ptr<TrackElement> parentTrack);
    void firePluginSuiteCallback(PluginSuite& pluginSuite, ReaperAPI const& api) override;
private:
};

class DirectTrack : public MediaTrackElement {
public:
    DirectTrack(std::vector<adm::ElementConstVariant> admElements, std::shared_ptr<TrackElement> parentTrack);
    void firePluginSuiteCallback(PluginSuite& pluginSuite, ReaperAPI const& api) override;
private:
};

class HoaTrack : public MediaTrackElement {
public:
    HoaTrack(std::vector<adm::ElementConstVariant> admElements, std::shared_ptr<TrackElement> parentTrack);
    void firePluginSuiteCallback(PluginSuite& pluginSuite, ReaperAPI const& api) override;
private:
};

class GroupTrack : public MediaTrackElement {
public:
    GroupTrack(std::vector<adm::ElementConstVariant> admElements, std::shared_ptr<TrackElement> parentTrack, std::unique_ptr<TrackGroup> group);
    void firePluginSuiteCallback(PluginSuite& pluginSuite, ReaperAPI const& api) override;
    void createProjectElements(PluginSuite& pluginSuite, ReaperAPI const& api) override;
private:
    TrackGroup trackGroup;
};

}
