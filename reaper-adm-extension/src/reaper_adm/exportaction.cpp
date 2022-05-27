#include "exportaction.h"

ExportManager::ExportManager(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst, reaper_plugin_info_t *rec) : api{ api }, rec{ rec }
{
    if(!(this->api)) {
        throw std::logic_error("api is null, make sure when getManager() is first called, a valid API pointer is provided.");
    }

    if(!rec) {
        throw std::logic_error("rec is null, make sure when getManager() is first called, a valid API pointer is provided.");
    }

    auto dialogControl = RenderDialogControl::getInstance(api, inst);
    admSinkReg = pcmsink_register_t {
            ExportInfo.GetFmt,
            GetExtensionIfMatch,
            dialogControl.ShowConfig,
            CreateSinkIfMatch };
    if (rec->Register("pcmsink", &admSinkReg)) {
        printf("Registered normal Sink!\n");
    }

    nngHandle = std::make_unique<NngSelfRegister>();
}

ExportManager::~ExportManager() {
    if (rec && rec->Register("-pcmsink", &admSinkReg)) {
        printf("DeRegistered Sink!\n");
    }
    nngHandle.reset(); // This handle will be lost anyway, but just to be explicit... (calls nng_fini to tidy up globals and false mem leak messages)
}

ExportManager &ExportManager::getManager(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst, reaper_plugin_info_t *rec)
{
    static ExportManager exportMan {api, inst, rec};
    return exportMan;
}

ExportManager &ExportManager::getManager()
{
    return getManager(nullptr, nullptr, nullptr);
}

