#pragma once

#ifdef WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <WDL/swell/swell-functions.h>
#endif

#include <version/eps_version.h>
#include <string>

namespace NativeMessageBox {
#ifdef WIN32
namespace WinHelpers {

    /*
    Windows doesn't show message boxes whilst the Reaper splash screen is showing.
    Because message boxes are also blocking, trying to show one during splash will effectively freeze Reaper.
    This little helper launches the message box in a seperate thread, which seems to work.
    However, it is only suitable for informative 'OK-only' type messages, where you are not concerned with the user response.
    */

    typedef struct {
        std::string title;
        std::string text;
        int iconFlag;
    } NonBlockingMessageBox_ThreadData, * ptrNonBlockingMessageBox_ThreadData;

    inline DWORD WINAPI NonBlockingMessageBox_ThreadFunc(LPVOID threadData) {
        auto pThreadData = (ptrNonBlockingMessageBox_ThreadData)threadData;
        LPCSTR text = pThreadData->text.c_str();
        LPCSTR title = pThreadData->title.c_str();
        int iconFlag = pThreadData->iconFlag;
        MessageBox(NULL, text, title, MB_OK | iconFlag);
        delete pThreadData;
        return 0;
    }

    inline void NonBlockingMessageBox(std::string text, std::string title, int iconFlag = 0) {
        auto NonBlockingMessageBox_LatestThreadData = new NonBlockingMessageBox_ThreadData{ title, text, iconFlag };
        CreateThread(NULL, 0, NonBlockingMessageBox_ThreadFunc, NonBlockingMessageBox_LatestThreadData, 0, NULL);
    }

}
#endif

    inline void splashCompatibleMessage(const std::string& title, const std::string& msg, HWND hwnd = nullptr, long winIcon = MB_ICONINFORMATION) {
#ifdef WIN32
        // Windows version of Reaper locks up if you try show a message box during splash
        WinHelpers::NonBlockingMessageBox(msg, title, winIcon);
#else
        MessageBox(hwnd, msg.c_str(), title.c_str(), MB_OK);
#endif
    };


    inline void splashExtensionError(const char* errMsg, HWND hwnd = nullptr) {
        std::string text{ errMsg };
        if (eps::versionInfoAvailable()) {
            text += "\n\n[EAR Production Suite v";
            text += eps::currentVersion();
            text += "]";
        }
        else {
            text += "\n\n[EAR Production Suite version information unavailable!]";
        }
        splashCompatibleMessage("EAR Production Suite - Extension Error", text, hwnd, MB_ICONEXCLAMATION);
    };

}

