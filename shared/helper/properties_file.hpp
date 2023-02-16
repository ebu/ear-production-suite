#pragma once

#include "JuceHeader.h"
#include "resource_paths_juce-file.hpp"

inline std::unique_ptr<PropertiesFile> getPropertiesFile(
    InterProcessLock* lock) {
  PropertiesFile::Options options;
  options.applicationName = String("AllPlugins");
  options.filenameSuffix = String("settings");
  options.folderName = ResourcePaths::getSettingsDirectory(true).getFullPathName();
  options.processLock = lock;
  return std::make_unique<PropertiesFile>(options);
}
