#include "exportaction_dialogcontrol.h"

#include <WDL/swell/swell.h>

#include "resource.h"
#include "exportaction_issues.h"
#include "exportaction.h"

// Lots of assumptions made here! May need more re-jigging if future versions differ
#define EXPECTED_RENDER_DIALOG_WINDOW_TITLE "Render to File"
#define EXPECTED_FIRST_SAMPLE_RATE_COMBO_OPTION "8000"
#define EXPECTED_FIRST_CHANNEL_COUNT_COMBO_OPTION "Mono"
#define EXPECTED_PRESETS_BUTTON_TEXT "Presets"
#define REQUIRED_SOURCE_COMBO_OPTION "Master mix"
#define REQUIRED_BOUNDS_COMBO_OPTION "Entire project"

#define TAKEOVER_TEXT_FOR_CHANNEL_COUNT "Auto" // What we will display for "Channels" when our PCM Sink is selected
#define FALLBACK_VALUE_FOR_CHANNEL_COUNT "Stereo" // Value once we restore state, if we didn't know what it was before

#define TIMER_ID 1

int MakeWParam(int loWord, int hiWord)
{
    return (loWord & 0xFFFF) + ((hiWord & 0xFFFF) << 16);
}

RenderDialogControl::RenderDialogState::RenderDialogState(std::shared_ptr<ReaperAPI> api, REAPER_PLUGIN_HINSTANCE *inst) : reaperApi{ api }, reaperInst{ *inst } {
}

RenderDialogControl::RenderDialogState::~RenderDialogState() {
}

RenderDialogControl::RenderDialogState::ControlType RenderDialogControl::RenderDialogState::getControlType(HWND hwnd){

    if (!hwnd || !IsWindow(hwnd)) return UNKNOWN;

#ifdef __APPLE__
    if(SWELL_IsButton(hwnd)) return BUTTON;

    if(SWELL_IsStaticText(hwnd)){
        try {
            SWELL_CB_GetNumItems(hwnd, 0);
            return EDITABLECOMBO;
        } catch(...){
            return TEXT;
        }
    }

    try {
        SWELL_CB_GetNumItems(hwnd, 0);
        return COMBOBOX;
    } catch(...){}
#endif

#ifdef WIN32
    char szClassName[9];
    GetClassName(hwnd, szClassName, 9);
    if(strcmp(szClassName, "Edit") == 0) return TEXT;
    if(strcmp(szClassName, "Button") == 0) return BUTTON;

    if(strcmp(szClassName, "ComboBox") == 0) {
        COMBOBOXINFO info = { sizeof(COMBOBOXINFO) };
        GetComboBoxInfo(hwnd, &info);
        auto editBoxText = getWindowText(info.hwndItem);
        bool editable = SetWindowText(info.hwndItem, "TEST");
        SetWindowText(info.hwndItem, editBoxText.c_str());
        return editable ? EDITABLECOMBO : COMBOBOX;
    }
#endif

    return UNKNOWN;
}

HWND RenderDialogControl::RenderDialogState::getComboBoxEdit(HWND hwnd){
    auto controlType = getControlType(hwnd);
    if(controlType == COMBOBOX) return hwnd;
    if(controlType != EDITABLECOMBO) return HWND();

#ifdef __APPLE__
    return hwnd; //Same thing on OSX - editable bit IS the combobox
#endif

#ifdef WIN32
    COMBOBOXINFO info = { sizeof(COMBOBOXINFO) };
    GetComboBoxInfo(hwnd, &info);
    return info.hwndItem;
#endif

    return HWND();
}

std::string RenderDialogControl::RenderDialogState::getComboBoxItemText(HWND hwnd, int itemIndex)
{
    auto controlType = getControlType(hwnd);
    if(controlType != COMBOBOX && controlType != EDITABLECOMBO) return std::string();
    char itmtxt[100];
#ifdef WIN32
    SendMessage(hwnd, CB_GETLBTEXT, itemIndex, (LPARAM)itmtxt);
#else
    SWELL_CB_GetItemText(hwnd, 0, itemIndex, itmtxt, 100);
#endif
    return std::string(itmtxt);
}

std::string RenderDialogControl::RenderDialogState::getWindowText(HWND hwnd)
{
    char wintxt[100];
    GetWindowText(hwnd, wintxt, 100);
    return std::string(wintxt);
}

