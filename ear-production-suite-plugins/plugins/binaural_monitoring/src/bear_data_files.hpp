#pragma once
#include "JuceHeader.h"
#include <memory>
#include <vector>
#include <functional>

namespace ear {
namespace plugin {

class DataFileManager {
 public:
  DataFileManager();

  struct DataFile {
    juce::String filename;
    juce::File fullPath;
    juce::String label;
    bool isBearRelease{false};
  };

  void updateAvailableFiles();
  std::shared_ptr<DataFile> getSelectedDataFileInfo();
  std::vector<std::shared_ptr<DataFile>> getAvailableDataFiles();
  int getAvailableDataFilesCount();
  bool setSelectedDataFile(const juce::String& fullPath);
  bool setSelectedDataFile(const juce::File& fullPath);
  bool setSelectedDataFileDefault();
  std::shared_ptr<DataFile> getDataFileInfo(const juce::String& fullPath);
  std::shared_ptr<DataFile> getDataFileInfo(const juce::File& fullPath);
  void onSelectedDataFileChange(
      std::function<void(std::shared_ptr<DataFile>)> callback);

 private:
  juce::Array<juce::File> bearReleaseFiles_;
  std::shared_ptr<DataFile> selectedDataFile_;
  std::vector<std::shared_ptr<DataFile>> availableDataFiles_;
  std::function<void(std::shared_ptr<DataFile>)> selectedDataFileChangeCallback_;
};

}  // namespace plugin
}

