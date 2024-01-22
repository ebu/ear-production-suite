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
  bool setSelectedDataFile(const juce::String& filename);
  bool setSelectedDataFileDefault();
  std::shared_ptr<DataFile> getDataFileInfo(const juce::String& filename);
  bool dataFileAvailable(const juce::String& filename);
  void onSelectedDataFileChange(
      std::function<void(std::shared_ptr<DataFile>)> callback);

 private:
  std::shared_ptr<DataFile> selectedDataFile_;
  std::vector<std::shared_ptr<DataFile>> availableDataFiles_;
  std::function<void(std::shared_ptr<DataFile>)> selectedDataFileChangeCallback_;
};

}  // namespace plugin
}

