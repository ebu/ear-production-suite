#include "update_check.h"
#include <version/eps_version.h>
#include <iostream>
#include <vector>
#include <helper/resource_paths_juce-file.hpp>

#ifdef WIN32
#include "win_nonblock_msg.h"
#endif

UpdateChecker::UpdateChecker()
{
    settingsFile = ResourcePaths::getSettingsDirectory().getChildFile("UpdateCheck.settings");
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

bool UpdateChecker::setAutoCheckEnabled(bool enabled)
{
    settingAutoCheckEnabled = enabled;
    return saveSettings();
}

void UpdateChecker::doUpdateCheck(bool alwaysShowResult, bool failSilently, int timeoutMs)
{
    std::string body;
    auto getSuccess = getHTTPResponseBody(versionJsonUrl, body, timeoutMs);
    if (!getSuccess) {
        if (alwaysShowResult || !failSilently) {
            displayHTTPError();
        }
        return;
    }

    juce::var j;
    auto parseResult = juce::JSON::parse(body, j);
    if (parseResult.failed()) {
        if (alwaysShowResult || !failSilently) {
            displayJSONParseError();
        }
        return;
    }

    juce::var varVersionMajor = j.getProperty("version_major", juce::var());
    juce::var varVersionMinor = j.getProperty("version_minor", juce::var());
    juce::var varVersionRevision = j.getProperty("version_revision", juce::var());

    if (!varVersionMajor.isInt() ||
        !varVersionMinor.isInt() ||
        !varVersionRevision.isInt()) {
        if (alwaysShowResult || !failSilently) {
            displayJSONVariableError();
        }
        return;
    }

    int versionMajor = static_cast<int> (varVersionMajor);
    int versionMinor = static_cast<int> (varVersionMinor);
    int versionRevision = static_cast<int> (varVersionRevision);

    bool newAvailable = false;
    if (versionMajor > eps::versionMajor()) {
        newAvailable = true;
    }
    else if (versionMajor == eps::versionMajor()) {
        if (versionMinor > eps::versionMinor()) {
            newAvailable = true;
        }
        else if (versionMinor == eps::versionMinor()) {
            if (versionRevision > eps::versionRevision()) {
                newAvailable = true;
            }
        }
    }

    if (newAvailable) {
        if (alwaysShowResult ||
            settingLastReportedVersionMajor != versionMajor ||
            settingLastReportedVersionMinor != versionMinor ||
            settingLastReportedVersionRevision != versionRevision) {
            // Haven't mentioned this version before or told to always show result
            settingLastReportedVersionMajor = versionMajor;
            settingLastReportedVersionMinor = versionMinor;
            settingLastReportedVersionRevision = versionRevision;
            saveSettings();
            std::string versionText;
            juce::var varVersionText = j.getProperty("version", juce::var());
            if (varVersionText.isString()) {
                versionText = varVersionText.toString().toStdString();
            }
            displayUpdateAvailable(versionText);
        }
    }
    else if (alwaysShowResult) {
        displayUpdateUnavailable();
    }
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

void UpdateChecker::displayHTTPError()
{
    displayError("Failed to get data.");
}

void UpdateChecker::displayJSONParseError()
{
    displayError("Failed to parse data.");
}

void UpdateChecker::displayJSONVariableError()
{
    displayError("Unexpected data.");
}

void UpdateChecker::displayError(const std::string& errorText)
{
    std::string text{ "An error occurred whilst checking for updates:\n\n" };
    text += errorText;
    displayMessageBox(messageBoxTitles, text, MB_ICONEXCLAMATION);
}

void UpdateChecker::displayUpdateAvailable(const std::string& versionText)
{
    std::string text;
    if (versionText.empty()) {
        text = "A new version of the EAR Production Suite is now available.";
    }
    else {
        text = "EAR Production Suite " + versionText + " is now available.";
    }
    text += "\n\nDownload from https://ear-production-suite.ebu.io/";
    text += "\n\nNo further notifications will appear for this version. You can disable all future notifications through the Extensions menu.";
    displayMessageBox(messageBoxTitles, text, MB_ICONINFORMATION);
}

void UpdateChecker::displayUpdateUnavailable()
{
    displayMessageBox(messageBoxTitles, "No updates are currently available.", MB_ICONINFORMATION);
}

void UpdateChecker::displayMessageBox(const std::string& title, const std::string& text, long winIcon)
{
#ifdef WIN32
    // Windows version of Reaper locks up if you try show a message box during splash
    winhelpers::NonBlockingMessageBox(text, title, winIcon);
#else
    MessageBox(nullptr, text.c_str(), title.c_str(), MB_OK);
#endif
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
        settingLastReportedVersionMajor = lastReportedElement->getIntAttribute("VersionMajor", 0);
        settingLastReportedVersionMinor = lastReportedElement->getIntAttribute("VersionMinor", 0);
        settingLastReportedVersionRevision = lastReportedElement->getIntAttribute("VersionRevision", 0);
    }

    auto autoCheckElement = updateCheckElement->getChildByName("AutoCheck");
    if (autoCheckElement) {
        settingAutoCheckEnabled = autoCheckElement->getBoolAttribute("OnStartUp", false);
    }

    return true;
}

bool UpdateChecker::saveSettings()
{
    auto updateCheckElement = new juce::XmlElement("UpdateCheck");

    auto lastReportedElement = new juce::XmlElement("LastReportedVersion");
    lastReportedElement->setAttribute("VersionMajor", settingLastReportedVersionMajor);
    lastReportedElement->setAttribute("VersionMinor", settingLastReportedVersionMinor);
    lastReportedElement->setAttribute("VersionRevision", settingLastReportedVersionRevision);
    updateCheckElement->addChildElement(lastReportedElement);

    auto autoCheckElement = new juce::XmlElement("AutoCheck");
    autoCheckElement->setAttribute("OnStartUp", settingAutoCheckEnabled);
    updateCheckElement->addChildElement(autoCheckElement);

    return updateCheckElement->writeToFile(settingsFile, juce::StringRef{});
}

bool UpdateChecker::settingsFileExists()
{
    return settingsFile.existsAsFile();
}