HWND RenderDialogControl::RenderDialogState::ShowConfig(const void *cfg, int cfg_l, HWND parent)
{
        const void *x[2] = { cfg,(void *)&cfg_l };
        return CreateDialogParam(reaperInst, MAKEINTRESOURCE(IDD_EXPORT), parent,
            RenderDialogControl::wavecfgDlgProc,
            (LPARAM)x);
}

LRESULT RenderDialogControl::RenderDialogState::selectInComboBox(HWND hwnd, std::string text) {
#ifdef WIN32
    auto lparam = text.c_str();
    // Find item
    LRESULT pos = SendMessage(hwnd, (int)CB_FINDSTRINGEXACT, -1, (LPARAM)lparam);
    if(pos == CB_ERR) return CB_ERR;
    // Select item
    auto res = SendMessage(hwnd, (int)CB_SETCURSEL, (WPARAM)pos, 0);
    if(res == CB_ERR) return CB_ERR;
    // Make sure applications control change callbacks are fired
    auto nID = GetWindowLongPtr(hwnd, -12); // get GWL_ID
    int send_cbn_selchange = MakeWParam(nID, CBN_SELCHANGE);
    // Send the WM_COMMAND to the parent, not the control itself
    return SendMessage(GetParent(hwnd), 0x111, send_cbn_selchange, (LPARAM)hwnd);
#else
    int count = SWELL_CB_GetNumItems(hwnd, 0);
    for(int i = 0; i < count; i++){
        char itmtxt[100];
        SWELL_CB_GetItemText(hwnd, 0, i, itmtxt, 100);
        if(itmtxt == text){
            SWELL_CB_SetCurSel(hwnd, 0, i);
            // Make sure applications control change callbacks are fired
            auto nID = GetWindowLongPtr(hwnd, -12); // get GWL_ID
            int send_cbn_selchange = MakeWParam(nID, CBN_SELCHANGE);
            // Send the WM_COMMAND to the parent, not the control itself
            return SendMessage(GetParent(hwnd), 0x111, send_cbn_selchange, (LPARAM)hwnd);
        }
    }
    return CB_ERR;
#endif
}

void RenderDialogControl::RenderDialogState::startPreparingRenderControls(HWND hwndDlg)
{
    // Our dialog displayed - reset all vars (might not be the first time around)
    boundsControlHwnd.reset();
    sourceControlHwnd.reset();
    presetsControlHwnd.reset();
    sampleRateControlHwnd.reset();
    channelsControlHwnd.reset();
    sampleRateControlSetError = false;
    channelsControlSetError = false;
    sampleRateLastOption.clear();
    channelsLastOption.clear();

    startedPrepareDialogControls = true; // Prevents paint message relauching this
    EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_REFRESH), false);
    SetWindowText(GetDlgItem(hwndDlg, IDC_INFOPANE), LPCSTR("\r\nValidating Project Structure...\r\n\r\nPlease Wait..."));
    SetTimer(hwndDlg,
             TIMER_ID,            // timer identifier
             100,                 // interval(ms)
             (TIMERPROC)NULL);    // no timer callback (use window message)
}

BOOL CALLBACK RenderDialogControl::RenderDialogState::prepareRenderControl_pass1(HWND hwnd, LPARAM lParam) { // Caps BOOL is actually int for EnumChildWindows compatibility
                                                                // Prepare Render Dialog Control for ADM export.
                                                                // This will involve fixing some values and disabling those controls

    // MUST do source combo before bounds combo (it affects options)
    if (hwnd && IsWindow(hwnd))
    {
        if (getControlType(hwnd) == COMBOBOX) {
            // See if this is the 'Source' dropdown by setting it to the option we want - if successful, it was, and so disable it
            if (selectInComboBox(hwnd, REQUIRED_SOURCE_COMBO_OPTION) != CB_ERR) {
                sourceControlHwnd = hwnd;
                EnableWindow(hwnd, false);
            }
        }
    }

    return true; // MUST return true to continue iterating through controls
}

