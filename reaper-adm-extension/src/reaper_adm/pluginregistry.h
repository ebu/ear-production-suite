#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "pluginsuite_ear.h"

namespace admplug {
    class PluginSuite;
    class ReaperAPI;

    class PluginRegistry
    {

    public:
        PluginRegistry() : repopulateInstalledPlugins_FailureWarningSent{ false } {}
        ~PluginRegistry() {}

        PluginRegistry(PluginRegistry const&) = delete;
        void operator=(PluginRegistry const&) = delete;

        void repopulateInstalledPlugins(bool warnOnFailure, const ReaperAPI &api);
        bool checkPluginAvailable(std::string pluginName, const ReaperAPI &api, bool rescanPluginsAtStart = false, bool rescanPluginsOnFirstFail = true);
        bool checkPluginsAvailable(std::vector<std::string> pluginNames, const ReaperAPI & api, bool rescanPluginsAtStart = false, bool rescanPluginsOnFirstFail = true);

        static std::shared_ptr<PluginRegistry> getInstance();
        static const std::shared_ptr<EARPluginSuite> earPluginSuite;

    private:
        std::vector<std::string> installedPlugins;
        bool repopulateInstalledPlugins_FailureWarningSent;

        bool doPluginsCheck(std::vector<std::string> pluginNames);
    };

}

