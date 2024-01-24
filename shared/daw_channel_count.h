#pragma once
#include <cassert>

constexpr int const MAX_DAW_CHANNELS{ 128 };

inline int GetReaperChannelCount(float reaperAppVersionFlt) {
    return reaperAppVersionFlt >= 7.0f ? 128 : 64;
}

inline int GetReaperChannelCount(const char* reaperAppVersion) {
    if (!reaperAppVersion) {
        return MAX_DAW_CHANNELS;
    }
    try {
        auto reaperAppVersionFlt = std::stof(reaperAppVersion);
        return GetReaperChannelCount(reaperAppVersionFlt);
    }
    catch (const std::logic_error& e) {
        // catches nullptr and invalid arg to stof
        assert(false); // just to warn when in debug
    }
    return MAX_DAW_CHANNELS; // Couldn't parse version - let's assume 128 since older versions we tested on all seem parsable
}

inline int GetReaperChannelCount(void* reaperApiGetAppVersionFuncPtr) {
    if (!reaperApiGetAppVersionFuncPtr) {
        return MAX_DAW_CHANNELS;
    }
    using AppVersionT = char const* (*)();
    auto getAppVersion = reinterpret_cast<AppVersionT>(reaperApiGetAppVersionFuncPtr);
    return GetReaperChannelCount(getAppVersion());
}