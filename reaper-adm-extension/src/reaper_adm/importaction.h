#ifndef IMPORTACTION_H
#define IMPORTACTION_H

#include "reaperapi.h"
#include <vector>
#include <memory>
#include "admimporter.h"
#include "admmetadata.h"
#include "pluginsuite.h"
#include "reaperhost.h"
#include "progress/importlistener.h"

namespace admplug {

class ImportAction
{
public:
    ImportAction(REAPER_PLUGIN_HINSTANCE hInstance, HWND main, std::shared_ptr<PluginSuite> suite);
    void import(std::string importFile, const ReaperAPI& api);
    void import(MediaItem* source, const ReaperAPI& api);

    static bool canMediaExplode_QuickCheck(const ReaperAPI& api, MediaItem* mediaItem);
    static bool canMediaExplode_QuickCheck(const ReaperAPI& api, std::string mediaFile);

private:
    std::shared_ptr<PluginSuite> pluginSuite;
    REAPER_PLUGIN_HINSTANCE hInstance;
    static PCM_source* getSourceFromMediaItem(MediaItem* mediaItem, const ReaperAPI& api);
    static std::string getFilenameFromMediaItem(MediaItem* mediaItem, const ReaperAPI& api);
    static std::shared_ptr<ADMMetaData> getAdmMetaData(const ReaperAPI *api, std::string filename, bool showError = false);
    void performImport(MediaItem* fromMediaItem, std::string fileName, const ReaperAPI &api);
    HWND main;
};

}


#endif // IMPORTACTION_H
