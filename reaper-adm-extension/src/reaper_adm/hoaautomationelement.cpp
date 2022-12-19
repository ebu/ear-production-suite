#include "hoaautomationelement.h"
#include "pluginsuite.h"
#include "reaperapi.h"
#include "track.h"
#include "plugin.h"
#include "admextraction.h"

using namespace admplug;

namespace {
std::optional<AutomationPoint> getPoint(Parameter const& param, adm::AudioBlockFormatHoa block) {
    using namespace admplug::detail;
    return std::optional<AutomationPoint>();
}
}

HoaAutomationElement::HoaAutomationElement(ADMChannel channel, std::shared_ptr<TrackElement> track, std::shared_ptr<TakeElement> take) : admChannel{std::move(channel)}

{
    parentTake_ = take;
    parentTrack_ = track;
}

void HoaAutomationElement::createProjectElements(PluginSuite &pluginSuite, const ReaperAPI &api){
    pluginSuite.onHoaAutomation(*this, api);
}

double HoaAutomationElement::startTime() const {
    auto lockParentTake = parentTake_.lock();
    assert(lockParentTake);
    if(lockParentTake) {
        return lockParentTake->startTime();
    }
    return 0.0;
}

std::shared_ptr<Track> HoaAutomationElement::getTrack() const {
    auto lockParentTrack = parentTrack_.lock();
    assert(lockParentTrack);
    if(lockParentTrack) {
        return lockParentTrack->getTrack();
    }
    return nullptr;
}

ADMChannel HoaAutomationElement::channel() const {
    return admChannel;
}

adm::BlockFormatsConstRange<adm::AudioBlockFormatHoa> HoaAutomationElement::blocks() const
{
    if(!admChannel.channelFormat()) return adm::BlockFormatsConstRange<adm::AudioBlockFormatHoa>();
    return admChannel.channelFormat()->getElements<adm::AudioBlockFormatHoa>();
}

int getBlockOrder(adm::AudioBlockFormatHoa block) {
    return block.get<adm::Order>().get();
}

void HoaAutomationElement::apply(const PluginParameter &parameter, const Plugin &plugin) const {
    detail::applyAutomation(pointsFor(parameter), startTime(), parameter, plugin);
}

void HoaAutomationElement::apply(const TrackParameter &parameter, const Track &track) const {
    detail::applyAutomation(pointsFor(parameter), startTime(), parameter, track);
}

std::vector<AutomationPoint> HoaAutomationElement::pointsFor(Parameter const& parameter) const {
    std::vector<AutomationPoint> parameterPoints;
    if(admChannel.channelFormat()) {
        for(auto& block : admChannel.channelFormat()->getElements<adm::AudioBlockFormatHoa>()) {
            auto point = ::getPoint(parameter, block);
            if(point) {
                parameterPoints.push_back(*point);
            }
        }
    }
    return parameterPoints;
}

std::vector<adm::ElementConstVariant> HoaAutomationElement::getAdmElements() const
{
    //Copy-pasted from DirectSpeakersAutomationElement
    if(!admChannel.channelFormat()) return std::vector<adm::ElementConstVariant>();
    return std::vector<adm::ElementConstVariant>(1, admChannel.channelFormat());
}

int HoaAutomationElement::channelIndex() const
{
    //Copy-pasted from DirectSpeakersAutomationElement
    auto lockParentTake = parentTake_.lock();
    assert(lockParentTake);
    if(lockParentTake) {
        auto chans = lockParentTake->channelsOfOriginal();
        auto location = std::find(chans.cbegin(), chans.cend(), admChannel.channelOfOriginal());
        if(location == chans.cend()) return -1;
        return static_cast<int>(location - chans.cbegin());
    }
    return -1;
}