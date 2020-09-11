#pragma once

#include <vector>
#include <sstream>
#include <iomanip>

#include "reaperapi.h"
#include "exportaction_issues.h"
#include <bw64/bw64.hpp>
#include <adm/adm.hpp>

// Helper function for converting ints to hex-strings used in ADM ID's
template< typename T >
std::string intToHex( T i )
{
    std::stringstream stream;
    stream << std::setfill ('0') << std::setw(sizeof(T)*2)
        << std::hex << i;
    return stream.str();
}

using namespace admplug;

class AdmVstExportSources;
class EarVstExportSources;

class IExportSources
{
    /*
    INTERFACE (Abstract base class)
    A container and manager for a specific type of source of ADM
    Formerly handled by; AdmExportContainer
    */
public:
    IExportSources(ReaperAPI const&api) {} // Instances should populate on instantiation (equivelent of old AdmExportContainer::repopulate() method)
    ~IExportSources() {}

    virtual bool documentRequiresProgrammeTitles() = 0; // Does this source collection still require programme titles? (some sources (i.e EAR) provide this already)
    virtual bool documentRequiresContentTitles() = 0;   // Does this source collection still require content titles? (some sources (i.e EAR) provide this already)

    virtual bool validForExport() = 0;  // Can we export something from the sources within this container?

    virtual int getSampleRate() = 0;
    virtual int getTotalExportChannels() = 0;

    virtual void setRenderInProgress(bool state) = 0;
    virtual bool isFrameAvailable() = 0;
    virtual bool writeNextFrameTo(float* bufferWritePointer, bool skipFrameAvailableCheck = false) = 0;

    virtual std::shared_ptr<bw64::AxmlChunk> getAxmlChunk() = 0;
    virtual std::shared_ptr<bw64::ChnaChunk> getChnaChunk() = 0;
    virtual std::shared_ptr<adm::Document> getAdmDocument() = 0; // Required access prior to stringifying so that programme name and content name can be updtaed

    virtual std::vector<std::string> generateExportInfoStrings() = 0; // Processing Info
    virtual std::vector<std::string> generateExportErrorStrings() = 0; // Errors occurred during processing
    virtual std::vector<std::string> generateExportWarningStrings() = 0; // Warnings generated during processing

    virtual std::string getExportSourcesName() {
        return std::string("Unknown Export Sources");
    }
};

class AdmExportHandler
{
    /*
    Finds and contains sources of ADM for export (IAdmExportSource children)
    Delivers metrics,
    Controls instances of IExportSources,
    Formerly partially handled by; AdmExportContainer
    */
public:
    AdmExportHandler() {}
    ~AdmExportHandler() {}

    void repopulate(ReaperAPI const&api);

    IExportSources* getAdmExportSources();

    std::vector<std::string> generateExportInfoStrings();
    std::vector<std::string> generateExportErrorStrings();
    std::vector<std::string> generateExportWarningStrings();

private:
    std::shared_ptr<AdmVstExportSources> admExportVstSources;
    std::shared_ptr<EarVstExportSources> earSceneMasterVstSources;

};