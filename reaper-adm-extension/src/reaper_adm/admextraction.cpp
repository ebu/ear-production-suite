#include "coordinate_conversion/coord_conv.hpp"
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
        auto inputPosition = block.get<adm::CartesianPosition>();
        if(hasExtent()) {
            auto inputExtent = std::make_tuple(getValueOrDefault<adm::Width>(),
                                               getValueOrDefault<adm::Height>(),
                                               getValueOrDefault<adm::Depth>());
            std::tie(position, extent) = adm::cartToPolar(std::make_tuple(inputPosition, inputExtent));
        } else {
            position = adm::pointCartToPolar(inputPosition);
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
        auto inputPosition = block.get<adm::CartesianSpeakerPosition>();
        position = adm::pointCartToPolar(inputPosition);
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
