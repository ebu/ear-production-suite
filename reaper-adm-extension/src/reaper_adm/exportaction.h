#pragma once

#include "reaperapi.h"
#include "reaper_plugin.h"
#include <memory>
#include "exportaction_pcmsink.h"
#include "exportaction_dialogcontrol.h"
#include "nng_wrappers.h"

namespace admplug {

    class ExportManager
    {
    public:
        ~ExportManager();
        static ExportManager& getManager(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst, reaper_plugin_info_t *rec);
        static ExportManager& getManager();
        static void closeManager();

        static struct ExportInfoStruct {
            static const unsigned int SINK_FOURCC{ REAPER_FOURCC('a', 'd', 'm', ' ') };

            static unsigned int GetFmt(const char **desc)
            {
                if (desc) *desc = "ADM File";
                return SINK_FOURCC;
            }

            static const char* GetExtension()
            {
                return "wav";
            }

            static bool cfgMatch(const void *cfg, int cfg_l) {
                return (*((int *)cfg) == SINK_FOURCC);
            }

        } ExportInfo;

    private:
        static ExportManager* exportMan;

        static const char* GetExtensionIfMatch(const void *cfg, int cfg_l)
        {
            return ExportInfo.cfgMatch(cfg, cfg_l)? ExportInfo.GetExtension() : NULL;
        }

        static PCM_sink* CreateSinkIfMatch(const char *filename, void *cfg, int cfg_l, int nch, int srate, bool buildpeaks)
        {
            if (ExportInfo.cfgMatch(cfg, cfg_l))
            {
                PCM_sink_adm *v = new PCM_sink_adm(ExportManager::getManager().api, filename, cfg, cfg_l, nch, srate, buildpeaks);
                return v;
                delete v; //Obviously won't reach here, but Reaper itself seems to call the sink destructor when done writing
            }
            return nullptr;
        }

        static HWND ShowConfigIfMatch(const void *cfg, int cfg_l, HWND parent)
        {
            return ExportInfo.cfgMatch(cfg, cfg_l)? RenderDialogControl::getInstance()->ShowConfig(cfg, cfg_l,parent) : 0;
        }

        ExportManager(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst, reaper_plugin_info_t *rec);

        std::shared_ptr<ReaperAPI> api;
        pcmsink_register_t admSinkReg;
        std::unique_ptr<RenderDialogControl> dialogControl;
        std::shared_ptr<NngSelfRegister> nngHandle;
    };
};
