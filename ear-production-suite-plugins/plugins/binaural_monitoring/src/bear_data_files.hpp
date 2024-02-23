#pragma once
#include "JuceHeader.h"
#include <vector>
#include <functional>
#include <optional>
#include <boost/container/flat_map.hpp>

namespace ear {
namespace plugin {

class DataFileManager {
 public:
  DataFileManager();

  struct DataFile {
    juce::String filename;
    juce::File fullPath;
    juce::String label;
    juce::String description;
    bool isBearRelease{false};
    friend bool operator<(DataFile const& lhs, DataFile const& rhs){
      return lhs.fullPath.getFileName().compareIgnoreCase(rhs.fullPath.getFileName()) < 0;
    }
    friend bool operator==(DataFile const& lhs, DataFile const& rhs) {
      return lhs.fullPath.getFileName().compareIgnoreCase(rhs.fullPath.getFileName()) == 0;
    }
    friend bool operator!=(DataFile const& lhs, DataFile const& rhs) {
      return !(lhs == rhs);
    }
  };

  bool onlyContainsDefault() const;
  bool defaultIsSelected() const;
  void updateAvailableFiles();
  std::optional<DataFile> getSelectedDataFileInfo() const;
  std::vector<DataFile> getAvailableDataFiles() const;
  bool setSelectedDataFile(const juce::String& fullPath);
  bool setSelectedDataFileDefault();
  void onSelectedDataFileChange(
      std::function<void(DataFile const&)> callback);
  using FileMap = boost::container::flat_map<juce::File, DataFile>;

 private:
  void setSelectedDataFile(DataFile const& file);
  std::size_t getAvailableDataFilesCount() const;
  bool setSelectedDataFile(const juce::File& fullPath);
  std::optional<DataFile> getDataFileInfo(const juce::File& fullPath) const;
  juce::Array<juce::File> bearReleaseFiles_;
  std::optional<DataFile> selectedDataFile_;
  FileMap releasedDataFiles;
  FileMap customDataFiles;

  std::function<void(DataFile const&)> selectedDataFileChangeCallback_;
};

}  // namespace plugin
}

