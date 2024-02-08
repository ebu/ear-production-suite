#pragma once

#include <vector>

namespace ear {
namespace plugin {

enum Layer { upper, middle, bottom };

inline uint16_t getPackFormatIdValueFromLegacySpeakerSetupsIndex(int index) {
    /* EPS prior to version 0.7.0 used a speaker_setup_index property rather 
       rather than a packformat ID to refer to a specific DS PF */
    const std::vector<int> oldIdOrdering{
        1,  // 0 = AP_00010001
        2,  // 1 = AP_00010002
        3,  // 2 = AP_00010003
        4,  // 3 = AP_00010004
        5,  // 4 = AP_00010005
        16, // 5 = AP_00010010
        7,  // 6 = AP_00010007
        8,  // 7 = AP_00010008
        9,  // 8 = AP_00010009
        15, // 9 = AP_0001000f
        23, // 10 = AP_00010017
        22, // 11 = AP_00010016
        10, // 12 = AP_0001000a
        12  // 13 = AP_0001000c
    };
    if (index >= 0 && index < oldIdOrdering.size()) {
        return oldIdOrdering[index];
    }
    return 0; // Invalid ID used throughout for "unset"
}

}  // namespace plugin
}  // namespace ear
