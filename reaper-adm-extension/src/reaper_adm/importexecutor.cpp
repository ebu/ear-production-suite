#include "importexecutor.h"
#include "importaction.h"

#ifdef _WIN32
#include <process.h>
#endif

using namespace admplug;

unsigned WINAPI importThreadFn(void* importer_ptr) {
    auto executor = reinterpret_cast<ImportExecutor*>(importer_ptr);
    executor->parse();
    return 0;
}

SerialImport::SerialImport(std::shared_ptr<ADMImporter> action) : action{std::move(action)}
{

}

void SerialImport::start()
{
    parseAndExtract();
}

void SerialImport::parseAndExtract()
{
    action->extractAudio();
}

void SerialImport::buildProject()
{
    action->buildProject();
}

ThreadedImport::ThreadedImport(std::shared_ptr<ADMImporter> action) : action{std::move(action)}
{
}

ThreadedImport::~ThreadedImport()
{
    closeThread();
}

void ThreadedImport::start()
{
    thread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, &importThreadFn, this, 0, NULL));
}

void ThreadedImport::parse()
{
    action->parse();
}

void ThreadedImport::parseAndExtract()
{
    action->extractAudio();
}

void ThreadedImport::buildProject() {
    closeThread();
    action->buildProject();
}

void ThreadedImport::closeThread()
{
    if(thread) {
        CloseHandle(thread);
        thread = nullptr;
    }

}
