#include <cassert>
#include "directspeakerautomationelement.h"
#include "pluginsuite.h"
#include "reaperapi.h"
#include "track.h"
#include "plugin.h"
#include "admextraction.h"

using namespace admplug;
namespace {
std::optional<AutomationPoint> getPoint(Parameter const& param, adm::AudioBlockFormatDirectSpeakers block) {
    using namespace admplug::detail;
    switch(param.admParameter()) {
    case AdmParameter::SPEAKER_AZIMUTH : {
        return getSphericalPoint<adm::Azimuth>(param, block);
    }
    case AdmParameter::SPEAKER_ELEVATION : {
        return getSphericalPoint<adm::Elevation>(param, block);
    }
    case AdmParameter::SPEAKER_DISTANCE : {
        return getSphericalPoint<adm::Distance>(param, block);
    }
    case AdmParameter::OBJECT_AZIMUTH: {}
        [[fallthrough]];
    case AdmParameter::OBJECT_ELEVATION: {}
        [[fallthrough]];
    case AdmParameter::OBJECT_DISTANCE: {}
        [[fallthrough]];
    case AdmParameter::OBJECT_GAIN: {}
        [[fallthrough]];
    case AdmParameter::NONE: {}
    }
    return std::optional<AutomationPoint>{};
}
}

DirectSpeakersAutomationElement::DirectSpeakersAutomationElement(ADMChannel channel, std::shared_ptr<TrackElement> track, std::shared_ptr<TakeElement> take) :
    admChannel{ std::move(channel) }
{
    parentTake_ = take;
    parentTrack_ = track;
}

void DirectSpeakersAutomationElement::createProjectElements(PluginSuite &pluginSuite, const ReaperAPI &api)
{
    pluginSuite.onDirectSpeakersAutomation(*this, api);
}

adm::BlockFormatsConstRange<adm::AudioBlockFormatDirectSpeakers> DirectSpeakersAutomationElement::blocks() const
{
    if(!admChannel.channelFormat()) return adm::BlockFormatsConstRange<adm::AudioBlockFormatDirectSpeakers>();
    return admChannel.channelFormat()->getElements<adm::AudioBlockFormatDirectSpeakers>();
}

double DirectSpeakersAutomationElement::startTime() const
{
    return parentTake_->startTime();
}

std::shared_ptr<Track> DirectSpeakersAutomationElement::getTrack() const
{
    return parentTrack_->getTrack();
}

int DirectSpeakersAutomationElement::channelIndex() const
{
    auto chans = parentTake_->trackUids();
    auto location = std::find(chans.cbegin(), chans.cend(), admChannel.trackUid());
    if(location == chans.cend()) return -1;
    return static_cast<int>(location - chans.cbegin());
}

void DirectSpeakersAutomationElement::apply(const PluginParameter &parameter, const Plugin &plugin) const
{
    detail::applyAutomation(pointsFor(parameter), startTime(), parameter, plugin);
}

void DirectSpeakersAutomationElement::apply(const TrackParameter &parameter, const Track &track) const
{
    detail::applyAutomation(pointsFor(parameter), startTime(), parameter, track);
}

std::vector<adm::ElementConstVariant> DirectSpeakersAutomationElement::getAdmElements() const
{
    if(!admChannel.channelFormat()) return std::vector<adm::ElementConstVariant>();
    return std::vector<adm::ElementConstVariant>(1, admChannel.channelFormat());
}

std::vector<AutomationPoint> DirectSpeakersAutomationElement::pointsFor(Parameter const& parameter) const {
    std::vector<AutomationPoint> parameterPoints;
    if(admChannel.channelFormat()) {
        for(auto& block : admChannel.channelFormat()->getElements<adm::AudioBlockFormatDirectSpeakers>()) {
            auto point = ::getPoint(parameter, block);
            if(point) {
                parameterPoints.push_back(*point);
            }
        }
    }
    return parameterPoints;
}

ADMChannel admplug::DirectSpeakersAutomationElement::channel() const
{
    return admChannel;
}
