#include "update_check.h"
#include <version/eps_version.h>
#include <iostream>
#include <vector>

#ifdef WIN32
#include "win_nonblock_msg.h"
#endif

UpdateChecker::UpdateChecker(std::shared_ptr<admplug::ReaperAPI> reaperApi)
{
    assert(reaperApi);
    api = reaperApi;
}

UpdateChecker::~UpdateChecker()
{
    if (updateCheckThread) {
        // Wait for current instance to complete
        updateCheckThread->join();
    }
}

bool UpdateChecker::autoCheckEnabled()
{
    return true;
}

void UpdateChecker::doUpdateCheck(bool alwaysShowResult, bool failSilently)
{
    if (updateCheckThread) {
        // Wait for current instance to complete
        updateCheckThread->join();
    }
    updateCheckThread.reset();
    updateCheckThread = std::make_unique<std::thread>([this, alwaysShowResult, failSilently] { this->doUpdateCheckTask(alwaysShowResult, failSilently); });
}

void UpdateChecker::doUpdateCheckTask(bool alwaysShowResult, bool failSilently)
{
    std::string body;
    auto getSuccess = getHTTPResponseBody(versionJsonUrl, body);
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
        // TODO: if(we haven't mentioned this version before)
        std::string versionText;
        juce::var varVersionText = j.getProperty("version", juce::var());
        if (varVersionText.isString()) {
            versionText = varVersionText.toString().toStdString();
        }
        displayUpdateAvailable(versionText);
    }
    else if (alwaysShowResult) {
        displayUpdateUnavailable();
    }
}

bool UpdateChecker::getHTTPResponseBody(const std::string& url, std::string& responseBody)
{
    juce::URL jUrl{ url };
    juce::MemoryBlock memoryBlock;
    auto res = jUrl.readEntireBinaryStream(memoryBlock);
    responseBody = memoryBlock.toString().toStdString();
    return res;
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
    api->ShowMessageBox(text.c_str(), title.c_str(), 0);
#endif
}
