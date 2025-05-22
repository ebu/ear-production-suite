#include "render_dialog_modifier.hpp"
#include "../reaperapi.h"
#include "../resource.h"
#include "helper/char_encoding.hpp"

#include <WDL/swell/swell.h>
#include <WDL/wdltypes.h>
#include <cassert>
#include <exportaction_admsourcescontainer.h>

namespace admplug {
namespace {

struct CallbackData {
  RenderDialogModifier* dialogModifier;
  std::shared_ptr<AdmExportHandler> admExportHandler;
};

constexpr int TIMER_ID = 1;
std::string EXPECTED_RENDER_DIALOG_WINDOW_TITLE{ "Render to File" };
std::string EXPECTED_FIRST_SAMPLE_RATE_COMBO_OPTION{ "8000" };
std::string EXPECTED_FIRST_CHANNEL_COUNT_COMBO_OPTION{ "Mono" };
std::string EXPECTED_PRESETS_BUTTON_TEXT{ "Presets" };
std::string EXPECTED_PRESERVE_SAMPLE_RATE_CHECKBOX_TEXT{ "Preserve source media sample rate if possible" };
constexpr int EXPECTED_NORMALIZE_BUTTON_ID{ 1067 };
constexpr int EXPECTED_NORMALIZE_CHECKBOX_ID{ 1069 };
std::string EXPECTED_SECOND_PASS_CHECKBOX_TEXT{ "2nd pass render" };
std::string EXPECTED_MONO2MONO_CHECKBOX_TEXT{ "Tracks with only mono media to mono files" };
std::string EXPECTED_MULTI2MULTI_CHECKBOX_TEXT{ "Multichannel tracks to multichannel files" };
std::string REQUIRED_SOURCE_COMBO_OPTION{ "Master mix" };
std::string REQUIRED_BOUNDS_COMBO_OPTION{ "Entire project" };
std::string EXPECTED_CHANNEL_COUNT_LABEL_TEXT{ "Channels:" };
std::string REQUIRED_CHANNEL_COUNT_COMBO_OPTION{ "Mono" };
std::string EXPECTED_FIRST_RESAMPLE_MODE_COMBO_OPTION{ "Point Sampling (lowest quality, retro)" };
constexpr int EXPECTED_RESAMPLE_MODE_COMBO_ID{ 1000 };

int MakeWParam(int loWord, int hiWord)
{
  return (loWord & 0xFFFF) + ((hiWord & 0xFFFF) << 16);
}

void localise(std::string& str, std::shared_ptr<ReaperAPI> api) {
  if(std::stof(api->GetAppVersion()) >= 6.11f) {
    auto loc = api->LocalizeString(str.c_str(), "render", 0);
    std::string localised{ api->LocalizeString(str.c_str(),"render", 0) };
    if(!localised.empty() && localised != str) {
      str = localised;
      return;
    }
    localised = api->LocalizeString(str.c_str(), "DLG_506", 0);
    if(!localised.empty() && localised != str) {
      str = localised;
      return;
    }
    // Note that the above sections fall-through to common anyway, but for completeness...
    localised = api->LocalizeString(str.c_str(), "common", 0);
    if(!localised.empty() && localised != str) {
      str = localised;
      return;
    }
  }
}

BOOL CALLBACK pass1_callback(HWND hwnd, LPARAM lParam) {
  if (lParam) {
    auto data = reinterpret_cast<CallbackData*>(lParam);
    return data->dialogModifier->prepareRenderControl_pass1(hwnd, data->admExportHandler);
  }
  return false;
}

BOOL CALLBACK pass2_callback(HWND hwnd, LPARAM lParam) {
  if (lParam) {
    auto modifier = reinterpret_cast<RenderDialogModifier*>(lParam);
    return modifier->prepareRenderControl_pass2(hwnd, lParam);
  }
  return false;
}

}

RenderDialogModifier::Control
RenderDialogModifier::getControlType(HWND hwnd) {
  if (!hwnd || !IsWindow(hwnd))
    return Control::UNKNOWN;

#ifdef WIN32
  char szClassName[9];
  GetClassName(hwnd, szClassName, 9);
  if (strcmp(szClassName, "Edit") == 0)
    return TEXT;
  if (strcmp(szClassName, "Static") == 0)
    return TEXT;
  if (strcmp(szClassName, "Button") == 0)
    return BUTTON;

  if (strcmp(szClassName, "ComboBox") == 0) {
    COMBOBOXINFO info = {sizeof(COMBOBOXINFO)};
    GetComboBoxInfo(hwnd, &info);
    auto editBoxText = getWindowText(info.hwndItem);
    bool editable = SetWindowText(info.hwndItem, "TEST");
    SetWindowText(info.hwndItem, editBoxText.c_str());
    return editable ? EDITABLECOMBO : COMBOBOX;
  }
#else
  if (SWELL_IsButton(hwnd))
    return Control::BUTTON;

  if (SWELL_IsStaticText(hwnd)) {
    try {
      SWELL_CB_GetNumItems(hwnd, 0);
      return Control::EDITABLECOMBO;
    } catch (...) {
      return Control::TEXT;
    }
  }

  try {
    SWELL_CB_GetNumItems(hwnd, 0);
    return Control::COMBOBOX;
  } catch (...) {
  }
#endif
  return Control::UNKNOWN;
}

HWND RenderDialogModifier::getComboBoxEdit(HWND hwnd) {
  auto controlType = getControlType(hwnd);
  if (controlType == Control::COMBOBOX)
    return hwnd;
  if (controlType != Control::EDITABLECOMBO)
    return HWND();

#ifdef WIN32
  COMBOBOXINFO info = {sizeof(COMBOBOXINFO)};
  GetComboBoxInfo(hwnd, &info);
  return info.hwndItem;
#else
  return hwnd; // Same thing on OSX - editable bit IS the combobox
#endif
  return HWND();
}

std::string RenderDialogModifier::getComboBoxItemText(HWND hwnd, int idx) {
  {
    auto controlType = getControlType(hwnd);
    if (controlType != Control::COMBOBOX && controlType != Control::EDITABLECOMBO)
      return std::string();
    char itmtxt[100];
#ifdef WIN32
    SendMessage(hwnd, CB_GETLBTEXT, itemIndex, (LPARAM)itmtxt);
#else
    SWELL_CB_GetItemText(hwnd, 0, idx, itmtxt, 100);
#endif
    return std::string(itmtxt);
  }
}

std::string RenderDialogModifier::getWindowText(HWND hwnd) {
#ifdef WIN32
  wchar_t wintxt[100];
  GetWindowTextW(hwnd, wintxt, 100);
#else
  char wintxt[100];
  GetWindowText(hwnd, wintxt, 100);
#endif
  return bufferToString(wintxt);
}

LRESULT RenderDialogModifier::selectInComboBox(HWND hwnd, std::string text) {
#ifdef WIN32
  auto lparam = text.c_str();
  // Find item
  LRESULT pos = SendMessage(hwnd, (int)CB_FINDSTRINGEXACT, -1, (LPARAM)lparam);
  if (pos == CB_ERR)
    return CB_ERR;
  // Select item
  auto res = SendMessage(hwnd, (int)CB_SETCURSEL, (WPARAM)pos, 0);
  if (res == CB_ERR)
    return CB_ERR;
  // Make sure applications control change callbacks are fired
  auto nID = GetWindowLongPtr(hwnd, -12); // get GWL_ID
  int send_cbn_selchange = MakeWParam(nID, CBN_SELCHANGE);
  // Send the WM_COMMAND to the parent, not the control itself
  return SendMessage(GetParent(hwnd), 0x111, send_cbn_selchange, (LPARAM)hwnd);
#else
  int count = SWELL_CB_GetNumItems(hwnd, 0);
  for (int i = 0; i < count; i++) {
    char itmtxt[100];
    SWELL_CB_GetItemText(hwnd, 0, i, itmtxt, 100);
    if (itmtxt == text) {
      SWELL_CB_SetCurSel(hwnd, 0, i);
      // Make sure applications control change callbacks are fired
      auto nID = GetWindowLongPtr(hwnd, GWL_ID); // get GWL_ID
      int send_cbn_selchange = MakeWParam(nID, CBN_SELCHANGE);
      // Send the WM_COMMAND to the parent, not the control itself
      return SendMessage(GetParent(hwnd), WM_COMMAND, send_cbn_selchange,
                         (LPARAM)hwnd);
    }
  }
  return CB_ERR;
#endif
}

bool RenderDialogModifier::getCheckboxState(HWND hwnd) {
  return SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void RenderDialogModifier::setCheckboxState(HWND hwnd, bool state) {
  auto currentState = getCheckboxState(hwnd);
  if (state == currentState)
    return;
  // Have to simulate a click in case it has a listener attached to it
  auto parentHwnd = GetParent(hwnd);
  auto controlId = GetWindowLong(hwnd, GWL_ID);
  SendMessage(hwnd, BM_SETCHECK, state ? BST_CHECKED : BST_UNCHECKED, 0);
  SendMessage(parentHwnd, WM_COMMAND, MAKELONG(controlId, BN_CLICKED),
              (LPARAM)hwnd);
}

void RenderDialogModifier::startPreparingRenderControls(HWND hwndDlg) {
  localise(EXPECTED_RENDER_DIALOG_WINDOW_TITLE, reaperApi);
  localise(EXPECTED_FIRST_SAMPLE_RATE_COMBO_OPTION, reaperApi);
  localise(EXPECTED_FIRST_CHANNEL_COUNT_COMBO_OPTION, reaperApi);
  localise(EXPECTED_PRESETS_BUTTON_TEXT, reaperApi);
  localise(EXPECTED_PRESERVE_SAMPLE_RATE_CHECKBOX_TEXT, reaperApi);
  localise(EXPECTED_SECOND_PASS_CHECKBOX_TEXT, reaperApi);
  localise(EXPECTED_MONO2MONO_CHECKBOX_TEXT, reaperApi);
  localise(EXPECTED_MULTI2MULTI_CHECKBOX_TEXT, reaperApi);
  localise(REQUIRED_SOURCE_COMBO_OPTION, reaperApi);
  localise(REQUIRED_BOUNDS_COMBO_OPTION, reaperApi);
  localise(EXPECTED_CHANNEL_COUNT_LABEL_TEXT, reaperApi);
  localise(REQUIRED_CHANNEL_COUNT_COMBO_OPTION, reaperApi);
  localise(EXPECTED_FIRST_RESAMPLE_MODE_COMBO_OPTION, reaperApi);

  // Our dialog displayed - reset all vars (might not be the first time around)
  controls.boundsControlHwnd.reset();
  controls.sourceControlHwnd.reset();
  controls.presetsControlHwnd.reset();
  controls.normalizeCheckboxControlHwnd.reset();
  controls.normalizeControlHwnd.reset();
  controls.secondPassControlHwnd.reset();
  controls.monoToMonoControlHwnd.reset();
  controls.multiToMultiControlHwnd.reset();
  controls.preserveSampleRateControlHwnd.reset();
  controls.sampleRateControlHwnd.reset();
  controls.channelsControlHwnd.reset();
  controls.channelsLabelHwnd.reset();
  controls.resampleModeControlHwnd.reset();
  controls.sampleRateControlSetError = false;
  controls.channelsControlSetError = false;
  controls.normalisationOptionsEnabled = false;
  controls.sampleRateLastOption.clear();
  controls.channelsLastOption.clear();

  startedPrepareDialogControls = true; // Prevents paint message relauching this
  EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_REFRESH), false);
  SetWindowText(
      GetDlgItem(hwndDlg, IDC_INFOPANE),
      LPCSTR("\r\nValidating Project Structure...\r\n\r\nPlease Wait..."));
  SetTimer(hwndDlg,
           TIMER_ID,         // timer identifier
           100,              // interval(ms)
           (TIMERPROC)NULL); // no timer callback (use window message)
}

