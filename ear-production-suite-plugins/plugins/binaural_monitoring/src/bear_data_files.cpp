#include "bear_data_files.hpp"

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
    df->label = "TODO!";  // TODO
    df->isBearRelease = true;  // TODO
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
  selectedDataFile_.reset();
  auto it = std::find_if(availableDataFiles_.begin(), availableDataFiles_.end(),
                         [&filename](const std::shared_ptr<DataFile>& elm) {
                           return elm->filename == filename;
                         });

  if (it == availableDataFiles_.end()) return false;

  selectedDataFile_ = *it;
  return true;
}

}  // namespace plugin
}  // namespace ear
