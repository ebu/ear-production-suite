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
#include "pluginsuite_ear.h"
#include <version/eps_version.h>

#ifdef WIN32
#include "win_nonblock_msg.h"
#endif

/*

// USEFUL FOR TRACING MEMORY LEAKS BY OBJECT ALLOCATION NUMBER

#ifdef WIN32
#include "win_mem_debug.h"
#endif

#include <crtdbg.h>

#ifdef _DEBUG

#pragma warning(disable:4074)//initializers put in compiler reserved initialization area
#pragma init_seg(compiler)//global objects in this file get constructed very early on

struct CrtBreakAllocSetter {
    CrtBreakAllocSetter() {
        _crtBreakAlloc = 169;
    }
};

CrtBreakAllocSetter g_crtBreakAllocSetter;

#endif//_DEBUG

struct CrtBreakAllocSetter {
    CrtBreakAllocSetter() {
        CRT_SET
        //_crtBreakAlloc = 169;
    }
};

CrtBreakAllocSetter g_crtBreakAllocSetter;
*/

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
#else
const std::map<const std::string, const int> defaultMenuPositions = {
    {"&File", 1},
    {"&Insert", 4},
    {"&Help", 10},
    {"E&xtensions", 9},
    {"Project &templates", 8},
    {"Empty item", 2},
    {"Group", 4}};
}
#endif