std::string RenderDialogModifier::takeOverRenderControls(
    HWND hwndDlg, std::shared_ptr<AdmExportHandler> const& admExportHandler) {
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

        controls.sampleRateControlSetError = false;
        controls.channelsControlSetError = false;

        CallbackData data{this, admExportHandler};
        EnumChildWindows(renderDialogHandle, pass1_callback, reinterpret_cast<LPARAM>(&data));
        EnumChildWindows(renderDialogHandle, pass2_callback, reinterpret_cast<LPARAM>(this));
        UpdateWindow(renderDialogHandle);
        // By this point we should have handles to all controls we're taking over

        // Note that some controls are not checked as they are not present in all versions of REAPER anyway, so it's OK for them to be missing
        bool allControlsSuccessful = controls.boundsControlHwnd.has_value() &&
            controls.sourceControlHwnd.has_value() &&
            controls.presetsControlHwnd.has_value() &&
            controls.sampleRateControlHwnd.has_value() &&
            controls.channelsControlHwnd.has_value() &&
            controls.channelsLabelHwnd.has_value() &&
            !controls.sampleRateControlSetError && !controls.channelsControlSetError;
        std::string warnings = "";
        if(!allControlsSuccessful) {
            warnings.append("WARNING: Unable to takeover all render controls. REAPER version may be unsupported and render may fail.\r\n\r\n");
        }
        if (controls.normalisationOptionsEnabled) {
            warnings.append("WARNING: Built-in normalisation is not compatible with object-based audio. Render will fail. Please disable any normalisation/limiting/fade.\r\n\r\n");
        }
        return warnings;
}

