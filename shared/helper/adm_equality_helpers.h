#pragma once

#include <optional>
#include <functional>
#include <memory>
#include <algorithm>
#include <adm/adm.hpp>

namespace adm {
namespace helpers {

std::optional<adm::AudioBlockFormatDirectSpeakers const> static_ds_block(adm::AudioChannelFormat const& cf) {
    using namespace adm;
    if (cf.get<TypeDescriptor>() == TypeDefinition::DIRECT_SPEAKERS) {
        auto blocks = cf.getElements<AudioBlockFormatDirectSpeakers>();
        if (blocks.size() == 1) {
            return blocks.front();
        }
    }
    return {};
}

namespace equality {
    using namespace adm;

    enum class CompareHasResult { Neither, Both, Mismatch };

    template<typename Property, typename Parent> CompareHasResult compareHas(Parent const& lhs, Parent const& rhs) {
        bool hasProp = lhs.template has<Property>();
        if (hasProp != rhs.template has<Property>())
            return CompareHasResult::Mismatch;
        if (hasProp)
            return CompareHasResult::Both;
        return CompareHasResult::Neither;
    }

    template<typename T> bool equals(T const& lhs, T const& rhs) {
        return std::equal_to<T>{}(lhs, rhs);
    }

    template<typename Property, typename Parent> bool propertyMatch(Parent const& lhs, Parent const& rhs) {
        switch (compareHas<Property>(lhs, rhs)) {
        case CompareHasResult::Mismatch:
            return false;
        case CompareHasResult::Neither:
            return true;
        case CompareHasResult::Both:
            return equals(lhs.template get<Property>(), rhs.template get<Property>());
        default:
            return false; // just to keep compiler warnings away
        }
    }

    template<typename... Properties, typename Parent> bool propertiesMatch(Parent const& lhs, Parent const& rhs) {
        return (propertyMatch<Properties, Parent>(lhs, rhs) && ...);
    }

    template<> bool equals(Gain const& lhs, Gain const& rhs) {
        return lhs.get() == rhs.get();
    }

    template<> bool equals(SpeakerLabels const& lhs, SpeakerLabels const& rhs) {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    template<> bool equals(HeadphoneVirtualise const& lhs, HeadphoneVirtualise const& rhs) {
        return propertiesMatch<Bypass, DirectToReverberantRatio>(lhs, rhs);
    }

    template<> bool equals(ScreenEdgeLock const& lhs, ScreenEdgeLock const& rhs) {
        return propertiesMatch<HorizontalEdge, VerticalEdge>(lhs, rhs);
    }

    template<> bool equals(SphericalSpeakerPosition const& lhs, SphericalSpeakerPosition const& rhs) {
        return propertiesMatch<Azimuth, AzimuthMin, AzimuthMax, Elevation, ElevationMin, ElevationMax, Distance, DistanceMin, DistanceMax, ScreenEdgeLock>(lhs, rhs);
    }

    template<> bool equals(CartesianSpeakerPosition const& lhs, CartesianSpeakerPosition const& rhs) {
        return propertiesMatch<X, XMin, XMax, Y, YMin, YMax, Z, ZMin, ZMax, ScreenEdgeLock>(lhs, rhs);
    }

    bool isEquivalentByProperties(std::shared_ptr<const adm::AudioChannelFormat> cfA, std::shared_ptr<const adm::AudioChannelFormat> cfB) {
        if (compareHas<TypeDescriptor>(*cfA, *cfB) != CompareHasResult::Both) { 
            return false; 
        } 
        // TODO: only support matching up DS AudioChannelFormats for now 
        auto bfA = adm::helpers::static_ds_block(*cfA); 
        auto bfB = adm::helpers::static_ds_block(*cfB);
        if(bfA && bfB) { 
            return propertiesMatch<SpeakerLabels, Importance, HeadLocked, Gain, HeadphoneVirtualise, SphericalSpeakerPosition, CartesianSpeakerPosition>(*bfA, *bfB); 
        } 
        return false;
    }
}

}
}