extern "C" {
  void registerPluginLoad(std::function<void(std::string const&)> callback) {
      auto cbh = EARPluginCallbackHandler::getInstance();
      cbh->setPluginCallback(callback);
  }

  int REAPER_PLUGIN_DLL_EXPORT REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec)
  {
    using namespace admplug;
    std::unique_ptr<ReaperHost> reaper;

    auto nonBlockingMessage = [rec](const char* errMsg) {
        std::string text{ errMsg };
        if(eps::versionInfoAvailable()) {
            text += "\n\n[EAR Production Suite v";
            text += eps::currentVersion();
            text += "]";
        } else {
            text += "\n\n[EAR Production Suite version information unavailable!]";
        }
#ifdef WIN32
        // Windows version of Reaper locks up if you try show a message box during splash
        winhelpers::NonBlockingMessageBox(text, "EAR Production Suite - Extension Error", MB_ICONEXCLAMATION);
#else
        MessageBox(rec->hwnd_main, text.c_str(), "EAR Production Suite - Extension Error", MB_OK);
#endif
    };

    try {
        reaper = std::make_unique<ReaperHost>(hInstance, rec);
    } catch (FuncResolutionException const& e) {
        nonBlockingMessage(e.what());
        reaper = std::make_unique<ReaperHost>(hInstance, rec, false);
    } catch (ReaperAPIException const& e) {
        nonBlockingMessage(e.what());
        return 0;
    }

    if (!rec) {
        // unload plugin
        return 0;
    }

    auto pluginCallbackHandler = EARPluginCallbackHandler::getInstance();
    rec->Register("API_registerPluginLoad", &registerPluginLoad);

    auto reaperMainMenu = std::dynamic_pointer_cast<RawMenu>(reaper->getMenu(MenuID::MAIN_MENU));
    assert(reaperMainMenu);
    auto pluginRegistry = PluginRegistry::getInstance();
    auto api = reaper->api();
    int actionCounter = 0;

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

    for (auto& pluginSuite : *pluginRegistry->getPluginSuites()) {
        std::string actionName("Explode using ");
        actionName += pluginSuite.first;
        std::string actionSID("ADM_EXPLODE_");
        actionSID += std::to_string(actionCounter++);

        auto explodeAction = std::make_shared<StatefulAction<ImportAction>> (
            actionName.c_str(),
            actionSID.c_str(),
            std::make_unique<ImportAction>(hInstance, rec->hwnd_main, pluginSuite.second),
            [pluginSuite](ReaperAPI& api, ImportAction& importer) {
                auto mediaItem = api.GetSelectedMediaItem(0, 0);
                if (mediaItem) {
                    importer.import(mediaItem, api);
                }
                else {
                    api.ShowMessageBox("Please select a source before running this action.", "ADM: Explode to Takes", 0);
                }
            });
        explodeAction->setEnabled(pluginSuite.second->pluginSuiteUsable(*api));
        auto explodeId = reaper->addAction(explodeAction);
        auto explodeItem = std::make_unique<MenuAction>(actionName.c_str(), explodeId);
        admContextMenu->insert(std::move(explodeItem), std::make_shared<EndOffset>(0));
    }

    auto mediaContextMenu = reaper->getMenu(MenuID::MEDIA_ITEM_CONTEXT);
    mediaContextMenu->insert(
      std::move(admContextMenu),
      std::make_shared<AfterNamedItem>("Group", "common",
                                       defaultMenuPositions.at("Group"), *api));

    // Extensions Menu

    std::string actionName("About EAR Production Suite");
    std::string actionSID("ADM_SHOW_EPS_INFO");

    auto infoAction = std::make_shared<SimpleAction> (
      actionName.c_str(),
      actionSID.c_str(),
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

    api->AddExtensionsMainMenu();
    auto reaperExtMenu = reaperMainMenu->getMenuByText(
        "E&xtensions", "common", -1 , *api);

    if(!reaperExtMenu) {
        reaperExtMenu = reaperMainMenu->getMenuByText(
            "&Help", "common", -1, *api);
    }

    if(reaperExtMenu) {
        auto infoActionId = reaper->addAction(infoAction);
        auto infoActionItem = std::make_unique<MenuAction>(actionName.c_str(), infoActionId);
        auto admExtMenuInserter = std::make_shared<StartOffset>(0);

        reaperExtMenu->insert(std::move(infoActionItem), admExtMenuInserter);
        reaperExtMenu->init();
    }

    // File menu

    auto admFileMenu = std::make_unique<SubMenu>("Create project from ADM BW64 file");
    auto admFileMenuUpdateCallback = [api](MenuItem& item) {};
    admFileMenu->updateCallback = admFileMenuUpdateCallback;

    for (auto& pluginSuite : *pluginRegistry->getPluginSuites()) {
        std::string actionName("Create using ");
        actionName += pluginSuite.first;
        std::string actionSID("ADM_CREATE_PROJECT_");
        actionSID += std::to_string(actionCounter++);

        auto explodeAction = std::make_shared<StatefulAction<ImportAction>> (
            actionName.c_str(),
            actionSID.c_str(),
            std::make_unique<ImportAction>(hInstance, rec->hwnd_main, pluginSuite.second),
            [pluginSuite](ReaperAPI& api, ImportAction& importer) {

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
        explodeAction->setEnabled(pluginSuite.second->pluginSuiteUsable(*api));
        auto explodeId = reaper->addAction(explodeAction);
        auto explodeItem = std::make_unique<MenuAction>(actionName.c_str(), explodeId);
        admFileMenu->insert(std::move(explodeItem), std::make_shared<EndOffset>(0));
    }

  auto projectTemplateString = "Project &templates";
  auto admFileMenuInserter = std::make_shared<BeforeNamedItem>(
      projectTemplateString, "MENU_102",
      defaultMenuPositions.at(projectTemplateString), *api);
  auto reaperFileMenu = reaperMainMenu->getMenuByText(
      "&File", "common", defaultMenuPositions.at("&File"), *api);
    assert(reaperFileMenu);
  reaperFileMenu->insert(std::move(admFileMenu), admFileMenuInserter);
    reaperFileMenu->init();

    // Insert menu

    auto admInsertMenu = std::make_unique<SubMenu>("Import ADM BW64 file in to current project");
    auto admInsertMenuUpdateCallback = [api](MenuItem& item) {};
    admInsertMenu->updateCallback = admInsertMenuUpdateCallback;

    for (auto& pluginSuite : *pluginRegistry->getPluginSuites()) {
        std::string actionName("Import using ");
        actionName += pluginSuite.first;
        std::string actionSID("ADM_IMPORT_");
        actionSID += std::to_string(actionCounter++);

        auto explodeAction = std::make_shared<StatefulAction<ImportAction>> (
            actionName.c_str(),
            actionSID.c_str(),
            std::make_unique<ImportAction>(hInstance, rec->hwnd_main, pluginSuite.second),
            [pluginSuite](ReaperAPI& api, ImportAction& importer) {

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
        explodeAction->setEnabled(pluginSuite.second->pluginSuiteUsable(*api));
        auto explodeId = reaper->addAction(explodeAction);
        auto explodeItem = std::make_unique<MenuAction>(actionName.c_str(), explodeId);
        admInsertMenu->insert(std::move(explodeItem), std::make_shared<EndOffset>(0));
    }

  auto reaperInsertMenu = reaperMainMenu->getMenuByText(
      "&Insert", "common", defaultMenuPositions.at("&Insert"), *api);
  assert(reaperInsertMenu);
  reaperInsertMenu->insert(std::move(admInsertMenu),
                           std::make_shared<AfterNamedItem>(
                               "Empty item", "MENU_102",
                               defaultMenuPositions.at("Empty item"), *api));
    assert(reaperInsertMenu);
    reaperInsertMenu->init();

    return 1;
  }
}
