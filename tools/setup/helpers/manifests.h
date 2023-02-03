#pragma once

#include <vector>
#include <optional>
#include "JuceHeader.h"

namespace Locations {
    String getVst3Directory();
    String getUserPluginsDirectory();
}

struct InstallItem {
    juce::File source;
    juce::File destination;
    bool sourceValid;
};

struct UninstallFile {
    juce::File path;
};

struct UninstallDirectory {
    juce::File path;
    bool deleteAllContents;
};

class InstallManifest {
public:
    InstallManifest();
    ~InstallManifest();

    std::vector<String> getInvalidSources();
    void doInstall();
    std::vector<String> getInstallErrors();

private:
    std::vector<InstallItem> installItems;
    std::vector<String> installErrors;
    std::vector<String> installLog;
};

class UninstallManifest {
public:
    UninstallManifest();
    ~UninstallManifest();

    std::vector<String> getFoundFiles();
    void doUninstall();
    std::vector<String> getUninstallErrors();

private:
    void populateVectorsFromElement(juce::XmlElement* elm);
    std::optional<juce::File> pathFromElement(juce::XmlElement* elm);
    void sortDirectoriesDeepestFirst();

    std::vector<UninstallFile> uninstallFiles;
    std::vector<UninstallDirectory> uninstallDirectories;
    std::vector<String> uninstallErrors;
    std::vector<String> uninstallLog;
};