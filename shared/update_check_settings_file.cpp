#include "update_check_settings_file.h"
#include <version/eps_version.h>
#include <helper/resource_paths_juce-file.hpp>

UpdateCheckerSettingsFile::UpdateCheckerSettingsFile()
{
    settingsFile = ResourcePaths::getSettingsDirectory(true).getChildFile("UpdateCheck.settings");
    if (!settingsFileExists()) {
        // No settings file - perhaps first run?
        if (ensureSettingsFileExists()) {
            // Settings file is writable so probably was just first run.
            // Set autocheck on initially.
            setAutoCheckEnabled(true);
        }
    }
    loadSettings();
}

const bool UpdateCheckerSettingsFile::getAutoCheckEnabled()
{
    return settingAutoCheckEnabled;
}

const bool UpdateCheckerSettingsFile::setAutoCheckEnabled(bool enabled)
{
    settingAutoCheckEnabled = enabled;
    return saveSettings();
}

const Version UpdateCheckerSettingsFile::getLastReportedVersion()
{
    return settingLastReportedVersion;
}

const bool UpdateCheckerSettingsFile::setLastReportedVersion(const Version& version)
{
    settingLastReportedVersion = version;
    return saveSettings();
}


const bool UpdateCheckerSettingsFile::canRead()
{
    return loadSettings();
}

const bool UpdateCheckerSettingsFile::canWrite()
{
    return saveSettings();
}

bool UpdateCheckerSettingsFile::loadSettings()
{
    if (!settingsFileExists()) {
        return false;
    }

    juce::XmlDocument xml(settingsFile);
    auto updateCheckElement = xml.getDocumentElementIfTagMatches("UpdateCheck");
    if (!updateCheckElement) {
        return false;
    }

    auto lastReportedElement = updateCheckElement->getChildByName("LastReportedVersion");
    if (lastReportedElement) {
        settingLastReportedVersion.major = lastReportedElement->getIntAttribute("VersionMajor", 0);
        settingLastReportedVersion.minor = lastReportedElement->getIntAttribute("VersionMinor", 0);
        settingLastReportedVersion.revision = lastReportedElement->getIntAttribute("VersionRevision", 0);
    }

    auto autoCheckElement = updateCheckElement->getChildByName("AutoCheck");
    if (autoCheckElement) {
        settingAutoCheckEnabled = autoCheckElement->getBoolAttribute("OnStartUp", false);
    }

    return true;
}

bool UpdateCheckerSettingsFile::saveSettings()
{
    auto updateCheckElement = juce::XmlElement("UpdateCheck");

    auto lastReportedElement = new juce::XmlElement("LastReportedVersion");
    lastReportedElement->setAttribute("VersionMajor", settingLastReportedVersion.major);
    lastReportedElement->setAttribute("VersionMinor", settingLastReportedVersion.minor);
    lastReportedElement->setAttribute("VersionRevision", settingLastReportedVersion.revision);
    updateCheckElement.addChildElement(lastReportedElement);

    auto autoCheckElement = new juce::XmlElement("AutoCheck");
    autoCheckElement->setAttribute("OnStartUp", settingAutoCheckEnabled);
    updateCheckElement.addChildElement(autoCheckElement);

    return updateCheckElement.writeTo(settingsFile);
}

bool UpdateCheckerSettingsFile::settingsFileExists()
{
    return settingsFile.existsAsFile();
}

bool UpdateCheckerSettingsFile::ensureSettingsFileExists()
{
    bool success = settingsFileExists();
    if (!success) {
        success = saveSettings() && settingsFileExists();
    }
    return success;
}
