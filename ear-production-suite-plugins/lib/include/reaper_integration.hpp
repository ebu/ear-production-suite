#pragma once

#include <functional>
#include <string>

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

// When a plugin is initialised, it should register itself with the extension using this function
// A callback function should be provided as an argument, 
//  which the extension will use to return XML data to set the plugin state
inline bool registerPluginLoadWithExtension(
    IReaperHostApplication* reaperHostPtr,
    std::function<void(std::string const& xmlStateData)> callback) {
  if (!reaperHostPtr) return false;
  void registerPluginLoadSig(std::function<void(std::string const&)>);
  auto registerPluginLoadPtr = reaperHostPtr->getReaperApi("registerPluginLoad");
  if (!registerPluginLoadPtr) return false;
  auto registerPluginLoad = reinterpret_cast<decltype(&registerPluginLoadSig)>(registerPluginLoadPtr);
  registerPluginLoad(callback);
  return true;
}

// Queries the extension to get a unique ID for a plugin instance
inline uint32_t requestInstanceIdFromExtension(
    IReaperHostApplication* reaperHostPtr) {
  if(!reaperHostPtr) return 0;
  auto requestInputInstanceIdPtr = reaperHostPtr->getReaperApi("requestInputInstanceId");
  if (!requestInputInstanceIdPtr) return 0;
  using RequestInputInstanceIdSigT = uint32_t(*)();
  auto requestInputInstanceId = reinterpret_cast<RequestInputInstanceIdSigT>(requestInputInstanceIdPtr);
  return requestInputInstanceId();
}
