#pragma once

#include <vector>
#include <optional>
#include "JuceHeader.h"

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

private:
    std::vector<InstallItem> installItems;
};

class UninstallManifest {
public:
    UninstallManifest();
    ~UninstallManifest();

    std::vector<String> getFoundFiles();

private:
    void populateVectorsFromElement(juce::XmlElement* elm);
    std::optional<juce::File> pathFromElement(juce::XmlElement* elm);

    std::vector<UninstallFile> uninstallFiles;
    std::vector<UninstallDirectory> uninstallDirectories;
};