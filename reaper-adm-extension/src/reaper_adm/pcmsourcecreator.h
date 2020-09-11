#pragma once
#include <memory>
#include <string>
#include <vector>
#include "importaction.h"
#include "progress/importreporter.h"
#include "pluginsuite.h"

namespace adm {
  class AudioObject;
  class AudioTrackUid;
}

namespace admplug {
class IChannelIndexer;
class IPCMGroupRegistry;
class TakeElement;
class ReaperAPI;
class IADMMetaData;
class PCMWriterFactory;
class PCMReader;

class IPCMSourceCreator {
public:
    virtual ~IPCMSourceCreator() = default;
    virtual void addTake(std::shared_ptr<TakeElement> take) = 0;
    virtual void linkSources(ReaperAPI const&) = 0;
    virtual void extractSources(std::string outputDir, ImportContext const& context) = 0;
};

class PCMSourceCreator : public IPCMSourceCreator
{
public:
    PCMSourceCreator(std::unique_ptr<IPCMGroupRegistry> registry,
                     std::unique_ptr<PCMReader> pcmReader,
                     std::unique_ptr<PCMWriterFactory> pcmWriterFactory,
                     IADMMetaData const& metaData);
    ~PCMSourceCreator() override;
    virtual void addTake(std::shared_ptr<TakeElement> take) override;
    void linkSources(ReaperAPI const&) override;
    void extractSources(std::string outputDir, ImportContext const& context) override;
private:
    std::vector<std::string> fileNames;
    std::vector<std::string> createSourceFiles(std::string outputDir,
                                               ImportListener &broadcast,
                                               ImportReporter const& import);
    std::vector<std::shared_ptr<TakeElement>> takes;
    std::unique_ptr<IPCMGroupRegistry> registry;
    std::unique_ptr<PCMReader> reader;
    std::unique_ptr<PCMWriterFactory> pcmWriterFactory;
    std::unique_ptr<IChannelIndexer> indexer;
    std::string inputFile;
    std::string inputFileShort;

};

//#ifdef WIN32
//class ProgressReporter
//{
//public:
//    ProgressReporter(size_t totalFrames) : totalFrames{ totalFrames } {
//        framesForEachPct = totalFrames / 100; // This mechanism will fail if there are < 100 frames, but thats never really gonna be a problem! - however, it avoids the more likely problem of a narrowing conversion to double to calculate percentage
//        if(framesForEachPct == 0) framesForEachPct = 1;
//        progressUpdate(0);
//    }
//    ~ProgressReporter() {}

//    void progressUpdate(size_t additionalFramesProcessed) {
//        framesProcessed += additionalFramesProcessed;
//        currentPct = framesProcessed / framesForEachPct;
//        if(currentPct > 100) currentPct = 100; // Could happen due to flooring of framesForEachPct calc

//        if (currentPct != lastReportedPct) {
//            std::string opText{ "Writing stem files... " };
//            opText.append(std::to_string(currentPct));
//            opText.append("%");
//            ImportProgressModalController::setStatusText(opText.c_str());
//            lastReportedPct = currentPct;
//        }
//    }

//private:
//    size_t totalFrames;
//    size_t framesForEachPct;
//    size_t framesProcessed{ 0 };
//    int lastReportedPct{ -1 };
//    int currentPct{ 0 };
//};
//#endif

}
