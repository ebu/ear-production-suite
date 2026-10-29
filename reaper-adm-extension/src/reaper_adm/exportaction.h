#pragma once

#include "reaperapi.h"
#include "reaper_plugin.h"
#include <memory>
#include "exportaction_dialogcontrol.h"
#include "helper/nng_wrappers.h"

namespace admplug {

    class ExportManager
    {
    public:
        ~ExportManager();
        static ExportManager& getManager(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst, reaper_plugin_info_t *rec);
        static ExportManager& getManager();
        static std::shared_ptr<ReaperAPI> const& api() {return getManager().api_;};

    private:
        ExportManager(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst, reaper_plugin_info_t *rec);

        std::shared_ptr<ReaperAPI> api_;
        pcmsink_register_t admSinkReg;
        reaper_plugin_info_t *rec;
    };
};
