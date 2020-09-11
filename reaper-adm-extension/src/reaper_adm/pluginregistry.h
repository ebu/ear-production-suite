#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

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

        std::map<std::string, std::shared_ptr<PluginSuite>>* getPluginSuites() { return &pluginSuites; }

        void repopulateInstalledPlugins(bool warnOnFailure, const ReaperAPI &api);
        bool checkPluginAvailable(std::string pluginName, const ReaperAPI &api, bool rescanPluginsAtStart = false, bool rescanPluginsOnFirstFail = true);
        bool checkPluginsAvailable(std::vector<std::string> pluginNames, const ReaperAPI & api, bool rescanPluginsAtStart = false, bool rescanPluginsOnFirstFail = true);

        static std::shared_ptr<PluginRegistry> getInstance();
        bool registerSupportedPluginSuite(std::string name, std::shared_ptr<PluginSuite> pluginSuite);

    private:
        std::map<std::string, std::shared_ptr<PluginSuite>> pluginSuites;
        std::vector<std::string> installedPlugins;
        bool repopulateInstalledPlugins_FailureWarningSent;

        bool doPluginsCheck(std::vector<std::string> pluginNames);
    };

}

