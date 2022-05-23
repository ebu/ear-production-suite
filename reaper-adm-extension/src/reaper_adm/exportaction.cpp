#include "exportaction.h"

ExportManager* ExportManager::exportMan = nullptr;

ExportManager::ExportManager(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst, reaper_plugin_info_t *rec) : api{ api }
{
    if(!(this->api)) {
        throw std::logic_error("api is null, make sure when getManager() is first called, a valid API pointer is provided.");
    }

    if(!rec) {
        throw std::logic_error("rec is null, make sure when getManager() is first called, a valid API pointer is provided.");
    }

    dialogControl = std::make_unique<RenderDialogControl>(api, inst);

    admSinkReg = pcmsink_register_t {
            ExportInfo.GetFmt,
            GetExtensionIfMatch,
            dialogControl->ShowConfig,
            CreateSinkIfMatch };

    if (rec->Register("pcmsink", &admSinkReg)) {
        printf("Registered normal Sink!\n");
    }
}

ExportManager::~ExportManager() {
    nngHandle.reset(); // This handle will be lost anyway, but just to be explicit...
}

ExportManager &ExportManager::getManager(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst, reaper_plugin_info_t *rec)
{
    if(!exportMan) exportMan = new ExportManager(api, inst, rec);
    return *exportMan;
}

ExportManager &ExportManager::getManager()
{
    return getManager(nullptr, nullptr, nullptr);
}

void ExportManager::closeManager()
{
    delete exportMan;
}
