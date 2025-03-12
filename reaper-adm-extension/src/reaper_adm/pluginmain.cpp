#define WIN32_LEAN_AND_MEAN
#include <memory>
#include <map>
#include <string>
#include "reaperhost.h"
#include "reaperapi.h"
#include "actionmanager.h"
#include "menu.h"
#include "admmetadata.h"
#include "importaction.h"
#include "exportaction.h"
#include "pluginsuite.h"
#include "pluginregistry.h"
#include <update_check.h>
#include <version/eps_version.h>
#include <helper/native_message_box.hpp>
#include <helper/resource_paths.hpp>
#include <helper/resource_paths_juce-file.hpp>

namespace {
#ifdef WIN32
const std::map<const std::string, const int> defaultMenuPositions = {
    {"&File", 0},
    {"&Insert", 3},
    {"&Help", 9},
    {"E&xtensions", 8},
    {"Project &templates", 8},
    {"Empty item", 2},
    {"Group", 4}};
}
#elif defined __APPLE__
const std::map<const std::string, const int> defaultMenuPositions = {
    {"&File", 1},
    {"&Insert", 4},
    {"&Help", 10},
    {"E&xtensions", 9},
    {"Project &templates", 8},
    {"Empty item", 2},
    {"Group", 4}};
}
#else
const std::map<const std::string, const int> defaultMenuPositions = {
    {"&File", 0},
    {"&Insert", 3},
    {"&Help", 9},
    {"E&xtensions", 8},
    {"Project &templates", 8},
    {"Empty item", 2},
    {"Group", 4}};
}
#endif

