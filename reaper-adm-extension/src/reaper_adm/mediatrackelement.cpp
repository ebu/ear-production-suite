#include <array>
#include "mediatrackelement.h"
#include "reaperapi.h"
#include "pluginsuite.h"
#include "color.h"
#include "admtraits.h"
#include "track.h"
#include "admtypehelpers.h"
#include "admvstcontrol.h"

namespace {
    class TrackNameOriginIdentifier : public boost::static_visitor<std::string>
    {
    public:
        // Unicode options - looks like superscript;
        // A = [-31, -76, -84]
        // a = [-31, -75, -125]
        // c = [-31, -74, -100] // Note; no uppercase character available
        // O = [-31, -76, -68]
        // o = [-31, -75, -110]
        // P = [-31, -76, -66]
        // p = [-31, -75, -106]
        // R = [-31, -76, -65]
        // r = [-54, -77]

        std::string operator()(std::shared_ptr<adm::AudioProgramme const>) const
        {
            return std::string{-31, -76, -84, -31, -76, -66, -54, -77}; // "APr"
        }

        std::string operator()(std::shared_ptr<adm::AudioContent const>) const
        {
            return std::string{-31, -76, -84, -31, -74, -100}; // "Ac"
        }

        template<typename T>
        std::string operator()(std::shared_ptr<T const> element) const {
            return "";
        }

    };

}


using namespace admplug;

MediaTrackElement::MediaTrackElement(std::vector<adm::ElementConstVariant> admElements, std::shared_ptr<TrackElement> parentTrack, std::unique_ptr<TrackGroup> masterOfGroup) :
    elements{admElements},
    groupMaster{std::move(masterOfGroup)}
{
    addParentProjectElement(parentTrack);
}

void MediaTrackElement::createProjectElements(PluginSuite &pluginSuite, ReaperAPI const& api)
{
    auto mediaTrack = api.createTrackAtIndex(0, true);
    assert(mediaTrack);
    track = std::make_shared<TrackInstance>(mediaTrack, api);
    nameTrackFromElementName();

    firePluginSuiteCallback(pluginSuite, api);
}

std::vector<adm::ElementConstVariant> MediaTrackElement::getAdmElements() const
{
    return elements;
}

MediaItem *MediaTrackElement::addMediaItem(ReaperAPI const& api)
{
    assert(track);
    return api.AddMediaItemToTrack(track->get());
}

std::vector<TrackGroup> admplug::MediaTrackElement::slaveOfGroups() const
{
    std::vector<TrackGroup> groupMasters;
    for (auto parentTrack : parentElements) {
        groupMasters.push_back(parentTrack->masterOfGroup());
    }
    return groupMasters;
}

TrackGroup admplug::MediaTrackElement::masterOfGroup() const
{
    return groupMaster->id();
}

void MediaTrackElement::nameTrackFromElementName()
{
    auto prefix = boost::apply_visitor(TrackNameOriginIdentifier(), elements[0]);
    auto name = boost::apply_visitor(AdmNameReader(), elements[0]);
    if(prefix.length() > 0) {
        name = prefix + " " + name;
    }
    std::array<char, 33> trackNameBuffer;
    auto nameLength = std::min<std::size_t>(trackNameBuffer.size() - 1, name.size());
    name.copy(trackNameBuffer.data(), nameLength);
    trackNameBuffer[nameLength] = '\0';
    track->setName(trackNameBuffer.data());
}


ObjectTrack::ObjectTrack(std::vector<adm::ElementConstVariant> admElements, std::shared_ptr<TrackElement> parentTrack) : MediaTrackElement(admElements, parentTrack, nullptr)
{
}

void ObjectTrack::firePluginSuiteCallback(PluginSuite &pluginSuite, const ReaperAPI &api) const
{
    pluginSuite.onCreateObjectTrack(*this, api);
}

DirectTrack::DirectTrack(std::vector<adm::ElementConstVariant> admElements, std::shared_ptr<TrackElement> parentTrack) : MediaTrackElement(admElements, parentTrack, nullptr)
{

}

void DirectTrack::firePluginSuiteCallback(PluginSuite &pluginSuite, const ReaperAPI &api) const
{
    pluginSuite.onCreateDirectTrack(*this, api);
}

HoaTrack::HoaTrack(std::vector<adm::ElementConstVariant> admElements, std::shared_ptr<TrackElement> parentTrack) : MediaTrackElement(admElements, parentTrack, nullptr)
{

}

void HoaTrack::firePluginSuiteCallback(PluginSuite &pluginSuite, const ReaperAPI &api) const
{
    pluginSuite.onCreateHoaTrack(*this, api);
}

GroupTrack::GroupTrack(std::vector<adm::ElementConstVariant> admElements, std::shared_ptr<TrackElement> parentTrack, std::unique_ptr<TrackGroup> group) :

    MediaTrackElement(admElements, parentTrack, std::make_unique<TrackGroup>(*group)),
    trackGroup{*group}
{

}

void GroupTrack::firePluginSuiteCallback(PluginSuite &pluginSuite, const ReaperAPI &api) const
{
    pluginSuite.onCreateGroup(*this, api);
}

void admplug::GroupTrack::createProjectElements(PluginSuite & pluginSuite, ReaperAPI const & api)
{
    if(pluginSuite.representAdmStructureWithGroups(api)) {
        MediaTrackElement::createProjectElements(pluginSuite, api);
    }
}
