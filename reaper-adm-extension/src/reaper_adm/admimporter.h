#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <adm/route.hpp>
#include "reaperapi.h"
#include "parameter.h"

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
    bool haveParametersBeenClipped();
    void sendWarningMessage();
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
    
    std::unordered_map<AdmParameter, std::pair<int, std::string>> clippedParamInfo {
      {AdmParameter::OBJECT_AZIMUTH,   {0, {"Azimuth"}}},
      {AdmParameter::OBJECT_DISTANCE,  {0, {"Distance"}}},
      {AdmParameter::OBJECT_ELEVATION, {0, {"Elevation"}}},
      {AdmParameter::OBJECT_GAIN,      {0, {"Gain"}}},
      {AdmParameter::OBJECT_HEIGHT,    {0, {"Height"}}},
      {AdmParameter::OBJECT_WIDTH,     {0, {"Width"}}},
      {AdmParameter::OBJECT_DEPTH,     {0, {"Depth"}}},
      {AdmParameter::OBJECT_DIFFUSE,   {0, {"Diffuse"}}}
    };
};

}
