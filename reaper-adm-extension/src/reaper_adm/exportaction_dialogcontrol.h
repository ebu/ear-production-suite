#pragma once

#include <memory>
#include <map>

#include <WDL/wdltypes.h>

#include "reaperapi.h"
#include "reaper_plugin.h"
#include "exportaction_admsourcescontainer.h"
#include "export_dialog/render_dialog_modifier.hpp"

namespace admplug {
class RenderDialogControl;

// This represents the custom control panel we add to the Render dialog in
// reaper when the ADM BW64 file format is selected. ShowConfig() creates it, and
// admCfgDialogProc() is the message handler.
class RenderConfigDialog {
public:

    RenderConfigDialog(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst);
    ~RenderConfigDialog();

    HWND ShowConfig(const void *cfg, int cfg_l, HWND parent);


private:
    friend class RenderDialogControl;

    std::shared_ptr<ReaperAPI> reaperApi;
    REAPER_PLUGIN_HINSTANCE reaperInst;

    std::shared_ptr<AdmExportHandler> admExportHandler;
    RenderDialogModifier dialogModifier;

    std::string getAdmExportVstsInfoString();
    int loadConfig(void **loadedData, char const *description);
    void saveState(HWND hwnd, void *data);
    void saveConfig(HWND hwnd);
    INT_PTR initDialog(HWND hwnd, void *cfgData, int cfgLength);
    WDL_DLGRET wavecfgDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

    bool startedPrepareDialogControls{ false };
    ExportProfile selectedProfile{ExportProfile::EBU_PRODUCTION_3392};
};

class RenderDialogControl {
public:
    static RenderDialogControl& getInstance(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst);
    static RenderDialogControl& getInstance();

    static WDL_DLGRET wavecfgDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        return dialog().wavecfgDlgProc(hwndDlg, uMsg, wParam, lParam);
    }

    static HWND ShowConfig(const void *cfg, int cfg_l, HWND parent) {
        return dialog().ShowConfig(cfg, cfg_l,parent);
    }

    ExportProfile getProfile() {
        return dialog().selectedProfile;
    }

private:
    RenderDialogControl(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst) : api_{api}, dialog_{api, inst} {}
    static RenderConfigDialog& dialog() {
        return getInstance().dialog_;
    }
    std::shared_ptr<ReaperAPI> api_;
    RenderConfigDialog dialog_;
};
}
