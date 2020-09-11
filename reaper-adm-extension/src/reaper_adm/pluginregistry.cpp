#include <fstream>

#include "pluginregistry.h"
#include "pluginsuite.h"
#include "reaperapi.h"
#include "filehelpers.h"

#ifdef WIN32
#include "win_nonblock_msg.h"
#endif

using namespace admplug;

void admplug::PluginRegistry::repopulateInstalledPlugins(bool warnOnFailure, const ReaperAPI &api)
{
    installedPlugins.clear();

    // First open and read the vst file - reaper-vstplugins64.ini
    auto vstIniPath = std::string{api.GetResourcePath()};
    vstIniPath += (file::dirChar() + std::string{"reaper-vstplugins64.ini"});
    std::ifstream ifs(vstIniPath.c_str());

    // Next, parse the file... CSV, 3 part; filename + "=" + uid??, uid???, name (e.g,  VISR SceneMasterVST (visr) (64ch)) - we might need to do some lazy matching on that... (e.g, end bit probably isn't expected)
    if (ifs.is_open()) {
        std::string line;
        while (std::getline(ifs, line)) {
            auto startOffset = line.find("=");
            if (startOffset == std::string::npos) continue;
            startOffset = line.find(",", startOffset + 1);
            if (startOffset == std::string::npos) continue;
            startOffset = line.find(",", startOffset + 1);
            if (startOffset == std::string::npos) continue;
            installedPlugins.push_back(line.substr(startOffset + 1));
        }
        ifs.close();
    }
    else if(warnOnFailure){
        std::string msg("Can not open plugin cache file!\n\n");
        msg += vstIniPath;
        const char* title = "ADM Extension";
#ifdef WIN32
        // Windows version of Reaper locks up if you try show a message box during splash
        winhelpers::NonBlockingMessageBox(msg, title, MB_ICONEXCLAMATION);
#else
        api.ShowMessageBox(msg.c_str(), title, 0);
#endif
        repopulateInstalledPlugins_FailureWarningSent = true;
    }
}

bool admplug::PluginRegistry::checkPluginAvailable(std::string pluginName, const ReaperAPI &api, bool rescanPluginsAtStart, bool rescanPluginsOnFirstFail)
{
    std::vector<std::string> searchText{ pluginName };
    return checkPluginsAvailable(searchText, api, rescanPluginsAtStart, rescanPluginsOnFirstFail);
}

bool admplug::PluginRegistry::checkPluginsAvailable(std::vector<std::string> pluginNames, const ReaperAPI &api, bool rescanPluginsAtStart, bool rescanPluginsOnFirstFail)
{
    if (rescanPluginsAtStart) {
        repopulateInstalledPlugins(!repopulateInstalledPlugins_FailureWarningSent, api);
    }

    if (doPluginsCheck(pluginNames)) {
        return true;
    }

    if(rescanPluginsOnFirstFail) {
        repopulateInstalledPlugins(!repopulateInstalledPlugins_FailureWarningSent, api);
        if (doPluginsCheck(pluginNames)) {
            return true;
        }
    }

    return false;
}

bool admplug::PluginRegistry::doPluginsCheck(std::vector<std::string> pluginNames)
{
    for (auto& reqPlugin : pluginNames) {
        std::string compName(reqPlugin);
        compName += " (";
        bool pluginFound = false;
        for (auto& availPlugin : installedPlugins) {
            if (availPlugin.compare(0, compName.size(), compName) == 0) {
                pluginFound = true;
                break;
            }
        }
        if (!pluginFound) return false;
    }
    return true;
}

std::shared_ptr<PluginRegistry> PluginRegistry::getInstance() {
    static auto instance = std::make_shared<PluginRegistry>();
    return instance;
}

bool PluginRegistry::registerSupportedPluginSuite(std::string name, std::shared_ptr<PluginSuite> pluginSuite) {
    if (auto it = pluginSuites.find(name); it == pluginSuites.end())
    {
        pluginSuites[name] = pluginSuite;
        return true;
    }
    return false;
};
