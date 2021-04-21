#include "importprogress.h"

using namespace admplug;

ImportProgress::ImportProgress(REAPER_PLUGIN_HINSTANCE instance,
                                           HWND main) :
    hInstance(instance),
    main(main)
{

}

void ImportProgress::setStatus(ImportStatus newStatus) {
    std::scoped_lock<std::mutex> lock(mutex);
    state = newStatus;
}

void ImportProgress::elementAdded()
{
    std::scoped_lock<std::mutex> lock(mutex);
    ++progress.parsedCount;
}

void ImportProgress::elementCreated()
{
    std::scoped_lock<std::mutex> lock(mutex);
    ++progress.createdCount;
}

void ImportProgress::totalFrames(uint64_t frames)
{
    std::scoped_lock<std::mutex> lock(mutex);
    frameCount = frames;
}

void ImportProgress::framesWritten(uint64_t frames)
{
    std::scoped_lock<std::mutex> lock(mutex);
    currentFrames = frames;
}

std::pair<uint64_t, uint64_t> ImportProgress::sampleProgress() {
    std::scoped_lock<std::mutex> lock(mutex);
    return std::make_pair(currentFrames, frameCount);
}

ImportStatus ImportProgress::status() const
{
    std::scoped_lock<std::mutex> lock(mutex);
    return state;
}

ElementProgress ImportProgress::getProgress()
{
    std::scoped_lock<std::mutex> lock(mutex);
    return progress;
}

void ImportProgress::error(const std::exception &e)
{
    std::scoped_lock<std::mutex> lock(mutex);
    state = ImportStatus::ERROR_OCCURRED;
    errorText = e.what();
}

void ImportProgress::warning(const std::string& textToShow, const ImportStatus nextState)
{
    std::scoped_lock<std::mutex> lock(mutex);
    stateToEnterAfterWarning = nextState;
    state = ImportStatus::WARNING_OCCURRED;
    warningText = textToShow;
}

std::optional<std::string> ImportProgress::getError() {

    std::scoped_lock<std::mutex> lock(mutex);
    return errorText;
}

std::optional<std::string> ImportProgress::getWarning() {

    std::scoped_lock<std::mutex> lock(mutex);
    return warningText;
}


void ImportProgress::dialogClosed()
{
    box = nullptr;
}

void ImportProgress::setDialog(ReaperDialogBox *dialog)
{
    box = dialog;
}
