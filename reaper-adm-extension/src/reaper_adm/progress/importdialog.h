#pragma once

#include "importprogress.h"

namespace admplug {
class ImportExecutor;
class ReaperDialogBox {
public:
    ReaperDialogBox(HWND main,
                    REAPER_PLUGIN_HINSTANCE instance,
                    std::shared_ptr<ImportProgress> parent,
                    std::shared_ptr<ImportExecutor> executor);
    ReaperDialogBox(const ReaperDialogBox& other) = delete;
    ReaperDialogBox& operator=(ReaperDialogBox const& other) = delete;
    ~ReaperDialogBox();
    BOOL init(HWND dialogHandle);
    WDL_DLGRET process(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
private:
    void onStateChanged();
    void update();
    void finish();
    std::string getStatusText();
    void setStatusText(std::string text);
    void setHeaderText(std::string text);
    void setButtonText(std::string text);
    void setButtonEnabled(bool enabled);
    void reportAudioProgress();
    ImportStatus currentState {ImportStatus::INIT};
    HWND dialog;
    std::weak_ptr<ImportProgress> parent;
    std::shared_ptr<ImportExecutor> executor;
    HBRUSH backgroundBrush{ nullptr };
    bool closing{false};
};

}
