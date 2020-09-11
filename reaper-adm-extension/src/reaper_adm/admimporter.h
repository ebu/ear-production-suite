#pragma once
#include <string>
#include <vector>
#include <memory>
#include <adm/route.hpp>
#include "reaperapi.h"

namespace admplug {

class IADMMetaData;
class IPCMSourceCreator;
class ReaperAPI;
class ProjectTree;
class PluginSuite;
class ImportListener;
class ImportReporter;

struct ImportContext {
    std::shared_ptr<ImportListener> broadcast;
    std::shared_ptr<ImportReporter> import;
    std::shared_ptr<PluginSuite> pluginSuite;
    ReaperAPI const& api;
};

class ADMImporter
{
public:
    ADMImporter(MediaItem *fromMediaItem,
                std::string fileName,
                ImportContext context,
                std::string importPath);
    ~ADMImporter();
    void extractAudio();
    void buildProject();
    void parse();
private:
    MediaItem * originalMediaItem;
    std::string importPath;
    ImportContext context;
    HANDLE thread;
    std::shared_ptr<IPCMSourceCreator> sourceCreator;
    std::shared_ptr<IADMMetaData> metadata;
    std::unique_ptr<ProjectTree> project;
    std::string fileName;
    std::vector<std::shared_ptr<adm::AudioProgramme const>> programmes;
    std::vector<std::shared_ptr<adm::AudioContent const>> contents;
    std::vector<std::shared_ptr<adm::AudioObject const>> objects;
    std::vector<std::shared_ptr<adm::AudioTrackUid const>> uids;
};

}