BOOL CALLBACK RenderDialogControl::RenderDialogState::prepareRenderControl_pass2(HWND hwnd, LPARAM lParam) { // Caps BOOL is actually int for EnumChildWindows compatibility
                                                                                                             // Prepare Render Dialog Control for ADM export.
                                                                                                             // This will involve fixing some values and disabling those controls

    if (hwnd && IsWindow(hwnd))
    {
        std::string winStr = getWindowText(hwnd);
        auto controlType = getControlType(hwnd);

        if (controlType == BUTTON) {
            if (winStr == EXPECTED_PRESETS_BUTTON_TEXT){
                // This is the presets button which could be used to override our forced settings - disable it
                presetsControlHwnd = hwnd;
                EnableWindow(hwnd, false);
            }
        }

        if (controlType == COMBOBOX || controlType == EDITABLECOMBO) {
            // NOTE: Sample Rate and Channels controls are;
            //       EDITABLECOMBO in REAPER <=6.11
            //       COMBOBOX in REAPER >=6.12

            auto itemText = getComboBoxItemText(hwnd);
            // See if this is the sample rate dropdown by seeing if the first item is EXPECTED_FIRST_SAMPLE_RATE_COMBO_OPTION
            if(itemText == EXPECTED_FIRST_SAMPLE_RATE_COMBO_OPTION){
                sampleRateControlHwnd = hwnd;
                auto editControl = getComboBoxEdit(hwnd);
                auto currentOption = getWindowText(editControl);
                if(sampleRateLastOption.length() == 0 && currentOption.length() > 0){
                    sampleRateLastOption = currentOption;
                }
                EnableWindow(editControl, false);
                UpdateWindow(editControl);
            }
            // See if this is the channels dropdown by seeing if the first item is EXPECTED_FIRST_CHANNEL_COUNT_COMBO_OPTION
            if(itemText ==  EXPECTED_FIRST_CHANNEL_COUNT_COMBO_OPTION){
                channelsControlHwnd = hwnd;
                auto editControl = getComboBoxEdit(hwnd);
                auto currentOption = getWindowText(editControl);
                if(channelsLastOption.length() == 0 && currentOption.length() > 0){
                    channelsLastOption = currentOption;
                }
                channelsControlSetError |= (SetWindowText(editControl, TAKEOVER_TEXT_FOR_CHANNEL_COUNT) == 0);
                EnableWindow(editControl, false);
                UpdateWindow(editControl);
            }
        }

        if (getControlType(hwnd) == COMBOBOX) {
            // See if this is the 'Bounds' dropdown by setting it to the option we want - if successful, it was, and so disable it
            if (selectInComboBox(hwnd, REQUIRED_BOUNDS_COMBO_OPTION) != CB_ERR){
                boundsControlHwnd = hwnd;
                EnableWindow(hwnd, false);
            }
        }

    }

    return true; // MUST return true to continue iterating through controls
}

