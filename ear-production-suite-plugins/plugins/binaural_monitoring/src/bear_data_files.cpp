#include "bear_data_files.hpp"
#include <bear/api.hpp>

namespace {
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

namespace {
void setFromMetadata(DataFileManager::DataFile& dataFile) {
  try {
    auto md = bear::DataFileMetadata::read_from_file(
        dataFile.fullPath.getFullPathName().toStdString());
    if (md.has_metadata()) {
      dataFile.label = md.get_label();
      dataFile.description = md.get_description();
      dataFile.isBearRelease = md.is_released();
    }
  }
  catch(std::exception const&) {
    /* 
    `read_from_file` could throw if;
     - the file does not exist, or otherwise unreadable
     - the file is not a valid tensorfile
     - the metadata within the file is not of the expected structure
    In all these cases, and any other, silently skip metadata extraction
    
    Note that a file with bad metadata may still be a usable filter set
    so we don't want to strike it off yet, nor is it worth doing deeper 
    checks here... the user can still select it as the processor will 
    catch unusable files (unreadable/invalid/corrupt etc) anyway when 
    it tries to start BEAR with it, and will report the actual error.
    */
  }
}

void updateFile(juce::File const& file, DataFileManager::FileMap& file_map,
                bool released) {
  auto [it, success] = file_map.insert(
      {file, DataFileManager::DataFile{.filename = file.getFileName(),
                                       .fullPath = file,
                                       .label = "",
                                       .description = "",
                                       .isBearRelease = released}});
  if (success) {  // not already added
    setFromMetadata(it->second);
  }
}
}  // namespace

bool DataFileManager::onlyContainsDefault() const {
  return customDataFiles.empty() && releasedDataFiles.size() == 1u;
}

void DataFileManager::updateAvailableFiles() {
  // add our expected released files
  for (auto const& file : bearReleaseFiles_) {
    if (file.existsAsFile()) {
      updateFile(file, releasedDataFiles, true);
    }
  }

  auto custom_files = getCustomDataFileDirectory().findChildFiles(
      juce::File::TypesOfFileToFind::findFiles, false, "*.tf");
  for (const auto& file : custom_files) {
    if (!releasedDataFiles.contains(file)) {  // on windows the custom / release
                                              // dirs are the same
      updateFile(file, customDataFiles, false);
    }
  }
}

std::optional<DataFileManager::DataFile>
DataFileManager::getSelectedDataFileInfo() const {
  return selectedDataFile_;
}

std::vector<DataFileManager::DataFile>
DataFileManager::getAvailableDataFiles() const {
  std::vector<DataFile> files;
  files.reserve(getAvailableDataFilesCount());
  auto fileExtractor = [](auto const& file) { return file.second; };
  std::transform(releasedDataFiles.begin(), releasedDataFiles.end(), std::back_inserter(files), fileExtractor);
  std::transform(customDataFiles.begin(), customDataFiles.end(), std::back_inserter(files), fileExtractor);
  return files;
}

std::size_t DataFileManager::getAvailableDataFilesCount() const {
  return releasedDataFiles.size() + customDataFiles.size();
}

void DataFileManager::setSelectedDataFile(DataFile const& file) {
  if(!selectedDataFile_ || file != *selectedDataFile_) {
    selectedDataFile_ = file;
    selectedDataFileChangeCallback_(file);
  }
}

bool DataFileManager::setSelectedDataFile(const juce::String& fullPath) {
  return setSelectedDataFile(juce::File(fullPath));
}

bool DataFileManager::setSelectedDataFile(const juce::File& fullPath) {
  auto const found = getDataFileInfo(fullPath);
  if (found) {
    setSelectedDataFile(*found);
  }
  return static_cast<bool>(found);
}

bool DataFileManager::setSelectedDataFileDefault() { 
  updateAvailableFiles();
  auto found = false;
  if(!releasedDataFiles.empty()) {
    setSelectedDataFile(releasedDataFiles.begin()->second);
    found = true;
  }
  return found;
}

namespace {
  std::optional<DataFileManager::DataFile> getDataFile(DataFileManager::FileMap const& map, juce::File const& file) {
    std::optional<DataFileManager::DataFile> dataFile;
  if(auto it = map.find(file); it != map.end()) {
    dataFile = it->second;
  }
  return dataFile;
  }
}

std::optional<DataFileManager::DataFile> DataFileManager::getDataFileInfo(
    const juce::File& fullPath) const {
  auto file = getDataFile(releasedDataFiles, fullPath);
  if(!file) {
    file = getDataFile(customDataFiles, fullPath);
  }
  return file;
}

void DataFileManager::onSelectedDataFileChange(
    std::function<void(DataFile const&)> callback) {
  selectedDataFileChangeCallback_ = std::move(callback);
}

}  // namespace plugin
}  // namespace ear
