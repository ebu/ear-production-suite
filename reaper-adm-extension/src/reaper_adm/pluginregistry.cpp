#include <fstream>

#include "pluginregistry.h"
#include "pluginsuite.h"
#include "reaperapi.h"
#include "filehelpers.h"
#include <helper/native_message_box.hpp>

using namespace admplug;

const std::shared_ptr<EARPluginSuite> admplug::PluginRegistry::earPluginSuite = std::make_shared<EARPluginSuite>();

void admplug::PluginRegistry::repopulateInstalledPlugins(bool warnOnFailure, const ReaperAPI &api)
{
    installedPlugins.clear();

    static const std::vector<std::string> iniFiles{std::string{"reaper-vstplugins64.ini"},
                                                std::string{"reaper-vstplugins_arm64.ini"}};
    auto vstIniPath = std::string{api.GetResourcePath()}.append(file::dirChar());
    bool fileFound{false};
    for(auto const&fileName : iniFiles) {
      std::string filePath{vstIniPath + fileName};
      std::ifstream ifs{filePath};

      // Next, parse the fileName... CSV, 3 part; filename + "=" + uid??, uid???, name (e.g,  EAR Object (EBU), EAR Binaural Monitoring (EBU) (128ch)) - we might need to do some lazy matching on that... (e.g, end bit probably isn't expected)
      if (ifs) {
        fileFound = true;
        std::string line;
        while (std::getline(ifs, line)) {
          auto startOffset = line.find('=');
          if (startOffset == std::string::npos)
            continue;
          startOffset = line.find(',', startOffset + 1);
          if (startOffset == std::string::npos)
            continue;
          startOffset = line.find(',', startOffset + 1);
          if (startOffset == std::string::npos)
            continue;
          installedPlugins.push_back(line.substr(startOffset + 1));
        }
      }
    }
    if(warnOnFailure && !fileFound){
        NativeMessageBox::splashExtensionError("Can not open plugin cache file!");
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
