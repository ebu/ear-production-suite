#include "mediatakeelement.h"
#include "elementcomparator.h"
#include "reaperapi.h"
#include <chrono>
#include <adm/helper/get_optional_property.hpp>
#include "admtraits.h"
#include "pluginsuite.h"
#include "admvstcontrol.h"
#include "track.h"

using namespace admplug;
using std::chrono::nanoseconds;

namespace {
template<typename T>
double toSeconds(T property) {
    return property.get().count() / 1000000000.0;
}

template<typename Tag>
double toSeconds(adm::detail::NamedType<adm::Time, Tag> const& time) {
  return time.get().asNanoseconds().count() / 1000000000.0;
}

}



///////////// PUBLIC /////////////////////



MediaTakeElement::MediaTakeElement(std::shared_ptr<const adm::AudioObject> obj,
                                    std::shared_ptr<TrackElement> parentTrack,
                                    MediaItem* referenceItem) :
    object{obj},
    referenceItem{referenceItem}
{
    parents.push_back(parentTrack);
}

void MediaTakeElement::createProjectElements(PluginSuite &pluginSuite, const ReaperAPI &api)
{
    for(auto parent : parents) {
        auto trackEl = parent.get();
        if(trackEl && trackEl->getTrack() && !hasMediaItem(api, *trackEl)) {
            auto mediaItem = createMediaItem(api, *trackEl);
            createTake(api, mediaItem);
        }
    }
}

void MediaTakeElement::setSource(PCM_source * source)
{
    pcmSource = source;
}

void admplug::MediaTakeElement::addTrackUid(std::shared_ptr<adm::AudioTrackUid const> uid)
{
    trackUids_.push_back(uid);
}

bool admplug::MediaTakeElement::hasTrackUid(std::shared_ptr<adm::AudioTrackUid const> uid)
{
    for(auto& trackUid : trackUids_) {
        if(uid == trackUid) return true;
    }
    return false;
}

int admplug::MediaTakeElement::trackUidCount() const
{
    return trackUids_.size();
}

std::vector<std::shared_ptr<adm::AudioTrackUid const>> admplug::MediaTakeElement::trackUids() const
{
    return trackUids_;
}

double MediaTakeElement::startTime() const
{
    return position;
}

std::shared_ptr<adm::AudioObject const> admplug::MediaTakeElement::getAudioObject()
{
    return object;
}



///////////// PRIVATE /////////////////////


bool MediaTakeElement::hasMediaItem(ReaperAPI const& api, TrackElement& track) {
    return api.GetTrackNumMediaItems(track.getTrack()->get()) > 0;
}

MediaItem* MediaTakeElement::createMediaItem(ReaperAPI const& api, TrackElement& track)
{
    auto mediaItem = track.addMediaItem(api);
    setMediaItemPosition(api, mediaItem);
    setMediaItemDuration(api, mediaItem);
    return mediaItem;
}


void MediaTakeElement::setMediaItemPosition(ReaperAPI const& api, MediaItem* mediaItem) {
    auto const start = adm::getPropertyOr(object, adm::Start{adm::Time(nanoseconds::zero())});
    position = getOriginalMediaItemStartOffset(api) + toSeconds(start);
    api.SetMediaItemInfo_Value(mediaItem, "D_POSITION", position);
}

double MediaTakeElement::getOriginalMediaItemStartOffset(ReaperAPI const& api) const {
    auto start = 0.0;
    if (referenceItem) {
        start = api.GetMediaItemInfo_Value(referenceItem, "D_POSITION");
    }
    return start;
}

void MediaTakeElement::setMediaItemDuration(ReaperAPI const& api, MediaItem* mediaItem)
{
    if(object->has<adm::Duration>() && object->get<adm::Duration>() != nanoseconds::zero()) {
        setMediaItemLengthFromDurationProperty(api, mediaItem);
    } else {
        setMediaItemLengthFromSourceLength(api, mediaItem);
    }
}

void MediaTakeElement::setMediaItemLengthFromDurationProperty(ReaperAPI const& api, MediaItem* mediaItem) {
    auto const duration = object->get<adm::Duration>();
    api.SetMediaItemLength(mediaItem, toSeconds(duration), true);
}

void MediaTakeElement::setMediaItemLengthFromSourceLength(ReaperAPI const& api, MediaItem* mediaItem) {
    bool lengthIsInQuarterNotes{false};
    auto length = api.GetMediaSourceLength(pcmSource, &lengthIsInQuarterNotes);
    if(!lengthIsInQuarterNotes) {
        api.SetMediaItemLength(mediaItem, length, false);
    }
}

MediaItem_Take* MediaTakeElement::createTake(ReaperAPI const& api, MediaItem* mediaItem) {
   assert(pcmSource);
   auto mediaItemTake = api.AddTakeToMediaItem(mediaItem);
   api.SetMediaItemTake_Source(mediaItemTake, pcmSource);

   auto start = adm::getPropertyOr(object, adm::Start{nanoseconds::zero()});
   api.SetMediaItemTakeInfo_Value(mediaItemTake, "D_STARTOFFS", toSeconds(start));

   nameTakeFromElementName(api, mediaItemTake);
   return mediaItemTake;
}

void MediaTakeElement::nameTakeFromElementName(admplug::ReaperAPI const & api, MediaItem_Take* mediaItemTake) {
    adm::ElementConstVariant objECV = object;
    auto name = boost::apply_visitor(AdmNameReader(), objECV);
    // all but max profile limit length to 32
    auto nameLength = std::min<std::size_t>(32, name.size());
    name.copy(takeNameBuffer.data(), nameLength);
    takeNameBuffer[nameLength] = '\0';
    api.setTakeName(mediaItemTake, takeNameBuffer.data());
}

std::vector<adm::ElementConstVariant> admplug::MediaTakeElement::getAdmElements() const
{
    return std::vector<adm::ElementConstVariant>(1, object);
}
