#include "cartesianspeakerlayouts.h"
#include <algorithm>
#include <vector>
#include <map>
#include <adm/elements.hpp>
using namespace admplug;
namespace {
constexpr int32_t toInt(SupportedCartesianSpeakerLayout layout){
    return static_cast<int32_t>(layout);
}

template<typename RangeT, typename F>
auto findIfIn(RangeT range, F predicate) {
    return std::find_if(range.begin(), range.end(), predicate);
}

template<typename RangeT>
bool channelWithNameIsIn(std::string const& name, RangeT const& range) {
    using namespace adm;
    auto const channelName = AudioChannelFormatName{name};
    auto pos = findIfIn(range, [channelName](std::shared_ptr<adm::AudioChannelFormat const> const& channelFormat) {
      return channelFormat->has<AudioChannelFormatName>() && channelFormat->get<AudioChannelFormatName>() == channelName;
    });
    return pos != range.end();
}

template<typename RangeT>
bool channelsPresent(std::vector<std::string> const& names, RangeT const& range) {
    return std::all_of(names.begin(), names.end(), [range](std::string const& name) {
      return channelWithNameIsIn(name, range);});
}

struct LayoutDefinition {
    std::string packFormatId;
    std::vector<std::string> speakerNames;
    std::vector<std::int32_t> silenceTracks;
};
const std::map<SupportedCartesianSpeakerLayout, LayoutDefinition> supportedCartesianCommonMap {
    {
        // AP_00010016 is is not quite right as top speakers in wrong place,
        // Should be top L and top R not top side L and top side R
        // However, this is the closest we have in common definitions at the moment
        SupportedCartesianSpeakerLayout::L7_1_2,
        {
            "AP_00010016",
            {
                "RoomCentricLeft",
                "RoomCentricRight",
                "RoomCentricCenter",
                "RoomCentricLFE",
                "RoomCentricLeftSideSurround",
                "RoomCentricRightSideSurround",
                "RoomCentricLeftRearSurround",
                "RoomCentricRightRearSurround",
                "RoomCentricLeftTopSurround",
                "RoomCentricRightTopSurround"
            },
            {}
        }
    },
    {
         // same problem as above
        SupportedCartesianSpeakerLayout::L7_0_2,
        {
            "AP_00010016",
            {
                {
                    "RoomCentricLeft",
                    "RoomCentricRight",
                    "RoomCentricCenter",
                    "RoomCentricLeftSideSurround",
                    "RoomCentricRightSideSurround",
                    "RoomCentricLeftRearSurround",
                    "RoomCentricRightRearSurround",
                    "RoomCentricLeftTopSurround",
                    "RoomCentricRightTopSurround"
                }
            },
            {3}
        }
    },
    {
        SupportedCartesianSpeakerLayout::L7_1,
        {
            "AP_0001000f",
            {
                "RoomCentricLeft",
                "RoomCentricRight",
                "RoomCentricCenter",
                "RoomCentricLFE",
                "RoomCentricLeftSideSurround",
                "RoomCentricRightSideSurround",
                "RoomCentricLeftRearSurround",
                "RoomCentricRightRearSurround"
            },
            {}
        }
    },
    {SupportedCartesianSpeakerLayout::L7_0,
        {
            "AP0001000f",
            {"RoomCentricLeft",
                "RoomCentricRight",
                "RoomCentricCenter",
                "RoomCentricLeftSideSurround",
                "RoomCentricRightSideSurround",
                "RoomCentricLeftRearSurround",
                "RoomCentricRightRearSurround"
            },
            {3}
        }
    },
    {SupportedCartesianSpeakerLayout::L5_1,
        {"AP_00010003",
            {"RoomCentricLeft",
                "RoomCentricRight",
                "RoomCentricCenter",
                "RoomCentricLFE",
                "RoomCentricLeftSideSurround",
                "RoomCentricRightSideSurround"
            },
            {}
        }
    },
    {SupportedCartesianSpeakerLayout::L5_0,
        {
            "AP_0001000c",
            {
                "RoomCentricLeft",
                "RoomCentricRight",
                "RoomCentricCenter",
                "RoomCentricLeftSideSurround",
                "RoomCentricRightSideSurround"},
            {}
        }
    },
    {SupportedCartesianSpeakerLayout::L3_0,
        {
            "AP_0001000a",
            {
                "RoomCentricLeft",
                "RoomCentricRight",
                "RoomCentricCenter"
            },
            {}
        }
    },
    {SupportedCartesianSpeakerLayout::L2_0,
        {
            "AP_00010002",
            {
                "RoomCentricLeft",
                "RoomCentricRight"
            },
            {}
        }
    }
};

}

std::optional<SupportedCartesianSpeakerLayout>
admplug::getCartLayout(const adm::AudioPackFormat &packFormat) {
    auto channels = packFormat.getReferences<adm::AudioChannelFormat>();
    auto channelCount = channels.size();
    if(channels.size() < toInt(SupportedCartesianSpeakerLayout::MAX)) {
        auto pos = supportedCartesianCommonMap.find(
            static_cast<SupportedCartesianSpeakerLayout>(channelCount));
        if (pos != supportedCartesianCommonMap.end()) {
            auto const& layoutMapping = pos->second;
            if (channelsPresent(layoutMapping.speakerNames, channels)) {
                return pos->first;
            }
        }
    }
    return {};
}
std::string admplug::getMappedCommonPackId(SupportedCartesianSpeakerLayout layout) {
    return supportedCartesianCommonMap.find(layout)->second.packFormatId;
}
std::vector<int> admplug::silentTrackIndicesFor(SupportedCartesianSpeakerLayout layout) {
    return supportedCartesianCommonMap.find(layout)->second.silenceTracks;
}
