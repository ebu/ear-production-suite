#pragma once

#include <memory>
#include <map>
#include <optional>

#include <WDL/wdltypes.h>

#include "reaperapi.h"
#include "reaper_plugin.h"
#include "exportaction_admsourcescontainer.h"

using namespace admplug;

class RenderDialogControl;

class RenderDialogState {
public:
    RenderDialogState(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst);
    ~RenderDialogState();

    HWND ShowConfig(const void *cfg, int cfg_l, HWND parent);

private:
    friend class RenderDialogControl;

    enum ControlType{
        UNKNOWN,
        TEXT, //Note: On OSX this can be editable text or a label. Mirrored this behaviour for Windows.
        BUTTON, //Note: This includes radios and checkboxes.
        COMBOBOX,
        EDITABLECOMBO
    };

    ControlType getControlType(HWND hwnd);
    HWND getComboBoxEdit(HWND hwnd);
    std::string getComboBoxItemText(HWND hwnd, int idx = 0);
    std::string getWindowText(HWND hwnd);
    LRESULT selectInComboBox(HWND hwnd, std::string text);
    bool getCheckboxState(HWND hwnd);
    void setCheckboxState(HWND hwnd, bool state);

    std::shared_ptr<ReaperAPI> reaperApi;
    REAPER_PLUGIN_HINSTANCE reaperInst;

    std::optional<HWND> boundsControlHwnd{};
    std::optional<HWND> sourceControlHwnd{};
    std::optional<HWND> presetsControlHwnd{};
    std::optional<HWND> sampleRateControlHwnd{};
    std::optional<HWND> channelsControlHwnd{};
    std::optional<HWND> channelsLabelHwnd{};
    std::optional<HWND> secondPassControlHwnd{};
    std::optional<HWND> normalizeControlHwnd{};
    std::optional<HWND> resampleModeControlHwnd{};
    std::optional<HWND> monoToMonoControlHwnd{};
    std::optional<HWND> multiToMultiControlHwnd{};
    bool sampleRateControlSetError{false};
    bool channelsControlSetError{false};

    std::string sampleRateLastOption{};
    std::string channelsLastOption{};
    bool secondPassLastState{ false };
    bool monoToMonoLastState{ false };
    bool multiToMultiLastState{ false };

    std::shared_ptr<AdmExportHandler> admExportHandler;

    void startPreparingRenderControls(HWND hwndDlg);
    BOOL CALLBACK prepareRenderControl_pass1(HWND hwnd, LPARAM lParam);
    BOOL CALLBACK prepareRenderControl_pass2(HWND hwnd, LPARAM lParam);
    std::string getAdmExportVstsInfoString();
    WDL_DLGRET wavecfgDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

    bool startedPrepareDialogControls{ false };

    std::string EXPECTED_RENDER_DIALOG_WINDOW_TITLE{ "Render to File" };
    std::string EXPECTED_FIRST_SAMPLE_RATE_COMBO_OPTION{ "8000" };
    std::string EXPECTED_FIRST_CHANNEL_COUNT_COMBO_OPTION{ "Mono" };
    std::string EXPECTED_PRESETS_BUTTON_TEXT{ "Presets" };
    std::string EXPECTED_NORMALIZE_BUTTON_TEXT{ "Normalize/Limit..." };
    std::string EXPECTED_SECOND_PASS_CHECKBOX_TEXT{ "2nd pass render" };
    std::string EXPECTED_MONO2MONO_CHECKBOX_TEXT{ "Tracks with only mono media to mono files" };
    std::string EXPECTED_MULTI2MULTI_CHECKBOX_TEXT{ "Multichannel tracks to multichannel files" };
    std::string REQUIRED_SOURCE_COMBO_OPTION{ "Master mix" };
    std::string REQUIRED_BOUNDS_COMBO_OPTION{ "Entire project" };
    std::string EXPECTED_CHANNEL_COUNT_LABEL_TEXT{ "Channels:" };
    std::string REQUIRED_CHANNEL_COUNT_COMBO_OPTION{ "Mono" };
    std::string EXPECTED_FIRST_RESAMPLE_MODE_COMBO_OPTION{ "Point Sampling (lowest quality, retro)" };

};

class RenderDialogControl {
public:
    static RenderDialogControl& getInstance(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst);
    static RenderDialogControl& getInstance();

    // Static callback methods which forward to the RenderDialogState instance (singleton)
    // Means we don't need to worry about binding methods.

    static WDL_DLGRET wavecfgDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        return getInstance().state.wavecfgDlgProc(hwndDlg, uMsg, wParam, lParam);
    }

    static HWND ShowConfig(const void *cfg, int cfg_l, HWND parent) {
        return getInstance().state.ShowConfig(cfg, cfg_l,parent);
    }

    static BOOL CALLBACK callback_PrepareRenderControl_pass1(HWND hwnd, LPARAM lParam) {
        return getInstance().state.prepareRenderControl_pass1(hwnd, lParam);
    }

    static BOOL CALLBACK callback_PrepareRenderControl_pass2(HWND hwnd, LPARAM lParam) {
        return getInstance().state.prepareRenderControl_pass2(hwnd, lParam);
    }

private:
    RenderDialogControl(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst) : state{api, inst} {}
    RenderDialogState state;
};
