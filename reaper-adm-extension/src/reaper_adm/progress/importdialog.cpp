#include "importdialog.h"
#include "importstatus.h"
#include "../resource.h"
#include "../importexecutor.h"
#include <map>
#include <array>
#include <sstream>
#include <iomanip>
#include <optional>

using namespace admplug;

#define DLG_BG_COLOUR COLOR_3DFACE

#ifndef HIWORD
#define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#endif

namespace {

struct DialogControls {
    std::optional<std::string> header;
    std::optional<std::string> status;
    std::string buttonText;
    bool buttonEnabled;
};

auto noChange = std::optional<std::string>{};
std::map<ImportStatus, DialogControls> const dialogControlStates{
    {ImportStatus::INIT,                        {noChange, noChange, "Cancel", false}},
    {ImportStatus::STARTING,                    {noChange, noChange, "Cancel", true}},
    {ImportStatus::PARSING_METADATA,            {noChange, "Parsing ADM metadata...", "Cancel", true}},
    {ImportStatus::EXTRACTING_AUDIO,            {noChange, "Extracting audio", "Cancel", true}},
    {ImportStatus::AUDIO_READY,                 {noChange, "Audio imported. Creating REAPER elements...", "Cancel", false}},
    {ImportStatus::CREATING_REAPER_ELEMENTS,    {noChange, noChange, "Cancel", false}},
    {ImportStatus::COMPLETE,                    {"Import Complete", "", "OK", true}},
    {ImportStatus::CANCELLED,                   {"Import Cancelled. Tidying up...", "", "Cancel", false}},
    {ImportStatus::ERROR_OCCURRED,              {"Import Failed", noChange, "OK", true}},
    {ImportStatus::WARNING_OCCURRED,            {"Import Warning", noChange, "OK", true}}
};
}

#ifndef _WIN32
  #include <WDL/swell/swell-dlggen.h>
  #include "../reaper_adm_object.rc_mac_dlg"
#endif


namespace {
const int TIMER_ID = 100;
const int TIMER_PERIOD_MS = 50;
WDL_DLGRET dlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static std::map<HWND, std::unique_ptr<ReaperDialogBox>> dialogs;
    if(uMsg == WM_INITDIALOG) {
        auto dialog = reinterpret_cast<ReaperDialogBox*>(lParam);
        dialogs.insert({hwndDlg ,std::unique_ptr<ReaperDialogBox>(dialog)});
        return dialog->init(hwndDlg);
    }
    if(uMsg == WM_DESTROY) {
        auto it = dialogs.find(hwndDlg);
        if(it != dialogs.end()) {
            dialogs.erase(it);
        }
        return 0;
    }
    if(auto const& mapPair = dialogs.find(hwndDlg); mapPair != dialogs.end()) {
        return mapPair->second->process(hwndDlg, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
}
}

ReaperDialogBox::ReaperDialogBox(HWND main,
                                 REAPER_PLUGIN_HINSTANCE instance,
                                 std::shared_ptr<ImportProgress> parent,
                                 std::shared_ptr<ImportExecutor> exec) :
    parent{parent},
    executor{std::move(exec)}
{
    CreateDialogParam(instance, MAKEINTRESOURCE(IDD_FORMVIEW), main, &dlgProc, reinterpret_cast<LPARAM>(this));
}

ReaperDialogBox::~ReaperDialogBox()
{
    if (backgroundBrush) {
        DeleteObject(backgroundBrush);
    }
}

void ReaperDialogBox::setStatusText(std::string text) {
    SetDlgItemText(dialog, TEXT_UPDATE, text.c_str());
}

void ReaperDialogBox::setHeaderText(std::string text) {
    SetDlgItemText(dialog, TEXT_DESCRIPTION, text.c_str());
}

void ReaperDialogBox::setButtonText(std::string text) {
    SetDlgItemText(dialog, IDC_CANCELBUTTON, text.c_str());
}

void ReaperDialogBox::setButtonEnabled(bool enabled) {
    HWND btn = GetDlgItem(dialog, IDC_CANCELBUTTON);
    EnableWindow(btn, enabled);
}

std::string ReaperDialogBox::getStatusText() {
    static std::array<char, 1024> textBuffer;
    std::string text;
    if(GetDlgItemText(dialog, TEXT_UPDATE, textBuffer.data(), textBuffer.size())) {
      text = std::string(textBuffer.data());
    }
    return text;
}

INT_PTR ReaperDialogBox::process(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!backgroundBrush) {
        backgroundBrush = CreateSolidBrush(GetSysColor(DLG_BG_COLOUR));
    }

