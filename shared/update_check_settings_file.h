#pragma once
#include <AppConfig.h>
#include <juce_core/juce_core.h>
#include <helper/version.hpp>

class UpdateCheckerSettingsFile {
public:
    UpdateCheckerSettingsFile();

    const bool getAutoCheckEnabled();
    const bool setAutoCheckEnabled(bool enabled);

    const Version getLastReportedVersion();
    const bool setLastReportedVersion(const Version& version);

    const bool canRead();
    const bool canWrite();

private:

    juce::File settingsFile;
    bool loadSettings();
    bool saveSettings();
    bool settingsFileExists();
    bool ensureSettingsFileExists();

    bool settingAutoCheckEnabled{ false };
    Version settingLastReportedVersion;
};