std::string RenderDialogControl::RenderDialogState::getAdmExportVstsInfoString() {
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

WDL_DLGRET RenderDialogControl::RenderDialogState::wavecfgDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {

    case WM_INITDIALOG:
    {
        // this is called when the dialog is initialized
        startedPrepareDialogControls = false;
        return 0;
    }

    case WM_PAINT:
    {
        // Have to do our control takeover in paint - on init, no controls are ready to take over yet!
        if(!startedPrepareDialogControls) {
            // Doesn't like UpdateWindow being called in paint... set up a timer to run dialog configuration routine.
            startPreparingRenderControls(hwndDlg);
        }

        return 0;
    }

    case WM_TIMER:
    {
        KillTimer(hwndDlg, TIMER_ID);

        HWND renderDialogHandle = GetParent(hwndDlg); // Need a handle to the parent dialog of the window area we're given (i.e, the render dialog itself)
        std::string windowTitle = getWindowText(renderDialogHandle);

        // May need to escape multiple times if later versions further encapsulate our area (REAPER v6.05+ certainly does!)
        constexpr int maxEscapeDepth = 3;
        int currentEscapeDepth = 0;
        while(windowTitle != EXPECTED_RENDER_DIALOG_WINDOW_TITLE) {
            currentEscapeDepth++;
            assert(currentEscapeDepth < maxEscapeDepth);
            if(currentEscapeDepth >= maxEscapeDepth) break;
            renderDialogHandle = GetParent(renderDialogHandle); // Need a handle to the parent dialog of the window area we're given (i.e, the render dialog itself)
            windowTitle = getWindowText(renderDialogHandle);
        }

        sampleRateControlSetError = false;
        channelsControlSetError = false;
        EnumChildWindows(renderDialogHandle, RenderDialogControl::callback_PrepareRenderControl_pass1, 0);
        EnumChildWindows(renderDialogHandle, RenderDialogControl::callback_PrepareRenderControl_pass2, 0);
        UpdateWindow(renderDialogHandle);
        // By this point we should have handles to all controls we're taking over

        admExportHandler = std::make_shared<AdmExportHandler>();
        admExportHandler->repopulate(*reaperApi);
        if(sampleRateControlHwnd.has_value()) {
            int sRate = 0;
            if(admExportHandler->getAdmExportSources()) {
                sRate = admExportHandler->getAdmExportSources()->getSampleRate();
            }
            if(sRate > 0) {
                auto sr = std::to_string(sRate);
                auto comboControl = *sampleRateControlHwnd;
                sampleRateControlSetError |= (selectInComboBox(comboControl, sr) == CB_ERR);
                auto editControl = getComboBoxEdit(comboControl);
                sampleRateControlSetError |= (SetWindowText(editControl, sr.c_str()) == 0);
                UpdateWindow(editControl);
            }
        }

        auto infoPaneText = getAdmExportVstsInfoString();
        bool allControlsSuccessful = boundsControlHwnd.has_value() &&
            sourceControlHwnd.has_value() &&
            presetsControlHwnd.has_value() &&
            sampleRateControlHwnd.has_value() &&
            channelsControlHwnd.has_value() &&
            !sampleRateControlSetError && !channelsControlSetError;
        if(!allControlsSuccessful) {
            infoPaneText = "WARNING: Unable to takeover all render controls. REAPER version may be unsupported and render may fail.\r\n\r\n" + infoPaneText;
        }

        SetWindowText(GetDlgItem(hwndDlg, IDC_INFOPANE), LPCSTR(infoPaneText.c_str()));
        EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_REFRESH), true);
        UpdateWindow(hwndDlg);
        admExportHandler.reset(); // We MUST reset this, otherwise we keep NNG ports open which the PCM sink will want to use if we render
        return 0;
    }

    case WM_COMMAND:
    {
        // this gets called in case something changed

        // Unsigned shorts
        auto controlId = LOWORD(wParam);
        auto notificationType = HIWORD(wParam);

        if (notificationType == BN_CLICKED && controlId == IDC_BUTTON_REFRESH) {
            // Repopulate ADM info
            startPreparingRenderControls(hwndDlg);
        }
        return 0;
    }

    case WM_USER + 1024:
    {
        // this gets called to retrieve the settings!

        if (wParam) *((int *)wParam) = 32;
        if (lParam)
        {
            ((int *)lParam)[0] = ExportManager::ExportInfo.SINK_FOURCC;
            ((int *)(((unsigned char *)lParam) + 4))[0] = REAPER_MAKELEINT(0);
            ((float *)(((unsigned char *)lParam) + 4))[1] = 0.f;

        }
        return 0;
    }

    case WM_DESTROY:
    {
        // Reenable the controls we disabled.
        if(boundsControlHwnd) EnableWindow(*boundsControlHwnd, true);
        if(presetsControlHwnd) EnableWindow(*presetsControlHwnd, true);
        if(sourceControlHwnd) EnableWindow(*sourceControlHwnd, true);

        // NOTE: Sample Rate and Channels controls are;
        //       EDITABLECOMBO in REAPER <=6.11
        //       COMBOBOX in REAPER >=6.12

        if(sampleRateControlHwnd) {
            auto editControl = getComboBoxEdit(*sampleRateControlHwnd);
            if(sampleRateLastOption.length() > 0) {
                selectInComboBox(*sampleRateControlHwnd, sampleRateLastOption);
                SetWindowText(editControl, sampleRateLastOption.c_str());
            }
            EnableWindow(editControl, true);
            UpdateWindow(editControl);
        }
        if(channelsControlHwnd) {
            auto editControl = getComboBoxEdit(*channelsControlHwnd);
            if(channelsLastOption.length() > 0){
                std::string opt = channelsLastOption == TAKEOVER_TEXT_FOR_CHANNEL_COUNT ? FALLBACK_VALUE_FOR_CHANNEL_COUNT : channelsLastOption;
                selectInComboBox(*channelsControlHwnd, opt);
                SetWindowText(editControl, opt.c_str());
            }
            EnableWindow(editControl, true);
            UpdateWindow(editControl);
        }

        return 0;
    }

    }
    return 0;
}
