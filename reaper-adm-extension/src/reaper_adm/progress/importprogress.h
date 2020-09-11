#pragma once

#include <mutex>
#include <optional>
#include "importlistener.h"
#include "importreporter.h"
#include "../reaper_plugin.h"
#include <WDL/wdltypes.h>

namespace admplug {

class ReaperDialogBox;

struct ElementProgress {
    int parsedCount;
    int createdCount;
};

class ImportProgress : public ImportListener, public ImportReporter {

public:
    ImportProgress(REAPER_PLUGIN_HINSTANCE instance, HWND main);
    void setStatus(ImportStatus status) override;
    void elementAdded() override;
    void elementCreated() override;
    virtual void totalFrames(uint64_t frames) override;
    virtual void framesWritten(uint64_t frames) override;
    void error(const std::exception &e) override;
    ImportStatus status() const override;
    ElementProgress getProgress();
    void dialogClosed();
    void setDialog(ReaperDialogBox* box);
    std::pair<uint64_t, uint64_t> sampleProgress();
    std::optional<std::string> getError();
private:
    uint64_t frameCount{0};
    uint64_t currentFrames{0};
    ImportStatus state{ImportStatus::INIT};
    mutable std::mutex mutex;
    ElementProgress progress{};
    void setHeaderText(const std::string &text);
    void setStatusText(std::string const& text);
    std::string getStatusText();
    void reportAudioProgress();
    REAPER_PLUGIN_HINSTANCE hInstance;
    HWND main{nullptr};
    ReaperDialogBox* box {nullptr};
    std::optional<std::string> errorText;
};

}
