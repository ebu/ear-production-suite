#include "exportaction_dialogcontrol.h"

#include <WDL/swell/swell.h>
#include <WDL/filewrite.h>

#include "exportaction.h"
#include "exportaction_pcmsink.h"
#include "exportaction_issues.h"
#include "resource.h"

#include <reaper_plugin_functions.h>
#include "export_dialog/render_dialog_modifier.hpp"

#define TIMER_ID 1

namespace admplug {
namespace {
constexpr unsigned int DIALOG_STATE_SIZE = 8;
}
RenderConfigDialog::RenderConfigDialog(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst) : reaperApi{ api }, reaperInst{ *inst }, dialogModifier({api}) {
}

RenderConfigDialog::~RenderConfigDialog() {
}


HWND RenderConfigDialog::ShowConfig(const void *cfg, int cfg_l, HWND parent)
{
    const void *x[2] = { cfg, reinterpret_cast<void*>(cfg_l)};
    return CreateDialogParam(reaperInst, MAKEINTRESOURCE(IDD_EXPORT), parent,
        RenderDialogControl::wavecfgDlgProc,
        (LPARAM)x);
}

std::string RenderConfigDialog::getAdmExportVstsInfoString() {
    std::string itemStarter{ "\r\n - " };
    std::string op;

    auto exportSources = admExportHandler->getAdmExportSources();
    if(exportSources) {
        op += "Using export source: \"";
        op += admExportHandler->getAdmExportSources()->getExportSourcesName();
        op += "\"";
    } else {
        op += "No suitable export sources!";
    }

    auto errors = admExportHandler->generateExportErrorStrings();
    auto warnings = admExportHandler->generateExportWarningStrings();
    auto infos = admExportHandler->generateExportInfoStrings();

    if (errors.size() > 0) {
        op.append("\r\n\r\nERRORS:");
        for(auto &msg : errors) {
            op.append(itemStarter);
            op.append(msg);
        }
    }

    if (warnings.size() > 0) {
        op.append("\r\n\r\nWARNINGS:");
        for(auto &msg : warnings) {
            op.append(itemStarter);
            op.append(msg);
        }
    }

    if (infos.size() > 0) {
        op.append("\r\n");
        if(warnings.size() > 0 || errors.size() > 0) {
            op.append("\r\nINFO:");
        }
        for(auto &msg : infos) {
            op.append(itemStarter);
            op.append(msg);
        }
    }

    return op;
}

int RenderConfigDialog::loadConfig(void **loadedData, char const *description) {
    static WDL_HeapBuf CONFIG_BUFFER;
    const char *fileName = get_ini_file();
    auto configLength =
        GetPrivateProfileInt(description, "default_size", 0, fileName);
    if (configLength < 1) {
        return 0;
    }
    if (GetPrivateProfileStruct(description, "default",
                                CONFIG_BUFFER.Resize(configLength), configLength,
                                fileName)) {
        *loadedData = CONFIG_BUFFER.Get();
        return configLength;
                                }
    return 0;
}

namespace {
constexpr int configSize() {
    return 8;
}
const char* INI_CONFIG_DESCRIPTION = "ADM export defaults";
}

// serialize the state to data
void RenderConfigDialog::saveState(HWND hwnd, void* data) {
    auto profile = static_cast<int>(selectedProfile);
    auto dataStream = reinterpret_cast<char*>(data);
    auto fourccOutput = reinterpret_cast<int*>(dataStream);
    *fourccOutput = PCM_sink_adm::fourCC;
    auto profileOutput = reinterpret_cast<int*>(dataStream + 4);
    *profileOutput = REAPER_MAKELEINT(profile);
}

// write out state to reaper ini file
void RenderConfigDialog::saveConfig(HWND hwnd) {
    char data[configSize()];
    saveState(hwnd, &data);
    char length_buffer[64];
    snprintf(length_buffer, 64, "%d", configSize());
    const char* fileName = get_ini_file();
    WritePrivateProfileString(INI_CONFIG_DESCRIPTION, "default_size", length_buffer, fileName);
    WritePrivateProfileStruct(INI_CONFIG_DESCRIPTION, "default", data, configSize(), fileName);
}

WDL_DLGRET RenderConfigDialog::initDialog(HWND hwnd, void* cfgData, int cfgLength) {
    // we've not been given a saved config
    if (cfgLength < 8) {
        cfgLength = loadConfig(&cfgData, INI_CONFIG_DESCRIPTION);
    }
    // the load was successful
    if (cfgLength >= 8) {
        selectedProfile = [cfgData]() {
            // skip FOURCC
            auto data = reinterpret_cast<char*>(cfgData) + 4;
            int profile = REAPER_MAKELEINT(*reinterpret_cast<int*>(data));
            return static_cast<ExportProfile>(profile);
        }();
    }
    // If the load failed (first run or corrupted file), we'll just continue
    // with values initialized from the constructor then save whatever was selected
    // as the defaults for next time when the dialog is closed

    auto profileComboBox = GetDlgItem(hwnd, IDC_COMBO2);
    SendMessage(profileComboBox, CB_ADDSTRING, 0, reinterpret_cast<INT_PTR>(static_cast<LPCSTR>("EBU Production Profile Tech 3392")));
    SendMessage(profileComboBox, CB_ADDSTRING, 0, reinterpret_cast<INT_PTR>(static_cast<LPCSTR>("EBU Production Profile Tech 3393")));
    SendMessage(profileComboBox, CB_SETCURSEL, static_cast<WPARAM>(selectedProfile), static_cast<LPARAM>(0));
    // this is called when the dialog is initialized
    startedPrepareDialogControls = false;
    return 0;

}

WDL_DLGRET RenderConfigDialog::wavecfgDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        void *cfgData = ((void **)lParam)[0];
        int cfgDataLength=reinterpret_cast<INT_PTR>(((void**)lParam)[1]);
        initDialog(hwndDlg, cfgData, cfgDataLength);
    }

    case WM_PAINT:
    {
        // Have to do our control takeover in paint - on init, no controls are ready to take over yet!
        if(!startedPrepareDialogControls) {
            // Doesn't like UpdateWindow being called in paint... set up a timer to run dialog configuration routine.
            dialogModifier.startPreparingRenderControls(hwndDlg);
        }

        return 0;
    }

    case WM_TIMER:
    {
        KillTimer(hwndDlg, TIMER_ID);
        admExportHandler = std::make_shared<AdmExportHandler>(static_cast<ExportProfile>(selectedProfile));
        admExportHandler->repopulate(*reaperApi);
        auto warnings = dialogModifier.takeOverRenderControls(hwndDlg, admExportHandler);
        auto infoPaneText = warnings + getAdmExportVstsInfoString();
        SetWindowText(GetDlgItem(hwndDlg, IDC_INFOPANE), LPCSTR(infoPaneText.c_str()));
        EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_REFRESH), true);
        UpdateWindow(hwndDlg);
        admExportHandler.reset(); // We MUST reset this, otherwise we keep NNG ports open which the PCM sink will want to use if we render
        return 0;
    }

        // called on user interaction
    case WM_COMMAND:
    {
        auto controlId = LOWORD(wParam);
        auto notificationType = HIWORD(wParam);

        if (controlId == IDC_COMBO2 && notificationType == CBN_SELCHANGE) {
            int itemIndex =
                SendMessage(reinterpret_cast<HWND>(lParam), CB_GETCURSEL, 0, 0);
            if (itemIndex != CB_ERR) {
                selectedProfile = static_cast<ExportProfile>(itemIndex);
            }
        }

        if (controlId == IDC_BUTTON_REFRESH && notificationType == BN_CLICKED) {
            // Repopulate ADM info
            dialogModifier.startPreparingRenderControls(hwndDlg);
        }
        return 0;
    }

        // gets called with just wParam to retrieve size
        // then gets called with lParam to get state
    case WM_USER + 1024:
    {
        // this gets called to retrieve the settings!

        if (wParam) {
            *(reinterpret_cast<unsigned int *>(wParam)) = DIALOG_STATE_SIZE;
        }
        if (lParam)
        {
            saveState(hwndDlg, reinterpret_cast<void*>(lParam));
        }
        return 0;
    }

    case WM_DESTROY:
    {
        // restoreControls();
        saveConfig(hwndDlg);
        return 0;
    }

    }
    return 0;
}

RenderDialogControl & RenderDialogControl::getInstance(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE * inst)
{
    static RenderDialogControl singleton {api, inst};
    return singleton;
}

RenderDialogControl & RenderDialogControl::getInstance()
{
    return getInstance(nullptr, nullptr);
}
}
