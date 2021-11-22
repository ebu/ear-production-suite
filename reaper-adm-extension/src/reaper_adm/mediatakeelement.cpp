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

MediaTakeElement::MediaTakeElement(std::shared_ptr<const adm::AudioObject> obj,
                                    std::shared_ptr<TrackElement> parentTrack,
                                    MediaItem* referenceItem) :
    object{obj},
    referenceItem{referenceItem}
{
    parent = parentTrack;
}

void MediaTakeElement::createProjectElements(PluginSuite &pluginSuite, const ReaperAPI &api)
{
   createMediaItem(api);
   createTake(api);
}

void MediaTakeElement::createMediaItem(ReaperAPI const& api)
{
    mediaItem = parent->addMediaItem(api);
    setMediaItemPosition(api);
    setMediaItemDuration(api);
}

void MediaTakeElement::nameTakeFromElementName(admplug::ReaperAPI const & api) {
    adm::ElementConstVariant objECV = object;
    auto name = boost::apply_visitor(AdmNameReader(), objECV);
    // all but max profile limit length to 32
    auto nameLength = std::min<std::size_t>(32, name.size());
    name.copy(takeNameBuffer.data(), nameLength);
    takeNameBuffer[nameLength] = '\0';
    api.setTakeName(mediaItemTake, takeNameBuffer.data());
}

void MediaTakeElement::setMediaItemPosition(ReaperAPI const& api) {
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

void MediaTakeElement::setMediaItemDuration(ReaperAPI const& api)
{
    if(object->has<adm::Duration>() && object->get<adm::Duration>() != nanoseconds::zero()) {
        setMediaItemLengthFromDurationProperty(api);
    } else {
        setMediaItemLengthFromSourceLength(api);
    }
}

void MediaTakeElement::setMediaItemLengthFromDurationProperty(ReaperAPI const& api) {
    auto const duration = object->get<adm::Duration>();
    api.SetMediaItemLength(mediaItem, toSeconds(duration), true);
}

void MediaTakeElement::setMediaItemLengthFromSourceLength(ReaperAPI const& api) {
    bool lengthIsInQuarterNotes{false};
    auto length = api.GetMediaSourceLength(pcmSource, &lengthIsInQuarterNotes);
    if(!lengthIsInQuarterNotes) {
        api.SetMediaItemLength(mediaItem, length, false);
    }
}

void MediaTakeElement::createTake(ReaperAPI const& api) {
   assert(pcmSource);
   mediaItemTake = api.AddTakeToMediaItem(mediaItem);
   api.SetMediaItemTake_Source(mediaItemTake, pcmSource);

   auto start = adm::getPropertyOr(object, adm::Start{nanoseconds::zero()});
   api.SetMediaItemTakeInfo_Value(mediaItemTake, "D_STARTOFFS", toSeconds(start));

   nameTakeFromElementName(api);
}

std::vector<adm::ElementConstVariant> admplug::MediaTakeElement::getAdmElements() const
{
    return std::vector<adm::ElementConstVariant>(1, object);
}

double MediaTakeElement::startTime() const
{
    return position;
}

void MediaTakeElement::setSource(PCM_source * source)
{
    pcmSource = source;
}

void MediaTakeElement::addChannel(ADMChannel channel)
{
    admChannels.push_back(channel);
}

bool admplug::MediaTakeElement::hasChannel(ADMChannel channel)
{
    for(auto& existingChannel : admChannels) {
        if(existingChannel == channel) return true;
    }
    return false;
}

std::vector<ADMChannel> MediaTakeElement::channels() const {
    return admChannels;
}

void admplug::MediaTakeElement::setChannels(std::vector<ADMChannel> channels)
{
    admChannels = channels;
}

