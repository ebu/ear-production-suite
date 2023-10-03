#pragma once

#include <memory>
#include "reaperhost.h"
#include "reaperapi.h"

class GlobalConfig {
public:
    static GlobalConfig& createInstance(ReaperAPI* api);
    static GlobalConfig& getInstance();

    uint8_t getMaxDawChannels();

private:
    GlobalConfig(ReaperAPI* api);

    uint8_t maxDawChannels{ 64 }; // 64 is safe for REAPER v6
};