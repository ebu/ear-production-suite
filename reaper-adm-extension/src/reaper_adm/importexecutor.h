#pragma once
#include <memory>
#include "reaper_plugin.h"

namespace admplug {
class ImportExecutor {
public:
    virtual ~ImportExecutor() = default;
    virtual void start() = 0;
    virtual void parse() = 0;
    virtual void parseAndExtract() = 0;
    virtual void buildProject() = 0;
};

class ADMImporter;

class SerialImport : public ImportExecutor {
public:
    SerialImport(std::shared_ptr<ADMImporter> action);
    void start();
    void parseAndExtract();
    void buildProject();
private:
    std::shared_ptr<ADMImporter> action;
};

class ThreadedImport : public ImportExecutor {
public:
    ThreadedImport(std::shared_ptr<ADMImporter> action);
    ~ThreadedImport();
    void start();
    void parse();
    void parseAndExtract();
    void buildProject();
private:
    void closeThread();
    HANDLE thread{nullptr};
    std::shared_ptr<ADMImporter> action;
};

}
