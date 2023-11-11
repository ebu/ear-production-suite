#pragma once
#include <optional>
#include <string>
#include <vector>
#include <adm/elements_fwd.hpp>
namespace admplug {
enum class SupportedCartesianSpeakerLayout : int32_t {
    L2_0 = 2,
    L3_0 = 3,
    L5_0 = 5,
    L5_1 = 6,
    L7_0 = 7,
    L7_1 = 8,
    L7_0_2 = 9,
    L7_1_2 = 10,
    MAX
};

std::optional<SupportedCartesianSpeakerLayout> getCartLayout(const adm::AudioPackFormat &packFormat);
std::string getMappedCommonPackId(SupportedCartesianSpeakerLayout layout);
std::vector<int> silentTrackIndicesFor(SupportedCartesianSpeakerLayout layout);
}
