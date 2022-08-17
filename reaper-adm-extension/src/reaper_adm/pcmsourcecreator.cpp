#include "pcmsourcecreator.h"
#include "pcmgroupregistry.h"
#include "channelindexer.h"
#include "pcmgroup.h"
#include "admmetadata.h"
#include "pcmwriterfactory.h"
#include "pcmwriter.h"
#include "pcmreader.h"
#include "reaperapi.h"
#include "mediatakeelement.h"

using namespace admplug;

PCMSourceCreator::PCMSourceCreator(std::unique_ptr<IPCMGroupRegistry> registry,
                                   std::unique_ptr<PCMReader> pcmReader,
                                   std::unique_ptr<PCMWriterFactory> pcmWriterFactory,
                                   IADMMetaData const& metadata) :
    registry{std::move(registry)},
    reader{std::move(pcmReader)},
    pcmWriterFactory{std::move(pcmWriterFactory)},
    indexer{std::make_unique<ChannelIndexer>(metadata)},
    inputFile{metadata.fileName()}
{
    size_t slashPos = inputFile.find_last_of("/\\");
    std::string noPath(slashPos == inputFile.npos? inputFile : inputFile.substr(slashPos+1));

    size_t dotPos = noPath.find_last_of(".");
    inputFileShort = (dotPos == noPath.npos ? noPath : noPath.substr(0, dotPos));
}

PCMSourceCreator::~PCMSourceCreator() = default;

void PCMSourceCreator::addTake(std::shared_ptr<TakeElement> take)
{
    takes.push_back(take);
}


void PCMSourceCreator::linkSources(const ReaperAPI& api) {
    for(std::size_t i = 0; i != fileNames.size(); ++i) {
        auto fileName = fileNames[i];
        auto pcmSource = api.PCM_Source_CreateFromFile(fileName.c_str());
        auto group = registry->allGroups()[i];
        registry->setTakeSourceFor(*group, pcmSource);
    }
}

void PCMSourceCreator::extractSources(std::string outputDir, const ImportContext &context)
{
    for(auto take : takes) {
        //auto channels = take->channels();
        // We do this as part of project tree now
        //channels = context.pluginSuite->reorderAndFilter(channels, context.api);
        //take->setChannels(channels);

        registry->add(take, PCMGroup{*indexer, take->trackUids()});

    }

    fileNames = createSourceFiles(outputDir, *context.broadcast, *context.import);
}

int admplug::PCMSourceCreator::channelForTrackUid(std::shared_ptr<const adm::AudioTrackUid> trackUid)
{
    return indexer->indexOf(trackUid);
}

std::vector<std::string> PCMSourceCreator::createSourceFiles(std::string outputDir,
                                                             ImportListener &broadcast,
                                                             ImportReporter const &import)
{
    std::vector<std::string> fileNames;
    std::vector<std::unique_ptr<IPCMWriter>> writers;
    for(auto group : registry->allGroups()) {
        auto writer = pcmWriterFactory->createGroupWriter(*group, outputDir, inputFileShort);
        fileNames.push_back(writer->fileName());
        writers.push_back(std::move(writer));
    }

    std::size_t totalFrames = reader->totalFrames();
    broadcast.totalFrames(totalFrames);
    std::size_t currentFrame{0};
    std::size_t currentBlock{0};
    auto readBlock = reader->read();
    while(readBlock->frameCount() > 0) {
        if(currentBlock % 20 == 0) {
            broadcast.framesWritten(currentFrame);
            if(import.status() == ImportStatus::CANCELLED) {
                break;
            }
        }

        for(auto& writer : writers) {
            writer->write(*readBlock);
        }
        currentFrame += readBlock->frameCount();
        currentBlock++;
        readBlock = reader->read();
    }
    return fileNames;
}