BOOL RenderDialogModifier::prepareRenderControl_pass1(
    HWND hwnd, std::shared_ptr<AdmExportHandler> const& admExportHandler) {
  // Caps BOOL is actually int for EnumChildWindows compatibility
  // Prepare Render Dialog Control for ADM export.
  // This will involve fixing some values and disabling those controls

  // MUST do source combo before bounds combo (it affects options)
  // MUST also do channel count in first pass , as changing that re-enables the
  // bounds checkbox!
  if (hwnd && IsWindow(hwnd)) {
    auto controlType = getControlType(hwnd);

    if (controlType == Control::COMBOBOX) {
      // See if this is the 'Source' dropdown by setting it to the option we
      // want - if successful, it was, and so disable it
      if (selectInComboBox(hwnd, REQUIRED_SOURCE_COMBO_OPTION) != CB_ERR) {
        controls.sourceControlHwnd = hwnd;
        EnableWindow(hwnd, false);
      }
    }

    if (controlType == Control::COMBOBOX ||
        controlType == Control::EDITABLECOMBO) {
      // NOTE: Sample Rate and Channels controls are;
      //       EDITABLECOMBO in REAPER <=6.11
      //       COMBOBOX in REAPER >=6.12

      auto itemText = getComboBoxItemText(hwnd);
      // See if this is the sample rate dropdown by seeing if the first item is
      // EXPECTED_FIRST_SAMPLE_RATE_COMBO_OPTION
      if (itemText == EXPECTED_FIRST_SAMPLE_RATE_COMBO_OPTION) {
        controls.sampleRateControlHwnd = hwnd;
        auto editControl = getComboBoxEdit(hwnd);
        auto currentOption = getWindowText(editControl);
        if (controls.sampleRateLastOption.length() == 0 &&
            currentOption.length() > 0) {
          controls.sampleRateLastOption = currentOption;
        }
        if (admExportHandler && admExportHandler->getAdmExportSources()) {
          int sRate = admExportHandler->getAdmExportSources()->getSampleRate();
          if (sRate > 0) {
            auto sr = std::to_string(sRate);
            auto comboControl = *controls.sampleRateControlHwnd;
            controls.sampleRateControlSetError |=
                (selectInComboBox(comboControl, sr) == CB_ERR);
            auto editControl = getComboBoxEdit(comboControl);
            controls.sampleRateControlSetError |=
                (SetWindowText(editControl, sr.c_str()) == 0);
            UpdateWindow(editControl);
          }
        }
        EnableWindow(editControl, false);
        UpdateWindow(editControl);
      }
      // See if this is the channels dropdown by seeing if the first item is
      // EXPECTED_FIRST_CHANNEL_COUNT_COMBO_OPTION
      if (itemText == EXPECTED_FIRST_CHANNEL_COUNT_COMBO_OPTION) {
        controls.channelsControlHwnd = hwnd;
        auto editControl = getComboBoxEdit(hwnd);
        auto currentOption = getWindowText(editControl);
        if (controls.channelsLastOption.length() == 0 &&
            currentOption.length() > 0) {
          controls.channelsLastOption = currentOption;
        }
        controls.channelsControlSetError |=
            (selectInComboBox(hwnd, REQUIRED_CHANNEL_COUNT_COMBO_OPTION) ==
             CB_ERR);
        UpdateWindow(editControl);
        ShowWindow(hwnd, SW_HIDE);
      }
    }

    if (controlType == Control::BUTTON) {
      // Normalise checkbox has no text, and order of control enumeration may
      // not be reliable. Lets check ID. Do this in first pass as we don't want
      // to disable the normalisation button if the checkbox isn't present.
      if (GetWindowLong(hwnd, GWL_ID) == EXPECTED_NORMALIZE_CHECKBOX_ID) {
        assert(getWindowText(hwnd) == "");
        controls.normalizeCheckboxControlHwnd = hwnd;
        controls.normalizeCheckboxLastState = getCheckboxState(hwnd);
        setCheckboxState(hwnd, false);
        EnableWindow(hwnd, false);
      }
    }
  }

  return true; // MUST return true to continue iterating through controls
}
BOOL RenderDialogModifier::prepareRenderControl_pass2(HWND hwnd,
                                                      LPARAM lParam) {
  Control controlType = Control::UNKNOWN;

  if (hwnd && IsWindow(hwnd)) {
    controlType = getControlType(hwnd);
    std::string winStr = getWindowText(hwnd);
    auto id = GetWindowLong(hwnd, GWL_ID);

    if (controlType == Control::BUTTON) {
      if (winStr == EXPECTED_PRESETS_BUTTON_TEXT) {
        // This is the presets button which could be used to override our forced
        // settings - disable it
        controls.presetsControlHwnd = hwnd;
        EnableWindow(hwnd, false);
      }
      if (winStr == EXPECTED_SECOND_PASS_CHECKBOX_TEXT) {
        // 2nd pass render causes a mismatch between expected number of received
        // block and actual number of received blocks (double) Could probably be
        // recified, but disable as quick fix for now
        controls.secondPassControlHwnd = hwnd;
        controls.secondPassLastState = getCheckboxState(hwnd);
        setCheckboxState(hwnd, false);
        EnableWindow(hwnd, false);
      }
      if (winStr == EXPECTED_MONO2MONO_CHECKBOX_TEXT) {
        controls.monoToMonoControlHwnd = hwnd;
        controls.monoToMonoLastState = getCheckboxState(hwnd);
        setCheckboxState(hwnd, false);
        EnableWindow(hwnd, false);
      }
      if (winStr == EXPECTED_MULTI2MULTI_CHECKBOX_TEXT) {
        controls.multiToMultiControlHwnd = hwnd;
        controls.multiToMultiLastState = getCheckboxState(hwnd);
        setCheckboxState(hwnd, false);
        EnableWindow(hwnd, false);
      }
      if (winStr == EXPECTED_PRESERVE_SAMPLE_RATE_CHECKBOX_TEXT) {
        controls.preserveSampleRateControlHwnd = hwnd;
        controls.preserveSampleRateLastState = getCheckboxState(hwnd);
        setCheckboxState(hwnd, false);
        EnableWindow(hwnd, false);
      }
      // Normalise button text changes depending on what options are enabled.
      // Lets check ID instead.
      if (id == EXPECTED_NORMALIZE_BUTTON_ID) {
        // We only disable the button if there isn't a checkbox (otherwise the
        // checkbox handles state and disables normalisation), and if no
        // normalisation is set, otherwise we have to leave so the user can
        // switch normalisation off.
        if (!controls.normalizeCheckboxControlHwnd.has_value()) {
          controls.normalisationOptionsEnabled = winStr.ends_with(" ON");
          if (!controls.normalisationOptionsEnabled) {
            controls.normalizeControlHwnd = hwnd;
            EnableWindow(hwnd, false);
          }
        }
      }
    }

    if (controlType == Control::TEXT || controlType == Control::EDITABLECOMBO) {
      if (winStr == EXPECTED_CHANNEL_COUNT_LABEL_TEXT) {
        // This is the label for the channel count combobox - hide it
        controls.channelsLabelHwnd = hwnd;
        ShowWindow(hwnd, SW_HIDE);
      }
    }

    if (controlType == Control::COMBOBOX ||
        controlType == Control::EDITABLECOMBO) {
      auto itemText = getComboBoxItemText(hwnd);
      // See if this is the resample mode dropdown by seeing if the first item
      // is EXPECTED_FIRST_RESAMPLE_MODE_COMBO_OPTION Not totally reliable as
      // these options change, so use ID too
      if (id == EXPECTED_RESAMPLE_MODE_COMBO_ID ||
          itemText == EXPECTED_FIRST_RESAMPLE_MODE_COMBO_OPTION) {
        controls.resampleModeControlHwnd = hwnd;
        auto editControl = getComboBoxEdit(hwnd);
        EnableWindow(editControl, false);
        UpdateWindow(editControl);
      }
    }

    if (controlType == Control::COMBOBOX) {
      // See if this is the 'Bounds' dropdown by setting it to the option we
      // want - if successful, it was, and so disable it
      if (selectInComboBox(hwnd, REQUIRED_BOUNDS_COMBO_OPTION) != CB_ERR) {
        controls.boundsControlHwnd = hwnd;
        EnableWindow(hwnd, false);
      }
    }
  }

  return true; // MUST return true to continue iterating through controls
}
void RenderDialogModifier::restoreControls() {
        // Reenable the controls we disabled.
        if(controls.boundsControlHwnd) EnableWindow(*controls.boundsControlHwnd, true);
        if(controls.presetsControlHwnd) EnableWindow(*controls.presetsControlHwnd, true);
        if(controls.sourceControlHwnd) EnableWindow(*controls.sourceControlHwnd, true);
        if (controls.normalizeControlHwnd) EnableWindow(*controls.normalizeControlHwnd, true);

        if(controls.secondPassControlHwnd) {
            EnableWindow(*controls.secondPassControlHwnd, true);
            setCheckboxState(*controls.secondPassControlHwnd, controls.secondPassLastState);
        }
        if(controls.monoToMonoControlHwnd) {
            EnableWindow(*controls.monoToMonoControlHwnd, true);
            setCheckboxState(*controls.monoToMonoControlHwnd, controls.monoToMonoLastState);
        }
        if(controls.multiToMultiControlHwnd) {
            EnableWindow(*controls.multiToMultiControlHwnd, true);
            setCheckboxState(*controls.multiToMultiControlHwnd, controls.multiToMultiLastState);
        }
        if (controls.preserveSampleRateControlHwnd) {
          EnableWindow(*controls.preserveSampleRateControlHwnd, true);
          setCheckboxState(*controls.preserveSampleRateControlHwnd, controls.preserveSampleRateLastState);
        }
        if (controls.normalizeCheckboxControlHwnd) {
          EnableWindow(*controls.normalizeCheckboxControlHwnd, true);
          setCheckboxState(*controls.normalizeCheckboxControlHwnd, controls.normalizeCheckboxLastState);
        }

        // NOTE: Sample Rate and Channels controls are;
        //       EDITABLECOMBO in REAPER <=6.11
        //       COMBOBOX in REAPER >=6.12

        if(controls.sampleRateControlHwnd) {
            auto editControl = getComboBoxEdit(*controls.sampleRateControlHwnd);
            if(controls.sampleRateLastOption.length() > 0) {
                selectInComboBox(*controls.sampleRateControlHwnd, controls.sampleRateLastOption);
                SetWindowText(editControl, controls.sampleRateLastOption.c_str());
            }
            EnableWindow(editControl, true);
            UpdateWindow(editControl);
        }
        if(controls.channelsControlHwnd) {
            auto editControl = getComboBoxEdit(*controls.channelsControlHwnd);
            if(controls.channelsLastOption.length() > 0){
                selectInComboBox(*controls.channelsControlHwnd, controls.channelsLastOption);
                SetWindowText(editControl, controls.channelsLastOption.c_str());
            }
            UpdateWindow(editControl);
            ShowWindow(*controls.channelsControlHwnd, SW_SHOW);
        }
        if(controls.channelsLabelHwnd) {
            ShowWindow(*controls.channelsLabelHwnd, SW_SHOW);
        }
        if(controls.resampleModeControlHwnd) {
            auto editControl = getComboBoxEdit(*controls.resampleModeControlHwnd);
            EnableWindow(editControl, true);
            UpdateWindow(editControl);
        }
}
} // namespace admplug