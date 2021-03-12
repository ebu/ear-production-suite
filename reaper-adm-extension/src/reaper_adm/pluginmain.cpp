#define WIN32_LEAN_AND_MEAN

#include <memory>
#include "reaperhost.h"
#include "reaperapi.h"
#include "actionmanager.h"
#include "menu.h"
#include "admmetadata.h"
#include "importaction.h"
#include "exportaction.h"
#include "pluginsuite.h"
#include "pluginregistry.h"

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

extern "C" {
  int REAPER_PLUGIN_DLL_EXPORT REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec)
  {
    using namespace admplug;
    std::unique_ptr<ReaperHost> reaper;

    auto nonBlockingMessage = [rec](const char* text) {
#ifdef WIN32
        // Windows version of Reaper locks up if you try show a message box during splash
        winhelpers::NonBlockingMessageBox(text, "ADM Extension Error", MB_ICONEXCLAMATION);
#else
        MessageBox(rec->hwnd_main, text, "ADM Extension Error", MB_OK);
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

    if(!rec) {
        // unload plugin
        return 0;
    }

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
    mediaContextMenu->insert(std::move(admContextMenu), std::make_shared<AfterNamedItem>("Group"));

    // File menu

    auto admFileMenu = std::make_unique<SubMenu>("Create project from ADM file");
    auto admFileMenuUpdateCallback = [api](MenuItem& item) {};
    admFileMenu->updateCallback = admFileMenuUpdateCallback;

    for (auto& pluginSuite : *pluginRegistry->getPluginSuites()) {
        std::string actionName("Create from ADM using ");
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
                if(api.GetUserFileNameForRead(filename, "ADM File to Open", "wav")) {
                    filenameStr = std::string(filename);
                    if(ImportAction::canMediaExplode_QuickCheck(api, filenameStr)) {
                        importer.import(filenameStr, api);
                    } else {
                        api.ShowMessageBox("Error: This file can not be imported.", "ADM Open", 0);
                    }
                }
            }
        });
        explodeAction->setEnabled(pluginSuite.second->pluginSuiteUsable(*api));
        auto explodeId = reaper->addAction(explodeAction);
        auto explodeItem = std::make_unique<MenuAction>(actionName.c_str(), explodeId);
        admFileMenu->insert(std::move(explodeItem), std::make_shared<EndOffset>(0));
    }

    auto reaperFileMenu = reaperMainMenu->getMenuByText("File");
    assert(reaperFileMenu);
    reaperFileMenu->insert(std::move(admFileMenu), std::make_shared<BeforeNamedItem>("Project templates"));
    reaperFileMenu->init();

    // Insert menu

    auto admInsertMenu = std::make_unique<SubMenu>("Import ADM file in to current project");
    auto admInsertMenuUpdateCallback = [api](MenuItem& item) {};
    admInsertMenu->updateCallback = admInsertMenuUpdateCallback;

    for (auto& pluginSuite : *pluginRegistry->getPluginSuites()) {
        std::string actionName("Import ADM file using ");
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
            if (api.GetUserFileNameForRead(filename, "ADM File to Import", "wav")) {
                filenameStr = std::string(filename);
                if(ImportAction::canMediaExplode_QuickCheck(api, filenameStr)) {
                    importer.import(filenameStr, api);
                } else {
                    api.ShowMessageBox("Error: This file can not be imported.", "ADM Import", 0);
                }
            }
        });
        explodeAction->setEnabled(pluginSuite.second->pluginSuiteUsable(*api));
        auto explodeId = reaper->addAction(explodeAction);
        auto explodeItem = std::make_unique<MenuAction>(actionName.c_str(), explodeId);
        admInsertMenu->insert(std::move(explodeItem), std::make_shared<EndOffset>(0));
    }

    auto reaperInsertMenu = reaperMainMenu->getMenuByText("Insert");
    assert(reaperInsertMenu);
    reaperInsertMenu->insert(std::move(admInsertMenu), std::make_shared<AfterNamedItem>("Empty item"));
    reaperInsertMenu->init();

    return 1;
  }
}
