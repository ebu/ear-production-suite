#include "bear_data_files.hpp"
#include <bear/api.hpp>

namespace {
bool operator==(const ear::plugin::DataFileManager::DataFile& lhs,
                const ear::plugin::DataFileManager::DataFile& rhs) {
  return (lhs.filename == rhs.filename) && (lhs.fullPath == rhs.fullPath) &&
         (lhs.isBearRelease == rhs.isBearRelease) && (lhs.label == rhs.label);
}

juce::File getBearDataFileDirectory() {
  auto vstPath = juce::File::getSpecialLocation(
      juce::File::SpecialLocationType::currentExecutableFile);
  vstPath = vstPath.getParentDirectory();
#ifdef __APPLE__
  vstPath = vstPath.getParentDirectory();
  vstPath = vstPath.getChildFile("Resources");
#endif
  return vstPath;
};

}

namespace ear {
namespace plugin {

DataFileManager::DataFileManager() {
  updateAvailableFiles();
}

void DataFileManager::updateAvailableFiles() {
  // Start a new vec and copy existing shared_ptrs of unchanged files 
  // This retains shared_ptrs unlike clear() and rebuild
  std::vector<std::shared_ptr<DataFileManager::DataFile>> dfs;
  auto files = getBearDataFileDirectory().findChildFiles(
      juce::File::TypesOfFileToFind::findFiles,
                                    false, "*.tf");
  for (const auto& file : files) {
    auto newDf = std::make_shared<DataFile>();
    newDf->fullPath = file;
    newDf->filename = file.getFileName();
    try {
      auto md = bear::DataFileMetadata::read_from_file(
          file.getFullPathName().toStdString());
      if (md.has_metadata()) {
        newDf->label = md.get_label();
        newDf->isBearRelease = md.is_released();
      }
    } catch (std::exception) {
    }
    // Use the existing struct where possible (exists and is identical)
    // -- this retains the same inst and shared_ptr
    auto existingDf = getDataFileInfo(file.getFileName());
    if (existingDf && *existingDf == *newDf) {
      dfs.push_back(existingDf);
    } else {
      dfs.push_back(newDf);
    }
  }
  availableDataFiles_ = dfs;
}

std::shared_ptr<DataFileManager::DataFile>
DataFileManager::getSelectedDataFileInfo() {
  return selectedDataFile_;
}

std::vector<std::shared_ptr<DataFileManager::DataFile>>
DataFileManager::getAvailableDataFiles() {
  return availableDataFiles_;
}

int DataFileManager::getAvailableDataFilesCount() {
  return availableDataFiles_.size();
}

bool DataFileManager::setSelectedDataFile(const juce::String& filename) {
  auto found = getDataFileInfo(filename);
  if (found == nullptr) return false;
  if (found != selectedDataFile_) {
    selectedDataFile_ = found;
    if (selectedDataFileChangeCallback_) {
      selectedDataFileChangeCallback_(selectedDataFile_);
    }
  }
  return true;
}

bool DataFileManager::setSelectedDataFileDefault() { 
  updateAvailableFiles();
  // try look for expected file first
  auto defaultFileInfo = getDataFileInfo(BEAR_DATA_FILE);
  if (defaultFileInfo && setSelectedDataFile(defaultFileInfo->filename)) {
    return true;
  }
  // else look for any released (may have been renamed)
  for (auto const& df : availableDataFiles_) {
    if (df->isBearRelease && setSelectedDataFile(df->filename)) {
      return true;
    }
  }
  return false;
}

std::shared_ptr<DataFileManager::DataFile> DataFileManager::getDataFileInfo(const juce::String& filename) {
  auto it = std::find_if(availableDataFiles_.begin(), availableDataFiles_.end(),
                         [&filename](const std::shared_ptr<DataFile>& elm) {
                           return elm->filename == filename;
                         });

  return it == availableDataFiles_.end() ? nullptr : *it;
}

void DataFileManager::onSelectedDataFileChange(
    std::function<void(std::shared_ptr<DataFile>)> callback) {
  selectedDataFileChangeCallback_ = callback;
}

}  // namespace plugin
}  // namespace ear
