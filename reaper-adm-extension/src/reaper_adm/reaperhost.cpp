#include "reaperhost.h"
#include "reaper_plugin_functions.h"
#include "reaperapiimpl.h"
#include "actionmanager.h"
#include "menu.h"

admplug::ReaperHost::ReaperHost(REAPER_PLUGIN_HINSTANCE plug_hinstance, reaper_plugin_info_t *plug_info, bool testAPI) :
    plug_info{ plug_info },
    plug_hinstance{ plug_hinstance },
    actionMan{ ActionManager::getManager(api()) },
    menuMan{ MenuManager::getManager(api().get(), plug_info) },
    exportMan{ ExportManager::getManager(api(), &plug_hinstance, plug_info) }
{
    // plug_info will be null if the extension is closing down!

    if (plug_info != nullptr) {

        // Extension Starting!!!

        if (plug_info->caller_version != REAPER_PLUGIN_VERSION) {
            throw ReaperAPIException{ "Incompatible version of REAPER plugin API." };
        }
        if (!plug_info->GetFunc) {
            throw ReaperAPIException{ "Host has not provided GetFunc pointer. Unable to resolve API functions." };
        }
        if (testAPI) {
            auto error_count = REAPERAPI_LoadAPI(plug_info->GetFunc);
            if (error_count > 1) {
                throw FuncResolutionException{ error_count, " API functions not found.\nThis could lead to issues when using the EAR Prodution Suite extension.\n\nPlease consider updating your REAPER version." };
            }
        }
    }
    else {

        // Extension Closing Down!!!

        prepareToClose();
    }
}

admplug::ReaperHost::~ReaperHost() {
    // Use with caution! This destructor will run every time the REAPER_PLUGIN_ENTRYPOINT runs!
    // i.e, it will construct and destruct on start up of the extension,
    //      AND construct and destruct on shutdown of the extension!
}

void admplug::ReaperHost::prepareToClose()
{
    ExportManager::closeManager();
    // If any other managers need to tidy up before closing, put their clean-up methods here
}

HWND admplug::ReaperHost::parentWindow()
{
    return plug_info->hwnd_main;
}

std::shared_ptr<admplug::ReaperAPI> admplug::ReaperHost::api()
{
    if (!api_ptr) api_ptr = (std::make_shared<ReaperAPIImpl>(*plug_info));
    return api_ptr;
}

std::shared_ptr<admplug::ActionIdentifier> admplug::ReaperHost::addAction(std::shared_ptr<admplug::Action> action)
{
    return actionMan.addAction(action);
}

std::shared_ptr<admplug::TopLevelMenu> admplug::ReaperHost::getMenu(admplug::MenuID menuId)
{
    return menuMan.getReaperMenu(menuId);
}

admplug::FuncResolutionException::FuncResolutionException(int errorCount, std::string what) : std::runtime_error{std::to_string(errorCount) + what}
{
}

admplug::ReaperAPIException::ReaperAPIException(std::string what) : std::runtime_error{what}
{

}