extern "C" {

  uint32_t requestInputInstanceId() {
    auto iip = EARPluginInstanceIdProvider::getInstance();
    return iip->provideId();
  }

  void registerPluginLoad(std::function<void(std::string const&)> callback) {
      auto cbh = EARPluginCallbackHandler::getInstance();
      cbh->setPluginCallback(callback);
  }

  int REAPER_PLUGIN_DLL_EXPORT REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec)
  {
    using namespace admplug;
    std::unique_ptr<ReaperHost> reaper;

    try {
        reaper = std::make_unique<ReaperHost>(hInstance, rec);
    } catch (FuncResolutionException const& e) {
        NativeMessageBox::splashExtensionError(e.what(), rec->hwnd_main);
        reaper = std::make_unique<ReaperHost>(hInstance, rec, false);
    } catch (ReaperAPIException const& e) {
        NativeMessageBox::splashExtensionError(e.what(), rec->hwnd_main);
        return 0;
    }

    if (!rec) {
        // unload plugin
        return 0;
    }

    rec->Register("API_requestInputInstanceId", reinterpret_cast<void*>(&requestInputInstanceId));
    auto pluginCallbackHandler = EARPluginCallbackHandler::getInstance();
    rec->Register("API_registerPluginLoad", reinterpret_cast<void*>(&registerPluginLoad));

    auto api = reaper->api();

#ifndef __linux__
    // Linux requires libcurl even when using juce::URL
    // Can't guarentee installed, so let's just not do update check for now
    UpdateChecker updateChecker;
    if (updateChecker.getAutoCheckEnabled()) {
        updateChecker.doUpdateCheck(false, 1000);
    }
#endif

    auto reaperMainMenu = std::dynamic_pointer_cast<RawMenu>(reaper->getMenu(MenuID::MAIN_MENU));
    assert(reaperMainMenu);
    auto pluginRegistry = PluginRegistry::getInstance();

    // Item right-click menu

    auto admContextMenu = std::make_unique<SubMenu>("ADM");
    auto admContextMenuUpdateCallback = [api](MenuItem& item) {
        int numMediaItems = api->CountSelectedMediaItems(0);
        if (numMediaItems == 1) { // For now, lets just deal with one selection
            MediaItem* selMediaItem = api->GetSelectedMediaItem(0, 0);
            if (ImportAction::canMediaExplode_QuickCheck(*api, selMediaItem)) {
                item.setEnabled(true);
            } else {
                item.setEnabled(false);
            }
        } else {
            item.setEnabled(false);
        }
    };
    admContextMenu->updateCallback = admContextMenuUpdateCallback;

    // TODO: Would be useful to also have a "Explode AudioObjects to seperate tracks" option here,
    //       which just sticks takes on tracks with names but without plugins/routing/automation etc

    std::string explodeActionName("Explode using EAR Plugins");
    std::string explodeActionSID("ADM_EXPLODE_EAR");

    auto explodeAction = std::make_shared<StatefulAction<ImportAction>> (
      explodeActionName.c_str(),
      explodeActionSID.c_str(),
        std::make_unique<ImportAction>(hInstance, rec->hwnd_main, PluginRegistry::earPluginSuite),
        [](ReaperAPI& api, ImportAction& importer) {
            auto mediaItem = api.GetSelectedMediaItem(0, 0);
            if (mediaItem) {
                importer.import(mediaItem, api);
            }
            else {
                api.ShowMessageBox("Please select a source before running this action.", "ADM: Explode to Takes", 0);
            }
        });
    explodeAction->setEnabled(PluginRegistry::earPluginSuite->pluginSuiteUsable(*api));
    auto explodeActionId = reaper->addAction(explodeAction);
    auto explodeActionItem = std::make_unique<MenuAction>(explodeActionName.c_str(), explodeActionId);
    admContextMenu->insert(std::move(explodeActionItem), std::make_shared<EndOffset>(0));

    auto mediaContextMenu = reaper->getMenu(MenuID::MEDIA_ITEM_CONTEXT);
    mediaContextMenu->insert(
      std::move(admContextMenu),
      std::make_shared<AfterNamedItem>("Group", "common",
                                       defaultMenuPositions.at("Group"), *api));

    // Extensions Menu

    const std::string infoActionName("About EAR Production Suite");
    const std::string infoActionStrId("ADM_SHOW_EPS_INFO");

    auto infoAction = std::make_shared<SimpleAction> (
      infoActionName.c_str(),
      infoActionStrId.c_str(),
      [](admplug::ReaperAPI &api) {
        std::string title("EAR Production Suite");
        if(eps::versionInfoAvailable()) {
            title += " v";
            title += eps::baseVersion();
        }
        std::string msg;
        if(eps::versionInfoAvailable()) {
            msg += "Build Version:\n      ";
            msg += eps::currentVersion();
        } else {
            msg += "(Version information unavailable)";
        }
         msg += "\n\nhttps://ear-production-suite.ebu.io/";
        api.ShowMessageBox(msg.c_str(), title.c_str(), 0);
      }
    );

    const std::string btpMenuText("Browse tools and templates...");
    const std::string btpActionName("EAR Production Suite: Browse tools and templates");
    const std::string btpActionStrId("ADM_EPS_BROWSE");

    auto btpAction = std::make_shared<SimpleAction>(
        btpActionName.c_str(),
        btpActionStrId.c_str(),
        [](admplug::ReaperAPI& api) {
            auto path = ResourcePaths::getExtrasDirectory().getFullPathName().toStdString();
            if (!ResourcePaths::openDirectory(path)) {
                api.ShowMessageBox("Failed to open directory.", "Browse Tools and Templates", 0);
            }
        }
    );

    const std::string ucDisableMenuText("Disable update check on startup");
    const std::string ucDisableActionName("EAR Production Suite: Disable update check on startup");
    const std::string ucDisableActionStrId("ADM_EPS_DISABLE_UPDATE_CHECK");

    auto ucDisableAction = std::make_shared<SimpleAction>(
        ucDisableActionName.c_str(),
        ucDisableActionStrId.c_str(),
        [](admplug::ReaperAPI& api) {
        UpdateChecker updateChecker;
        updateChecker.setAutoCheckEnabled(false, true);
    }
    );

    const std::string ucEnableMenuText("Enable update check on startup");
    const std::string ucEnableActionName("EAR Production Suite: Enable update check on startup");
    const std::string ucEnableActionStrId("ADM_EPS_ENABLE_UPDATE_CHECK");

    auto ucEnableAction = std::make_shared<SimpleAction>(
        ucEnableActionName.c_str(),
        ucEnableActionStrId.c_str(),
        [](admplug::ReaperAPI& api) {
        UpdateChecker updateChecker;
        updateChecker.setAutoCheckEnabled(true, true);
    }
    );

    const std::string ucMenuText("Check for updates now...");
    const std::string ucActionName("EAR Production Suite: Check for updates now");
    const std::string ucActionStrId("ADM_EPS_UPDATE_CHECK");

    auto ucAction = std::make_shared<SimpleAction>(
        ucActionName.c_str(),
        ucActionStrId.c_str(),
        [](admplug::ReaperAPI& api) {
        UpdateChecker updateChecker;
        updateChecker.doUpdateCheck(true, 3000);
    }
    );

    // Make sure Extensions menu is added and then get it
    api->AddExtensionsMainMenu();
    auto reaperExtMenu = reaperMainMenu->getMenuByText(
        "E&xtensions", "common", -1 , *api);

    if(!reaperExtMenu) {
        // Extensions menu didn't appear for some reason - let's just use help menu
        reaperExtMenu = reaperMainMenu->getMenuByText(
            "&Help", "common", -1, *api);
    }

    // Populate menu
    if(reaperExtMenu) {
        auto epsMenu = std::make_unique<SubMenu>("EAR Production Suite");

        auto path = ResourcePaths::getExtrasDirectory();
        if (path.exists() && path.isDirectory()) {
            auto btpActionId = reaper->addAction(btpAction);
            auto btpActionItem = std::make_unique<MenuAction>(btpMenuText.c_str(), btpActionId);
            auto btpActionInserter = std::make_shared<StartOffset>(0);
            epsMenu->insert(std::move(btpActionItem), btpActionInserter);
        }

#ifndef __linux__
        auto ucActionId = reaper->addAction(ucAction);
        auto ucActionItem = std::make_unique<MenuAction>(ucMenuText.c_str(), ucActionId);
        epsMenu->insert(std::move(ucActionItem), std::make_shared<EndOffset>(0));

        auto ucEnableActionId = reaper->addAction(ucEnableAction);
        auto ucEnableActionItem = std::make_unique<MenuAction>(ucEnableMenuText.c_str(), ucEnableActionId);
        epsMenu->insert(std::move(ucEnableActionItem), std::make_shared<EndOffset>(0));

        auto ucDisableActionId = reaper->addAction(ucDisableAction);
        auto ucDisableActionItem = std::make_unique<MenuAction>(ucDisableMenuText.c_str(), ucDisableActionId);
        epsMenu->insert(std::move(ucDisableActionItem), std::make_shared<EndOffset>(0));
#endif

        auto infoActionId = reaper->addAction(infoAction);
        auto infoActionItem = std::make_unique<MenuAction>(infoActionName.c_str(), infoActionId);
        epsMenu->insert(std::move(infoActionItem), std::make_shared<EndOffset>(0));

        auto admExtMenuInserter = std::make_shared<StartOffset>(0);
        reaperExtMenu->insert(std::move(epsMenu), admExtMenuInserter);
        reaperExtMenu->init();
    }

    // File menu

    std::string createActionName("Create project from ADM BW64 file");
    std::string createActionSID("ADM_CREATE_PROJECT");

    auto createAction = std::make_shared<StatefulAction<ImportAction>> (
        createActionName.c_str(),
        createActionSID.c_str(),
        std::make_unique<ImportAction>(hInstance, rec->hwnd_main, PluginRegistry::earPluginSuite),
        [](ReaperAPI& api, ImportAction& importer) {

        api.Main_openProject("template:"); // This will TRY to start a blank project, but it will replace the current project if successful, so the user is prompted with the save dialog: yes, no, CANCEL
        auto res = api.IsProjectDirty(nullptr); // If the project is still dirty (1), the user must have canceled! (yes/no would save/not save and start a new clean project)
        if(res == 0) {
            char filename[4096];
            api.GetProjectPath(filename, 4096);
            auto filenameStr = std::string(filename);
            filenameStr += "/.wav";
            memcpy(filename, filenameStr.data(), filenameStr.length() + 1);
            if(api.GetUserFileNameForRead(filename, "ADM BW64 File to Open", "wav")) {
                filenameStr = std::string(filename);
                std::string errOut;
                if(ImportAction::canMediaExplode_QuickCheck(api, filenameStr, &errOut)) {
                    importer.import(filenameStr, api);
                } else {
                    std::string errMsg{ "Error: This file can not be imported.\n\nResponse: " };
                    errMsg += errOut;
                    api.ShowMessageBox(errMsg.c_str(), "ADM Open", 0);
                }
            }
        }
    });
    createAction->setEnabled(PluginRegistry::earPluginSuite->pluginSuiteUsable(*api));
    auto createActionId = reaper->addAction(createAction);
    auto createActionItem = std::make_unique<MenuAction>(createActionName.c_str(), createActionId);

    auto projectTemplateString = "Project &templates";
    auto admFileMenuInserter = std::make_shared<BeforeNamedItem>(
        projectTemplateString, "MENU_102",
        defaultMenuPositions.at(projectTemplateString), *api);
    auto reaperFileMenu = reaperMainMenu->getMenuByText(
        "&File", "common", defaultMenuPositions.at("&File"), *api);
      assert(reaperFileMenu);
    reaperFileMenu->insert(std::move(createActionItem), admFileMenuInserter);
    reaperFileMenu->init();

    // Insert menu

    auto admInsertMenu = std::make_unique<SubMenu>("Import ADM BW64 file in to current project");
    auto admInsertMenuUpdateCallback = [api](MenuItem& item) {};
    admInsertMenu->updateCallback = admInsertMenuUpdateCallback;

    // TODO: Would be useful to also have a "Import AudioObjects to seperate tracks" option here,
    //       which just sticks takes on tracks with names but without plugins/routing/automation etc

    std::string importActionName("Import using EAR Plugins");
    std::string importActionSID("ADM_IMPORT_EAR");

    auto importAction = std::make_shared<StatefulAction<ImportAction>> (
      importActionName.c_str(),
      importActionSID.c_str(),
        std::make_unique<ImportAction>(hInstance, rec->hwnd_main, PluginRegistry::earPluginSuite),
        [](ReaperAPI& api, ImportAction& importer) {

        char filename[4096];
        api.GetProjectPath(filename, 4096);
        auto filenameStr = std::string(filename);
        filenameStr += "/.wav";
        memcpy(filename, filenameStr.data(), filenameStr.length() + 1);
        if (api.GetUserFileNameForRead(filename, "ADM BW64 File to Import", "wav")) {
            filenameStr = std::string(filename);
            std::string errOut;
            if(ImportAction::canMediaExplode_QuickCheck(api, filenameStr), &errOut) {
                importer.import(filenameStr, api);
            } else {
                std::string errMsg{ "Error: This file can not be imported.\n\nResponse: " };
                errMsg += errOut;
                api.ShowMessageBox(errMsg.c_str(), "ADM Import", 0);
            }
        }
    });
    importAction->setEnabled(PluginRegistry::earPluginSuite->pluginSuiteUsable(*api));
    auto importActionId = reaper->addAction(importAction);
    auto importActionItem = std::make_unique<MenuAction>(importActionName.c_str(), importActionId);
    admInsertMenu->insert(std::move(importActionItem), std::make_shared<EndOffset>(0));

    auto reaperInsertMenu = reaperMainMenu->getMenuByText(
        "&Insert", "common", defaultMenuPositions.at("&Insert"), *api);
    assert(reaperInsertMenu);
    reaperInsertMenu->insert(std::move(admInsertMenu),
                             std::make_shared<AfterNamedItem>(
                               "Empty item", "MENU_102",
                               defaultMenuPositions.at("Empty item"), *api));
    reaperInsertMenu->init();

    return 1;
  }
}
