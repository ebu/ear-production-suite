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
    return parentTake_->startTime();
}

std::shared_ptr<Track> HoaAutomationElement::getTrack() const {
    return parentTrack_->getTrack();
}

ADMChannel HoaAutomationElement::channel() const {
    //TODO: Check
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

std::vector<ADMChannel> HoaAutomationElement::takeChannels() const
{
    //Copy-pasted from DirectSpeakersAutomationElement
    return parentTake()->channels();
}

int HoaAutomationElement::channelIndex() const
{
    //Copy-pasted from DirectSpeakersAutomationElement
    auto chans = takeChannels();
    auto location = std::find(chans.cbegin(), chans.cend(), admChannel);
    if(location == chans.cend()) return -1;
    return static_cast<int>(location - chans.cbegin());
}
