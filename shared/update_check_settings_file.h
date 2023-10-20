#pragma once
#include <AppConfig.h>
#include <juce_core/juce_core.h>
#include <helper/version.hpp>

class UpdateCheckerSettingsFile {
public:
    UpdateCheckerSettingsFile();
    ~UpdateCheckerSettingsFile();

    bool getAutoCheckEnabled();
    bool setAutoCheckEnabled(bool enabled);

    Version getLastReportedVersion();
    bool setLastReportedVersion(Version version);

    bool canReadSettingsFile();
    bool canWriteSettingsFile();

private:

    juce::File settingsFile;
    bool loadSettings();
    bool saveSettings();
    bool settingsFileExists();

    bool settingAutoCheckEnabled{ false };
    Version settingLastReportedVersion;
};
