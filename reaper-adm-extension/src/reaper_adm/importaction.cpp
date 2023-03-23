#include "importaction.h"
#include "importexecutor.h"
#include "progress/importlistener.h"
#include "progress/importprogress.h"
#include "progress/importdialog.h"
#include "bw64/bw64.hpp"
#include "admvstcontrol.h"
#include <array>

using namespace admplug;

ImportAction::ImportAction(REAPER_PLUGIN_HINSTANCE hInstance, HWND main, std::shared_ptr<PluginSuite> suite) :
    pluginSuite{suite},
    hInstance{ hInstance }, main{main}
{
}

void ImportAction::import(std::string importFile, const ReaperAPI& api)
{
    performImport(nullptr, importFile, api);
}

void ImportAction::import(MediaItem* fromMediaItem, const ReaperAPI& api)
{
    performImport(fromMediaItem, getFilenameFromMediaItem(fromMediaItem, api), api);
}

void ImportAction::performImport(MediaItem* fromMediaItem, std::string fileName, const ReaperAPI& api)
{
    if (pluginSuite->pluginSuiteRequiresAdmExportVst() && !AdmVst::isAvailable(api)) {
        int resp = api.ShowMessageBox("ADM Export VST not found!\n\nYou will be unable to export to ADM without this.\n\nClick OK to continue importing without export support.", "ADM Export VST", 1);
        //1 == OK, 2 == Close dialog/Cancel
        if (resp == 2) return;
    }

    auto progress = std::make_shared<ImportProgress>(hInstance, main);
    auto broadcast = std::make_shared<ImportBroadcaster>();
    broadcast->addListener(progress);
    std::array<char, 4096> buffer{};
    api.GetProjectPath(buffer.data(), buffer.size());
    std::string projectPath(buffer.data());

    auto importer = std::make_unique<ADMImporter>(fromMediaItem,
                                                  fileName,
                                                  ImportContext{broadcast, progress, pluginSuite, api},
                                                  projectPath);
    auto importExecutor = std::make_shared<ThreadedImport>(std::move(importer));
    // construction is for side effects - cleans up after itself when window closed
    new ReaperDialogBox(main, hInstance, progress, importExecutor);
}

PCM_source* ImportAction::getSourceFromMediaItem(MediaItem* mediaItem, const ReaperAPI& api) {
  auto currentTake = api.GetActiveTake(mediaItem);
  auto source = api.GetMediaItemTake_Source(currentTake);
  auto parentSource = api.GetMediaSourceParent(source);
  while (parentSource) {
    source = parentSource;
    parentSource = api.GetMediaSourceParent(source);
  }
  return source;
}

std::string admplug::ImportAction::getFilenameFromMediaItem(MediaItem * mediaItem, const ReaperAPI & api)
{
    std::array<char, 4096> buffer{};
    auto source = getSourceFromMediaItem(mediaItem, api);
    api.GetMediaSourceFileName(source, buffer.data(), buffer.size());
    return std::string(buffer.data());
}

std::shared_ptr<ADMMetaData> admplug::ImportAction::getAdmMetaData(const ReaperAPI *api, std::string filename, bool showError)
{
    try {
        return std::make_shared<ADMMetaData>(filename);
    }
    catch (const std::exception& e) {
        // Probably not RIFF/BW64/RF64, not WAVE, or simply not parseable.
        if (api && showError) {
            std::string msg("Error during ADM Metadata extraction:\n\n");
            msg += e.what();
            api->ShowMessageBox(msg.c_str(), "ADM Metadata", 0);
        }
    }
    return std::shared_ptr<ADMMetaData>();
}

bool admplug::ImportAction::canMediaExplode_QuickCheck(const ReaperAPI & api, MediaItem * mediaItem, std::string* errOut)
{
    return canMediaExplode_QuickCheck(api, getFilenameFromMediaItem(mediaItem, api), errOut);
}

bool admplug::ImportAction::canMediaExplode_QuickCheck(const ReaperAPI & api, std::string mediaFile, std::string* errOut)
{
    try {
        auto bw64File = bw64::readFile(mediaFile);
        if (!bw64File->chnaChunk()) return false;
        if (!bw64File->axmlChunk()) return false;
    }
    catch (const std::runtime_error& e) {
        // Probably not RIFF/BW64/RF64 or not WAVE.
        if(errOut) {
            *errOut = e.what();
        }
        return false;
    }
    return true;
}


