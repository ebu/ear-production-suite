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
                                    std::weak_ptr<TrackElement> parentTrack,
                                    MediaItem* referenceItem) :
    object{obj},
    referenceItem{referenceItem}
{
    addParentProjectElement(parentTrack);
}

void MediaTakeElement::createProjectElements(PluginSuite &pluginSuite, const ReaperAPI &api)
{
    for(auto parentWeak : parents) {
        if(auto parent = parentWeak.lock()) {
            auto trackEl = parent.get();
            if(trackEl && trackEl->getTrack() && !hasMediaItem(api, *trackEl)) {
                auto mediaItem = createMediaItem(api, *trackEl);
                createTake(api, mediaItem);
            }
        }
    }
}

void MediaTakeElement::setSource(PCM_source * source)
{
    pcmSource = source;
}

void admplug::MediaTakeElement::addChannelOfOriginal(int channelNum)
{
    channelsOfOriginal_.push_back(channelNum);
}

bool admplug::MediaTakeElement::hasChannelOfOriginal(int channelNum)
{
    for(auto& channel : channelsOfOriginal_) {
        if(channel == channelNum) return true;
    }
    return false;
}

int admplug::MediaTakeElement::channelCount() const
{
    return channelsOfOriginal_.size();
}

std::vector<int> admplug::MediaTakeElement::channelsOfOriginal() const
{
    return channelsOfOriginal_;
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

   nameTakeFromOriginalChannels(api, mediaItemTake);
   return mediaItemTake;
}

void MediaTakeElement::nameTakeFromOriginalChannels(admplug::ReaperAPI const & api, MediaItem_Take* mediaItemTake) {
    std::vector<std::pair<int, int>> nameParts;
    int curChIndex = 0;
    while(curChIndex < channelsOfOriginal_.size()) {
        int startChIndex = curChIndex;
        while((curChIndex + 1) < channelsOfOriginal_.size()) {
            if(channelsOfOriginal_[curChIndex + 1] != channelsOfOriginal_[startChIndex] + (curChIndex - startChIndex) + 1) {
                break;
            }
            curChIndex++;
        }

        nameParts.push_back(std::make_pair(channelsOfOriginal_[startChIndex], channelsOfOriginal_[curChIndex]));
        curChIndex++;
    }

    std::string n("Import Ch: ");
    for(int i = 0; i < nameParts.size(); ++i) {
        n += std::to_string(nameParts[i].first);
        if(nameParts[i].first != nameParts[i].second) {
            n += "-";
            n += std::to_string(nameParts[i].second);
        }
        if(i < nameParts.size() - 1) {
            n += ", ";
        }
    }

    api.setTakeName(mediaItemTake, n);
}

std::vector<adm::ElementConstVariant> admplug::MediaTakeElement::getAdmElements() const
{
    return std::vector<adm::ElementConstVariant>(1, object);
}
