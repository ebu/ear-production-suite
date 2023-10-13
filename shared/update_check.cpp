#include "update_check.h"
#include <version/eps_version.h>
#include <iostream>
#include <vector>
#include <helper/resource_paths_juce-file.hpp>
#include <helper/native_message_box.hpp>

UpdateChecker::UpdateChecker()
{
    if (eps::versionInfoAvailable()) {
        currentVersion = Version(
            eps::versionMajor(),
            eps::versionMinor(),
            eps::versionRevision());
    }

    settingsFile = ResourcePaths::getSettingsDirectory(true).getChildFile("UpdateCheck.settings");
    if (!settingsFileExists()) {
        // No settings file - perhaps first run?
        if (saveSettings() && settingsFileExists()) {
            // Settings file is writable so probably was just first run.
            // Set autocheck on initially.
            setAutoCheckEnabled(true);
        }
    }
    loadSettings();
}

UpdateChecker::~UpdateChecker()
{
}

bool UpdateChecker::getAutoCheckEnabled()
{
    return settingAutoCheckEnabled;
}

bool UpdateChecker::setAutoCheckEnabled(bool enabled, bool displayConfirmation)
{
    settingAutoCheckEnabled = enabled;
    auto success = saveSettings();
    if (displayConfirmation) {
        if (success) {
            if (enabled) {
                displayMessageBox(messageBoxTitles, "Update check will now be performed each time REAPER is started.", MB_ICONINFORMATION);
            }
            else {
                displayMessageBox(messageBoxTitles, "Update check will no longer be performed when REAPER is started.", MB_ICONINFORMATION);
            }
        }
        else {
            displayMessageBox(messageBoxTitles, "Error: Failed to save preferences.", MB_ICONEXCLAMATION);
        }
    }
    return success;
}

void UpdateChecker::doUpdateCheck(bool manualCheck, int timeoutMs)
{
    Version remoteVersion;
    std::string remoteVersionStr;
    auto success = getRemoteVersion(manualCheck, timeoutMs, remoteVersion, remoteVersionStr);

    if (success) {
        if (remoteVersion > currentVersion) {
            if (manualCheck || remoteVersion != settingLastReportedVersion) {
                // Haven't mentioned this version before or told to always show result
                settingLastReportedVersion = remoteVersion;
                saveSettings();
                displayUpdateAvailable(remoteVersionStr, manualCheck);
            }
        }
        else if (manualCheck) {
            displayUpdateUnavailable();
        }
    }
}

bool UpdateChecker::getRemoteVersion(bool reportErrors, int timeoutMs, Version& version, std::string& versionStr)
{
    std::string body;
    auto getSuccess = getHTTPResponseBody(versionJsonUrl, body, timeoutMs);
    if (!getSuccess) {
        if (reportErrors) {
            displayError("Failed to connect to server. Do you have a working internet connection?");
        }
        return false;
    }

    juce::var j;
    auto parseResult = juce::JSON::parse(body, j);
    if (parseResult.failed()) {
        if (reportErrors) {
            displayError("Failed to parse data.");
        }
        return false;
    }

    juce::var varVersionMajor = j.getProperty("version_major", juce::var());
    juce::var varVersionMinor = j.getProperty("version_minor", juce::var());
    juce::var varVersionRevision = j.getProperty("version_revision", juce::var());

    if (!varVersionMajor.isInt() ||
        !varVersionMinor.isInt() ||
        !varVersionRevision.isInt()) {
        if (reportErrors) {
            displayError("Unexpected data.");
        }
        return false;
    }

    version.major = static_cast<int> (varVersionMajor);
    version.minor = static_cast<int> (varVersionMinor);
    version.revision = static_cast<int> (varVersionRevision);

    versionStr.clear();
    juce::var varVersionText = j.getProperty("version", juce::var());
    if (varVersionText.isString()) {
        versionStr = varVersionText.toString().toStdString();
    }

    return true;
}

bool UpdateChecker::getHTTPResponseBody(const std::string& url, std::string& responseBody, int timeoutMs)
{
    juce::URL jUrl{ url };
    auto isOpt = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress).withConnectionTimeoutMs(timeoutMs);
    auto is = jUrl.createInputStream(isOpt);
    if (is != nullptr)
    {
        juce::MemoryBlock memoryBlock;
        is->readIntoMemoryBlock(memoryBlock);
        responseBody = memoryBlock.toString().toStdString();
        return true;
    }
    return false;
}

void UpdateChecker::displayError(const std::string& errorText)
{
    std::string text{ "An error occurred whilst checking for updates:\n\n" };
    text += errorText;
    displayMessageBox(messageBoxTitles, text, MB_ICONEXCLAMATION);
}

void UpdateChecker::displayUpdateAvailable(const std::string& versionText, bool instigatedManually)
{
    std::string text;
    if (versionText.empty()) {
        text = "A new version of the EAR Production Suite is now available.";
    }
    else {
        text = "EAR Production Suite " + versionText + " is now available.";
    }
    text += "\n\nDownload from https://ear-production-suite.ebu.io/";
    if (!instigatedManually) {
        text += "\n\nNo further notifications will appear for this version. You can disable all future notifications through the Extensions menu.";
    }
    displayMessageBox(messageBoxTitles, text, MB_ICONINFORMATION);
}

void UpdateChecker::displayUpdateUnavailable()
{
    displayMessageBox(messageBoxTitles, "No updates are currently available.", MB_ICONINFORMATION);
}

void UpdateChecker::displayMessageBox(const std::string& title, const std::string& text, long winIcon)
{
    // Windows version of Reaper locks up if you try show a message box during splash
    NativeMessageBox::splashCompatibleMessage(title.c_str(), text.c_str(), nullptr, winIcon);
}

bool UpdateChecker::loadSettings()
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

bool UpdateChecker::saveSettings()
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

bool UpdateChecker::settingsFileExists()
{
    return settingsFile.existsAsFile();
}
