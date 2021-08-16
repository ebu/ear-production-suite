#define WIN32_LEAN_AND_MEAN

#include <memory>
#include <map>
#include <string>
#include "reaperhost.h"
#include "reaperapi.h"
#include "actionmanager.h"
#include "menu.h"
#include "importaction.h"
#include "pluginsuite.h"
#include "pluginregistry.h"
#include "ui_text.h"

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
    {"Project &templates", 8},
    {"Empty item", 2},
    {"Group", 4}};
}
#else
const std::map<const std::string, const int> defaultMenuPositions = {
    {"&File", 1},
    {"&Insert", 4},
    {"Project &templates", 8},
    {"Empty item", 2},
    {"Group", 4}};
}
#endif

using eps::TextID;
using eps::uiText;

extern "C" {
  int REAPER_PLUGIN_DLL_EXPORT REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec)
  {
    using namespace admplug;
    std::unique_ptr<ReaperHost> reaper;

    auto nonBlockingMessage = [rec](const char* text) {
      auto title = uiText(TextID::EXTENSION_ERROR_TITLE);
#ifdef WIN32
      // Windows version of Reaper locks up if you try show a message box during
      // splash
      winhelpers::NonBlockingMessageBox(text, title.c_str(),
                                        MB_ICONEXCLAMATION);
#else
      MessageBox(rec->hwnd_main, text, title.c_str(), MB_OK);
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

    auto reaperMainMenu = std::dynamic_pointer_cast<RawMenu>(reaper->getMenu(MenuID::MAIN_MENU));
    assert(reaperMainMenu);
    auto pluginRegistry = PluginRegistry::getInstance();
    auto api = reaper->api();
    int actionCounter = 0;

    // Item right-click menu

    auto admContextMenu =
        std::make_unique<SubMenu>(uiText(TextID::CONTEXT_MENU));
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
        auto actionName = uiText(TextID::EXPLODE_ACTION_PREFIX);
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
                  api.ShowMessageBox(
                      uiText(TextID::EXPLODE_ERROR_DESCRIPTION)
                          .c_str(),
                      uiText(TextID::EXPLODE_ERROR_TITLE).c_str(),
                      0);
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
    // File menu

  auto admFileMenu =
      std::make_unique<SubMenu>(uiText(TextID::CREATE_PROJECT_MENU));
  auto admFileMenuUpdateCallback = [api](MenuItem& item) {};
    admFileMenu->updateCallback = admFileMenuUpdateCallback;

    for (auto& pluginSuite : *pluginRegistry->getPluginSuites()) {
      std::string actionName(uiText(TextID::CREATE_PROJECT_ACTION_PREFIX));
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
                if (api.GetUserFileNameForRead(
                        filename,
                        uiText(TextID::CREATE_PROJECT_FILE_PROMPT)
                            .c_str(),
                        "wav")) {
                  filenameStr = std::string(filename);
                    if(ImportAction::canMediaExplode_QuickCheck(api, filenameStr)) {
                        importer.import(filenameStr, api);
                    } else {
                      api.ShowMessageBox(
                          uiText(TextID::CREATE_PROJECT_ERROR_DESCRIPTION)
                              .c_str(),
                          uiText(TextID::CREATE_PROJECT_ERROR_TITLE)
                              .c_str(),
                          0);
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

    auto admInsertMenu =
        std::make_unique<SubMenu>(uiText(TextID::IMPORT_MENU));
    auto admInsertMenuUpdateCallback = [api](MenuItem& item) {};
    admInsertMenu->updateCallback = admInsertMenuUpdateCallback;

    for (auto& pluginSuite : *pluginRegistry->getPluginSuites()) {
        std::string actionName(uiText(TextID::IMPORT_ACTION_PREFIX));
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
            if (api.GetUserFileNameForRead(
                    filename, uiText(TextID::IMPORT_FILE_PROMPT).c_str(),
                    "wav")) {
              filenameStr = std::string(filename);
                if(ImportAction::canMediaExplode_QuickCheck(api, filenameStr)) {
                    importer.import(filenameStr, api);
                } else {
                  api.ShowMessageBox(
                      uiText(TextID::IMPORT_ERROR_DESCRIPTION).c_str(),
                      uiText(TextID::IMPORT_ERROR_TITLE).c_str(), 0);
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
