#pragma once

#include "JuceHeader.h"

inline std::unique_ptr<PropertiesFile> getPropertiesFile(
    InterProcessLock* lock) {
  PropertiesFile::Options options;
  options.applicationName = String("EAR Production Suite");
  options.filenameSuffix = String("xml");
  options.folderName = String("EBU");
  options.osxLibrarySubFolder = String("Application Support");
  options.processLock = lock;
  return std::make_unique<PropertiesFile>(options);
}
