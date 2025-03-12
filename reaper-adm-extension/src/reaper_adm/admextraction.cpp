#include <adm_coord_conv/adm_coord_conv.hpp>
#include "admextraction.h"

using namespace admplug::detail;

CartesianToSpherical::CartesianToSpherical(float x, float y, float z) :
    x{static_cast<double>(x)},
    y{static_cast<double>(y)},
    z{static_cast<double>(z)}
{
}

RoomCartesianToSpherical::RoomCartesianToSpherical(
    adm::AudioBlockFormatObjects block) : block{std::move(block)},
                                          extent{adm::Width{0.0},
                                                 adm::Height{0.0},
                                                 adm::Depth{0.0}}{}

bool RoomCartesianToSpherical::hasExtent() {
    return block.has<adm::Width>() ||
           block.has<adm::Height>() ||
           block.has<adm::Depth>();
}


void RoomCartesianToSpherical::calculate() {
    if(!position && block.has<adm::CartesianPosition>()) {

        auto admInputPosition = block.get<adm::CartesianPosition>();
        adm::coords::CartesianSource inputSource{ {
          admInputPosition.get<adm::X>().get(),
          admInputPosition.get<adm::Y>().get(),
          admInputPosition.get<adm::Z>().get()
        } };

        if (hasExtent()) {
          inputSource.extent = adm::coords::CartesianExtent{
            getValueOrDefault<adm::Width>().get(),
            getValueOrDefault<adm::Height>().get(),
            getValueOrDefault<adm::Depth>().get()
          };
        }

        auto convSource = adm::coords::convert(inputSource);
        position = adm::SphericalPosition(
          adm::Azimuth(convSource.position.azimuth),
          adm::Elevation(convSource.position.elevation),
          adm::Distance(convSource.position.distance)
        );

        if (convSource.extent.has_value()) {
          extent = std::make_tuple(
            adm::Width(convSource.extent->width),
            adm::Height(convSource.extent->height),
            adm::Depth(convSource.extent->depth)
          );
        }
    }
}

std::optional<RoomCartesianToSpherical> admplug::detail::getConverter(const adm::AudioBlockFormatObjects &block) {
    auto maybeX = getAdmComponent<adm::CartesianPosition, adm::X>(block);
    auto maybeY = getAdmComponent<adm::CartesianPosition, adm::Y>(block);
    auto maybeZ = getAdmComponent<adm::CartesianPosition, adm::Z>(block);
    if(maybeX && maybeY && maybeZ) {
        return RoomCartesianToSpherical{block};
    }
    return std::optional<RoomCartesianToSpherical>{};
}

std::optional<RoomCartesianToSphericalSpeaker> admplug::detail::getConverter(const adm::AudioBlockFormatDirectSpeakers& block) {
    if(block.has<adm::CartesianSpeakerPosition>()) {
        return RoomCartesianToSphericalSpeaker{block};
    }
    return {};
}

RoomCartesianToSphericalSpeaker::RoomCartesianToSphericalSpeaker(
    adm::AudioBlockFormatDirectSpeakers block) : block{std::move(block)}
{}

void RoomCartesianToSphericalSpeaker::calculate() {
    if(!position) {

        auto admInputPosition = block.get<adm::CartesianSpeakerPosition>();
        adm::coords::CartesianPosition inputPosition{
          getParamValueOrZero<adm::X>(admInputPosition),
          getParamValueOrZero<adm::Y>(admInputPosition),
          getParamValueOrZero<adm::Z>(admInputPosition)
        };

        auto convPosition = adm::coords::convert(inputPosition);
        position = adm::SphericalSpeakerPosition(
          adm::Azimuth(convPosition.azimuth),
          adm::Elevation(convPosition.elevation),
          adm::Distance(convPosition.distance)
        );
    }
}

std::vector<admplug::AutomationPoint> admplug::detail::simplify(const std::vector<admplug::AutomationPoint> &points)
{
    std::vector<AutomationPoint> filtered;
    if(!points.empty()) {
        filtered.push_back(points.front());
        bool previousCopied{true};
        for(auto currentPos = points.cbegin() + 1, end = points.cend(); currentPos != end; ++currentPos) {
            auto const& current = *currentPos;
            auto const& previous = *(currentPos - 1);
            if(current.value() != previous.value()) {
                if(!previousCopied) {
                    filtered.push_back(previous);
                }
                filtered.push_back(current);
                previousCopied = true;
            } else {
                previousCopied = false;
            }
        }
    }
    return filtered;
}

void admplug::detail::fixEffectiveTimeOverlaps(std::vector<AutomationPoint> &points)
{
    // This function assumes points are already sorted by start time
    // Effective time overlap can occur when a blocks rtime + duration exceeds the start time of the next block
    for(size_t i = 0; i < (points.size() - 1); i++) {
        if(points[i].effectiveTimeNs() > points[i + 1].timeNs()) {
            points[i].setDurationFromEffectiveTimeNs(points[i + 1].timeNs());
        }
    }
}
