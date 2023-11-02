#pragma once

#include "reaper_vst3_interfaces.h"
#include <daw_channel_count.h>

inline int DetermineChannelCount(IReaperHostApplication* reaperHostPtr) {
  if (!reaperHostPtr) {
    return MAX_DAW_CHANNELS;
  }
  auto getAppVersionPtr = reaperHostPtr->getReaperApi("GetAppVersion");
  if (!getAppVersionPtr) {
    return MAX_DAW_CHANNELS;
  }
  return GetReaperChannelCount(getAppVersionPtr);
}
