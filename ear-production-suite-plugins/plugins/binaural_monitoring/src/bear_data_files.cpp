#include "bear_data_files.hpp"
#include <bear/api.hpp>

namespace {
bool operator==(const ear::plugin::DataFileManager::DataFile& lhs,
                const ear::plugin::DataFileManager::DataFile& rhs) {
  return (lhs.filename == rhs.filename) && (lhs.fullPath == rhs.fullPath) &&
         (lhs.isBearRelease == rhs.isBearRelease) && (lhs.label == rhs.label) &&
         (lhs.description == rhs.description);
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

juce::File getCustomDataFileDirectory() {
  auto vstPath = juce::File::getSpecialLocation(
      juce::File::SpecialLocationType::currentExecutableFile);
  vstPath = vstPath.getParentDirectory();
#ifdef __APPLE__
  // vstPath is `EAR Binaural Monitoring.vst3/Contents/MacOS` - traverse up x3 to get to dir containing vst3 bundle
  vstPath = vstPath.getParentDirectory();
  vstPath = vstPath.getParentDirectory();
  vstPath = vstPath.getParentDirectory();
#endif
  return vstPath;
};

}

namespace ear {
namespace plugin {

DataFileManager::DataFileManager() {
  bearReleaseFiles_.add(
      getBearDataFileDirectory().getChildFile(BEAR_DATA_FILE));
  updateAvailableFiles();
}

void DataFileManager::updateAvailableFiles() {
  // Start a new vec and copy existing shared_ptrs of unchanged files 
  // This retains shared_ptrs unlike clear() and rebuild
  std::vector<std::shared_ptr<DataFileManager::DataFile>> dfs;
  // Lookup tfs where we expect to find custom files
  auto files = getCustomDataFileDirectory().findChildFiles(
      juce::File::TypesOfFileToFind::findFiles,
                                    false, "*.tf");
  // add our expected released files
  for(auto const& bearReleaseFile : bearReleaseFiles_) {
    if (bearReleaseFile.existsAsFile()) {
      // note that in win, the released bear dir is the same as the custom
      // dir, so we might have already found this with findChildFiles
      files.addIfNotAlreadyThere(bearReleaseFile);
    }
  }
  for (const auto& file : files) {
    auto newDf = std::make_shared<DataFile>();
    newDf->fullPath = file;
    newDf->filename = file.getFileName();
    try {
      auto md = bear::DataFileMetadata::read_from_file(
          file.getFullPathName().toStdString());
      if (md.has_metadata()) {
        newDf->label = md.get_label();
        newDf->description = md.get_description();
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

bool DataFileManager::setSelectedDataFile(const juce::String& fullPath) {
  return setSelectedDataFile(juce::File(fullPath));
}

bool DataFileManager::setSelectedDataFile(const juce::File& fullPath) {
  auto found = getDataFileInfo(fullPath);
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
  // look for any released by order
  for (auto const& bearReleaseFile : bearReleaseFiles_) {
    if (setSelectedDataFile(bearReleaseFile)) {
      return true;
    }
  }
  return false;
}

std::shared_ptr<DataFileManager::DataFile> DataFileManager::getDataFileInfo(
    const juce::String& fullPath) {
  return getDataFileInfo(juce::File(fullPath));
}

std::shared_ptr<DataFileManager::DataFile> DataFileManager::getDataFileInfo(
    const juce::File& fullPath) {
  auto it = std::find_if(availableDataFiles_.begin(), availableDataFiles_.end(),
                         [&fullPath](const std::shared_ptr<DataFile>& elm) {
                           return elm->fullPath == fullPath;
                         });

  return it == availableDataFiles_.end() ? nullptr : *it;
}

void DataFileManager::onSelectedDataFileChange(
    std::function<void(std::shared_ptr<DataFile>)> callback) {
  selectedDataFileChangeCallback_ = callback;
}

}  // namespace plugin
}  // namespace ear
