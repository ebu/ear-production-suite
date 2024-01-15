#include "bear_data_files.hpp"
#include <bear/api.hpp>

namespace ear {
namespace plugin {

DataFileManager::DataFileManager(const juce::File& path) {
  path_ = path;
  updateAvailableFiles();
}

void DataFileManager::updateAvailableFiles() {
  availableDataFiles_.clear();
  auto files = path_.findChildFiles(juce::File::TypesOfFileToFind::findFiles,
                                    false, "*.tf");
  for (const auto& file : files) {
    auto df = std::make_shared<DataFile>();
    df->fullPath = file;
    df->filename = file.getFileName();
    try {
      auto md = bear::DataFileMetadata::read_from_file(
          file.getFullPathName().toStdString());
       if(md.has_metadata()) {
        df->label = md.get_label();
        df->isBearRelease = md.is_released();
      }
    } catch (std::exception) {
    }
    availableDataFiles_.push_back(df);
  }
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

bool DataFileManager::dataFileAvailable(const juce::String& filename) {
  return getDataFileInfo(filename) != nullptr;
}

void DataFileManager::onSelectedDataFileChange(
    std::function<void(std::shared_ptr<DataFile>)> callback) {
  selectedDataFileChangeCallback_ = callback;
}

}  // namespace plugin
}  // namespace ear
