#include "global_config.h"

GlobalConfig& GlobalConfig::createInstance(ReaperAPI* api)
{
    // Instance must be created before get
    static GlobalConfig singleton{ api };
    return singleton;
}

GlobalConfig& GlobalConfig::getInstance()
{
    return createInstance(nullptr); // will get
}

uint8_t GlobalConfig::getMaxDawChannels()
{
    return maxDawChannels;
}

GlobalConfig::GlobalConfig(ReaperAPI* api)
{
    auto ver = api->GetAppVersion();
    auto verFlt = std::stof(ver);
    maxDawChannels = verFlt >= 7.0f ? 128 : 64;
}
