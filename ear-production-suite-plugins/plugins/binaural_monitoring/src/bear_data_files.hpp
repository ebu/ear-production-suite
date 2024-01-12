#pragma once
#include "JuceHeader.h"
#include <memory>
#include <vector>

namespace ear {
namespace plugin {

class DataFileManager {
 public:
  DataFileManager(const juce::File& path);

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

 private:
  juce::File path_;
  std::shared_ptr<DataFile> selectedDataFile_;
  std::vector<std::shared_ptr<DataFile>> availableDataFiles_;
};

}  // namespace plugin
}

