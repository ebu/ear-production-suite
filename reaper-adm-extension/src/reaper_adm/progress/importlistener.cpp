#include "importlistener.h"
#include "../reaper_plugin.h"
#include "../admimporter.h"

#include <string>
#include <WDL/swell/swell.h>
#include <WDL/wdltypes.h>

#ifdef _WIN32
#include <process.h>
#endif

using namespace admplug;

void ImportBroadcaster::addListener(std::shared_ptr<ImportListener> listener)
{
    listeners.push_back(std::move(listener));
}

void ImportBroadcaster::setStatus(ImportStatus status)
{
    for(auto& listener : listeners) {
        listener->setStatus(status);
    }
}

void ImportBroadcaster::elementAdded()
{
    for(auto& listener : listeners) {
        listener->elementAdded();
    }
}

void ImportBroadcaster::elementCreated()
{
    for(auto& listener : listeners) {
        listener->elementCreated();
    }
}

void ImportBroadcaster::totalFrames(uint64_t frames)
{
    for(auto& listener : listeners) {
        listener->totalFrames(frames);
    }
}

void ImportBroadcaster::framesWritten(uint64_t frames)
{
    for(auto& listener : listeners) {
        listener->framesWritten(frames);
    }
}

void ImportBroadcaster::error(const std::exception &e)
{
    for(auto& listener : listeners) {
        listener->error(e);
    }
}

