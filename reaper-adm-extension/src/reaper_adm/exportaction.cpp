#include "exportaction.h"
#include "exportaction_pcmsink.h"

namespace admplug {
namespace {
    bool configMatchesSink(const void *config, int configByteCount) {
        if (configByteCount >= 4) {
            auto const fourcc = reinterpret_cast<const unsigned int*>(config);
            return *fourcc == PCM_sink_adm::fourCC;
        }
        return false;
    }

    unsigned int getSinkFormatDescription(const char **description)
    {
        if (description) {
            *description = PCM_sink_adm::FILE_FORMAT_DESCRIPTION;
        }
        return PCM_sink_adm::fourCC;
    }

    const char* GetExtensionIfMatch(const void *config, int configByteCount)
    {
        if (configMatchesSink(config, configByteCount)) {
            return PCM_sink_adm::FILE_FORMAT_EXTENSION;
        }
        return nullptr;
    }

    PCM_sink* CreateSinkIfMatch(const char *filename, void *cfg, int cfg_l, int nch, int srate, bool buildpeaks)
    {
        if (configMatchesSink(cfg, cfg_l)) {
        // note this is not a leak, reaper takes ownership of the sink and
        // deletes it when the render dialog is closed
        PCM_sink_adm *v = new PCM_sink_adm(ExportManager::api(), filename, cfg,
                                           cfg_l, nch, srate, buildpeaks);
        return v;
      }
      return nullptr;
    }

    HWND ShowConfigIfMatch(const void *cfg, int cfg_l, HWND parent)
    {
        if (configMatchesSink(cfg, cfg_l)) {
            return RenderDialogControl::ShowConfig(cfg, cfg_l, parent);
        }
        return nullptr;
    }
}
}

ExportManager::ExportManager(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst, reaper_plugin_info_t *rec) : api_{ api }, rec{ rec }
{
    if(!(this->api_)) {
        throw std::logic_error("api is null, make sure when getManager() is first called, a valid API pointer is provided.");
    }

    if(!rec) {
        throw std::logic_error("rec is null, make sure when getManager() is first called, a valid API pointer is provided.");
    }

    auto dialogControl = RenderDialogControl::getInstance(api, inst);
    admSinkReg = pcmsink_register_t {
        &getSinkFormatDescription,
        &GetExtensionIfMatch,
        &ShowConfigIfMatch,
        &CreateSinkIfMatch };
    if (rec->Register("pcmsink", &admSinkReg) == 1) {
        std::cout << "Registered ADM export sink\n";
    }
}

ExportManager::~ExportManager() {
    if (rec != nullptr && rec->Register("-pcmsink", &admSinkReg) == 1) {
    }
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