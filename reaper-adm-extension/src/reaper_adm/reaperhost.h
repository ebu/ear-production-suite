#pragma once
#include <stdexcept>
#include <string>
#include <memory>
#define REAPERAPI_IMPLEMENT
#include "reaper_plugin.h"
#include "actionmanager.h"
#include "menu.h"
#include "exportaction.h"

namespace admplug {

class ReaperAPI;
class ActionManager;
class MenuManager;
class ExportManager;

class ReaperAPIException : public std::runtime_error {
public:
    ReaperAPIException(std::string what);
};

class FuncResolutionException : public ReaperAPIException {
public:
    FuncResolutionException(int errorCount);
};

class ReaperHost {
public:
    ReaperHost(REAPER_PLUGIN_HINSTANCE plug_hinstance, reaper_plugin_info_t* plug_info);
    ~ReaperHost();
    void prepareToClose();

    HWND parentWindow();
    std::shared_ptr<ReaperAPI> api();

    std::shared_ptr<ActionIdentifier> addAction(std::shared_ptr<Action> action);
    std::shared_ptr<TopLevelMenu> getMenu(MenuID menuId);

private:
    reaper_plugin_info_t* plug_info;
    REAPER_PLUGIN_HINSTANCE plug_hinstance;

    std::shared_ptr<ReaperAPI> api_ptr;

    ActionManager& actionMan;
    MenuManager& menuMan;
    ExportManager& exportMan;
};

}
