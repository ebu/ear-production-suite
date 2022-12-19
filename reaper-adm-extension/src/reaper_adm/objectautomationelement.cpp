#include <optional>
#include <tuple>
#include "objectautomationelement.h"
#include "automationenvelope.h"
#include "pluginsuite.h"
#include "reaperapi.h"
#include "parameter.h"
#include "plugin.h"

#include "admextraction.h"
//#include "automation.h"

using namespace admplug;

namespace {
using namespace admplug::detail;
std::optional<AutomationPoint> getPoint(Parameter const& param, adm::AudioBlockFormatObjects block) {
    switch(param.admParameter()) {
    case AdmParameter::OBJECT_AZIMUTH: {
        return getSphericalPoint<adm::Azimuth>(param, block);
    }
    case AdmParameter::OBJECT_ELEVATION: {
        return getSphericalPoint<adm::Elevation>(param, block);
    }
    case AdmParameter::OBJECT_DISTANCE: {
        return getSphericalPoint<adm::Distance>(param, block);
    }
    case AdmParameter::OBJECT_GAIN: {
        return detail::getPoint<adm::Gain>(param, block);
    }
    case AdmParameter::OBJECT_WIDTH: {
        return detail::getPoint<adm::Width>(param, block);
    }
    case AdmParameter::OBJECT_HEIGHT: {
        return detail::getPoint<adm::Height>(param, block);
    }
    case AdmParameter::OBJECT_DEPTH: {
        return detail::getPoint<adm::Depth>(param, block);
    }
    case AdmParameter::OBJECT_DIFFUSE: {
        return detail::getPoint<adm::Diffuse>(param, block);
    }
    case AdmParameter::OBJECT_DIVERGENCE: {
        return detail::getPoint<adm::ObjectDivergence,adm::Divergence>(param, block);
    }
    case AdmParameter::OBJECT_DIVERGENCE_AZIMUTH_RANGE: {
        return detail::getPoint<adm::ObjectDivergence,adm::AzimuthRange>(param, block);
    }
    case AdmParameter::SPEAKER_AZIMUTH: {}
        [[fallthrough]];
    case AdmParameter::SPEAKER_ELEVATION: {}
        [[fallthrough]];
    case AdmParameter::SPEAKER_DISTANCE: {}
        [[fallthrough]];
    case AdmParameter::NONE: {}
    }
    return std::optional<AutomationPoint>();
}
}

ObjectAutomationElement::ObjectAutomationElement(ADMChannel admChannel,
                                                std::shared_ptr<TrackElement> track,
                                                std::shared_ptr<TakeElement> take) :
    admChannel{std::move(admChannel)}
{
    parentTake_ = take;
    parentTrack_ = track;
}

void ObjectAutomationElement::createProjectElements(PluginSuite &pluginSuite, const ReaperAPI &api)
{
    pluginSuite.onObjectAutomation(*this, api);
}

adm::BlockFormatsConstRange<adm::AudioBlockFormatObjects> ObjectAutomationElement::blocks() const
{
    if(!admChannel.channelFormat()) return adm::BlockFormatsConstRange<adm::AudioBlockFormatObjects>();
    return admChannel.channelFormat()->getElements<adm::AudioBlockFormatObjects>();
}

double ObjectAutomationElement::startTime() const
{
    auto lockParentTake = parentTake_.lock();
    assert(lockParentTake);
    if(lockParentTake) {
        return lockParentTake->startTime();
    }
    return 0.0;
}

std::shared_ptr<Track> ObjectAutomationElement::getTrack() const
{
    auto lockParentTrack = parentTrack_.lock();
    assert(lockParentTrack);
    if(lockParentTrack) {
        return lockParentTrack->getTrack();
    }
    return nullptr;
}

ADMChannel ObjectAutomationElement::channel() const
{
    return admChannel;
}

std::vector<adm::ElementConstVariant> admplug::ObjectAutomationElement::getAdmElements() const
{
    if(!admChannel.channelFormat()) return std::vector<adm::ElementConstVariant>();
    return std::vector<adm::ElementConstVariant>(1, admChannel.channelFormat());
}

namespace {

    bool isJumpPositionEnabled(adm::AudioBlockFormatObjects const &block){
        auto jp = block.get<adm::JumpPosition>();
        return adm::isEnabled(jp);
    }

    std::chrono::nanoseconds getInterpolationLength(adm::AudioBlockFormatObjects const &block){
        if(block.has<adm::JumpPosition>() && isJumpPositionEnabled(block)){
            auto jumpPosition = block.get<adm::JumpPosition>();
            return jumpPosition.has<adm::InterpolationLength>() ?
                jumpPosition.get<adm::InterpolationLength>().get() :
                std::chrono::nanoseconds::zero();
        }
        return getValueOrZero<adm::Duration>(block);
    }

    using namespace std::chrono_literals;

    adm::AudioBlockFormatObjects getJumpPositionEndBlock(adm::AudioBlockFormatObjects block,
                                                         std::chrono::nanoseconds interpolationLength) {
        auto rTime = block.get<adm::Rtime>().get();
        if(rTime != 0ns) {
            block.set(adm::Rtime(rTime.asNanoseconds() + interpolationLength));
            auto duration = block.get<adm::Duration>().get();
            block.set(adm::Duration{duration.asNanoseconds() - interpolationLength});
        }
        return block;
    }

    std::vector<AutomationPoint> getJumpPositionPoints(adm::AudioBlockFormatObjects const block,
                                                       Parameter const& parameter,
                                                       std::chrono::nanoseconds interpolationLength) {
        assert(interpolationLength >= 0ns);
        std::vector<AutomationPoint> jumpPositionPoints;

        auto firstBlock = block;
        firstBlock.set(adm::Duration(interpolationLength));
        if(auto firstPoint = getPoint(parameter, firstBlock)) {
            jumpPositionPoints.push_back(*firstPoint);
        }

        auto secondBlock = getJumpPositionEndBlock(block, interpolationLength);
        if(auto secondPoint = getPoint(parameter, secondBlock)) {
            jumpPositionPoints.push_back(*secondPoint);
        }

        return jumpPositionPoints;
    }
}

std::vector<AutomationPoint> ObjectAutomationElement::pointsFor(Parameter const& parameter) const {
    std::vector<AutomationPoint> parameterPoints;
    if(admChannel.channelFormat()) {
        for(auto& block : admChannel.channelFormat()->getElements<adm::AudioBlockFormatObjects>()) {
            auto interpolationLength = getInterpolationLength(block);
            auto duration = getValueOrZero<adm::Duration>(block);
            if(interpolationLength != duration) {
                auto jumpPositionPoints = getJumpPositionPoints(block, parameter, interpolationLength);
                parameterPoints.insert(parameterPoints.end(), jumpPositionPoints.begin(), jumpPositionPoints.end());
            } else {
                auto point = getPoint(parameter, block);
                if(point) {
                    parameterPoints.push_back(*point);
                }
            }
        }
    }
    return parameterPoints;
}

template <typename AutomatableT>
void setPoint(AutomationPoint const& point, Parameter& parameter, AutomatableT& automatable) {
    automatable.set(parameter, point);
}

void admplug::ObjectAutomationElement::apply(const PluginParameter& parameter, const Plugin& plugin) const {
    detail::applyAutomation(pointsFor(parameter), startTime(), parameter, plugin);
}

void admplug::ObjectAutomationElement::apply(const TrackParameter& parameter, const Track& track) const {
    detail::applyAutomation(pointsFor(parameter), startTime(), parameter, track);
}
