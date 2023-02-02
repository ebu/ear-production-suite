#pragma once

#include <string>
#include <vector>
#include "JuceHeader.h"

struct InstallItem {
    juce::File source;
    juce::File destination;
    bool sourceValid;
};

class InstallManifest {
public:
    InstallManifest();
    ~InstallManifest();

    String getVst3Directory();
    String getUserPluginsDirectory();

    std::vector<String> getInvalidSources();

private:
    std::vector<InstallItem> installItems;
    File vst3Directory;
    File userPluginsDirectory;
};