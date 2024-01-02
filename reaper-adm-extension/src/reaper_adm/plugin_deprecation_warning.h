#pragma once

#include "reaperapi.h"
#include "pluginsuite.h"
#include <string>
#include <map>

const std::string pluginDeprecationMessage{
    "Support for FB360 and VISR plugin suites will be dropped in future versions of the EAR Production Suite." };

inline void pluginDeprecationWarning(const std::pair<std::string, std::shared_ptr<PluginSuite>>& pluginSuite, ReaperAPI& api) {
    if (pluginSuite.first != std::string("EAR")) { // a bit hacky, but this won't be in long-term so meh
        std::string msgboxText{ "Please note:\n" };
        msgboxText += pluginDeprecationMessage;
        api.ShowMessageBox(msgboxText.c_str(), "ADM Open", 0);
    }
}