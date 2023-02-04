#pragma once

#include <vector>
#include <optional>
#include <functional>
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

class InstallManifest : private Thread {
public:
    InstallManifest();
    ~InstallManifest() override;

    std::vector<String> getInvalidSources();
    void doInstall(std::function<void()> callbackWhenComplete);
    std::vector<String> getInstallErrors();

private:
    File dumpLog();
    void run() override;
    std::function<void()> callbackWhenInstallComplete;
    std::vector<InstallItem> installItems;
    std::vector<String> installErrors;
    std::vector<String> installLog;
};

class UninstallManifest : private Thread {
public:
    UninstallManifest();
    ~UninstallManifest() override;

    std::vector<String> getFoundFiles();
    void doUninstall(std::function<void()> callbackWhenComplete);
    std::vector<String> getUninstallErrors();

private:
    void populateVectorsFromElement(juce::XmlElement* elm);
    std::optional<juce::File> pathFromElement(juce::XmlElement* elm);
    void sortDirectoriesDeepestFirst();
    File dumpLog();
    void run() override;
    std::function<void()> callbackWhenUninstallComplete;
    std::vector<UninstallFile> uninstallFiles;
    std::vector<UninstallDirectory> uninstallDirectories;
    std::vector<String> uninstallErrors;
    std::vector<String> uninstallLog;
};