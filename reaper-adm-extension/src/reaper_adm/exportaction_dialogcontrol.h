#pragma once

#include <memory>
#include <map>
#include <optional>

#include <WDL/wdltypes.h>

#include "reaperapi.h"
#include "reaper_plugin.h"
#include "exportaction_admsourcescontainer.h"

using namespace admplug;

class RenderDialogControl {
public:
    RenderDialogControl(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst) : stateSingleton{getInstance(api, inst)} {}
    ~RenderDialogControl() {}

    // Static callback methods which forward to the RenderDialogState instance (singleton)
    // Means we don't need to worry about binding methods.

    static WDL_DLGRET wavecfgDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        return RenderDialogControl::getInstance()->wavecfgDlgProc(hwndDlg, uMsg, wParam, lParam);
    }

    static HWND ShowConfig(const void *cfg, int cfg_l, HWND parent) {
        return RenderDialogControl::getInstance()->ShowConfig(cfg, cfg_l,parent);
    }

    static BOOL CALLBACK callback_PrepareRenderControl_pass1(HWND hwnd, LPARAM lParam) {
        return RenderDialogControl::getInstance()->prepareRenderControl_pass1(hwnd, lParam);
    }

    static BOOL CALLBACK callback_PrepareRenderControl_pass2(HWND hwnd, LPARAM lParam) {
        return RenderDialogControl::getInstance()->prepareRenderControl_pass2(hwnd, lParam);
    }

private:
    class RenderDialogState {
    public:
        RenderDialogState(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst);
        ~RenderDialogState();

        HWND ShowConfig(const void *cfg, int cfg_l, HWND parent);

    private:
        friend class RenderDialogControl;

        enum ControlType{
            UNKNOWN,
            TEXT, //Note: On OSX this can be editable text or a label. On windows, it is editable text.
            BUTTON, //Note: On OSX this includes radios and checkboxes.
            COMBOBOX,
            EDITABLECOMBO
        };

        ControlType getControlType(HWND hwnd);
        HWND getComboBoxEdit(HWND hwnd);
        std::string getComboBoxItemText(HWND hwnd, int idx = 0);
        std::string getWindowText(HWND hwnd);
        LRESULT selectInComboBox(HWND hwnd, std::string text);

        std::shared_ptr<ReaperAPI> reaperApi;
        REAPER_PLUGIN_HINSTANCE reaperInst;

        std::optional<HWND> boundsControlHwnd{};
        std::optional<HWND> sourceControlHwnd{};
        std::optional<HWND> presetsControlHwnd{};
        std::optional<HWND> sampleRateControlHwnd{};
        std::optional<HWND> channelsControlHwnd{};
        bool sampleRateControlSetError{false};
        bool channelsControlSetError{false};

        std::string sampleRateLastOption{};
        std::string channelsLastOption{};

        std::shared_ptr<AdmExportHandler> admExportHandler;

        void startPreparingRenderControls(HWND hwndDlg);
        BOOL CALLBACK prepareRenderControl_pass1(HWND hwnd, LPARAM lParam);
        BOOL CALLBACK prepareRenderControl_pass2(HWND hwnd, LPARAM lParam);
        std::string getAdmExportVstsInfoString();
        WDL_DLGRET wavecfgDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

        bool startedPrepareDialogControls{ false };
    };

    std::shared_ptr<RenderDialogState> stateSingleton;
    inline static std::weak_ptr<RenderDialogState> stateSingletonStatic;

public:
    static std::shared_ptr<RenderDialogState> getInstance(std::shared_ptr<ReaperAPI> api = nullptr, REAPER_PLUGIN_HINSTANCE *inst  = nullptr) {
        if(std::shared_ptr<RenderDialogState> singleton = stateSingletonStatic.lock()) {
            return singleton;
        } else {
            if (api == nullptr || inst == nullptr) {
                throw std::logic_error("Expected api and inst handles passed to getInstance on first instance!");
            }
            singleton = std::make_shared<RenderDialogState>(api, inst);
            stateSingletonStatic = singleton;
            return singleton;
        }
    }

};