    switch (msg) {
    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = reinterpret_cast<HDC>(wParam);
        SetBkColor(hdcStatic, GetSysColor(DLG_BG_COLOUR));
        SetBkMode(hdcStatic, OPAQUE); // Do not attempt to use TRANSPARENT - old text is not cleared

        if (reinterpret_cast<HWND>(lParam) == GetDlgItem(dialog, TEXT_UPDATE)) {
            SetTextColor(hdcStatic, RGB(0, 0, 0));
        } else if (reinterpret_cast<HWND>(lParam) == GetDlgItem(dialog, TEXT_DESCRIPTION)) {
            SetTextColor(hdcStatic, RGB(128, 128, 128));
        }

        return reinterpret_cast<LRESULT>(backgroundBrush);
    }
    case WM_CTLCOLORDLG:
    {
        return reinterpret_cast<LRESULT>(backgroundBrush);
    }
#ifndef WIN32
    // This causes the timer to stall on windows,
    // but WM_SYSCOMMAND doesn't fire on macos so we need it there.
    case WM_CLOSE:
    {
        auto progress = parent.lock();
        if(progress) {
            auto state = progress->status();
            auto[headerTxt, statusTxt, buttonTxt, cancelAndCloseEnabled] = dialogControlStates.at(state);
            if(cancelAndCloseEnabled) {
                finish();
            }
        }
        return 0;
    }
#endif
    case WM_COMMAND:
    {
        if(HIWORD(wParam) == BN_CLICKED) {
            // Should really check which button was clicked, but theres only one...
            //if ((HWND)lParam == buttonHwnd)...
            if(currentState == ImportStatus::WARNING_OCCURRED) {
              currentState = ImportStatus::AUDIO_READY;
              onStateChanged();
            }
            else {
              finish();
            }
            return 0;
        }
        return DefWindowProc(window, msg, wParam, lParam);
    }

    case WM_TIMER:
    {
        auto progress = parent.lock();
        if(progress) {
            auto state = progress->status();
            if(currentState != state) {
                currentState = state;
                onStateChanged();
            }
            if(currentState == ImportStatus::INIT) {
                progress->setStatus(ImportStatus::STARTING);
                executor->start();
            }
            if(currentState == ImportStatus::EXTRACTING_AUDIO) {
                reportAudioProgress();
            }
            if(currentState == ImportStatus::COMPLETE) {
                finish();
            }
        }
        return 0;
    }
    default:
        return DefWindowProc(window, msg, wParam, lParam);
    }
}

void ReaperDialogBox::onStateChanged()
{
  update();
  if(currentState == ImportStatus::AUDIO_READY) {
      auto progress = parent.lock();
      if(progress) {
          progress->setStatus(ImportStatus::CREATING_REAPER_ELEMENTS);
      }
  }
  if(currentState == ImportStatus::CREATING_REAPER_ELEMENTS) {
      executor->buildProject();
  }
  if(currentState == ImportStatus::ERROR_OCCURRED) {
      auto progress = parent.lock();
      if(progress) {
          auto errorText = progress->getError();
          if(errorText) {
              setStatusText(*errorText);
          }
      }
  }
  if(currentState == ImportStatus::WARNING_OCCURRED) {
    auto progress = parent.lock();
    if(progress) {
        auto warningText = progress->getWarning();
        if(warningText) {
            setStatusText(*warningText);
        }
    }
  }
}

void ReaperDialogBox::update()
{
    auto[headerTxt, statusTxt, buttonTxt, buttonEnabled] = dialogControlStates.at(currentState);
    // Text
    if(headerTxt) {
        setHeaderText(*headerTxt);
    }
    if(statusTxt) {
        setStatusText(*statusTxt);
    }
    // Button
    setButtonEnabled(buttonEnabled);
    setButtonText(buttonTxt);
}

void ReaperDialogBox::reportAudioProgress()
{
    auto [headerTxt, statusTxt, buttonTxt, buttonEnabled] = dialogControlStates.at(ImportStatus::EXTRACTING_AUDIO);
    std::stringstream ss;
    ss.precision(0);
    ss << std::fixed;
    if(statusTxt) {
        ss << *statusTxt;
    }
    auto progress = parent.lock();
    if(progress) {
        auto [current, total] = progress->sampleProgress();
      if(current > 0) {
          auto percent = (current * 100.0) / total;
          ss << ' ' << percent << "%";
      }
      setStatusText(ss.str());
    }
}

BOOL ReaperDialogBox::init(HWND dialogHandle) {
    dialog = dialogHandle;
    SetTimer(dialogHandle, TIMER_ID, TIMER_PERIOD_MS, NULL);
    ShowWindow(dialog, SW_SHOW);
    return TRUE;
}


void ReaperDialogBox::finish()
{
    if(auto parentPtr = parent.lock(); parentPtr) {
        if(currentState == ImportStatus::COMPLETE || currentState == ImportStatus::ERROR_OCCURRED) {
            parentPtr->dialogClosed();
            KillTimer(dialog, TIMER_ID);
            DestroyWindow(dialog);
        } else if (!closing) {
            closing = true;
            parentPtr->setStatus(ImportStatus::CANCELLED);
        }
    } else {
        KillTimer(dialog, TIMER_ID);
        DestroyWindow(dialog);
    }
}
