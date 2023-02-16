#include "update_check.h"
#include <version/eps_version.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#ifdef WIN32
#include "win_nonblock_msg.h"
#endif

UpdateChecker::UpdateChecker()
{
}

UpdateChecker::~UpdateChecker()
{
}

bool UpdateChecker::autoCheckEnabled()
{
    return true;
}

void UpdateChecker::doUpdateCheck(bool alwaysShowResult, bool failSilently)
{
    completed = false;
    completionAction = nullptr;
    std::thread updateCheckThread([this, alwaysShowResult, failSilently] {
        this->doUpdateCheckTask(alwaysShowResult, failSilently);
        this->completed = true;
    });
    updateCheckThread.detach();

    auto start = std::chrono::high_resolution_clock::now();
    while(!completed) {
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        if (elapsed > std::chrono::milliseconds(1000)) {
            break;
        }
    }
    if (completed) {
        if (completionAction) {
            completionAction();
        }
    }
    else if (alwaysShowResult || !failSilently) {
        displayError("Timed out.");
    }
}

void UpdateChecker::doUpdateCheckTask(bool alwaysShowResult, bool failSilently)
{
    std::string body;
    auto getSuccess = getHTTPResponseBody(versionJsonUrl, body);
    if (!getSuccess) {
        if (alwaysShowResult || !failSilently) {
            completionAction = [this]() { this->displayHTTPError(); };
        }
        return;
    }

    juce::var j;
    auto parseResult = juce::JSON::parse(body, j);
    if (parseResult.failed()) {
        if (alwaysShowResult || !failSilently) {
            completionAction = [this]() { this->displayJSONParseError(); };
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
            completionAction = [this]() { this->displayJSONVariableError(); };
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
        completionAction = [this, versionText]() { this->displayUpdateAvailable(versionText); };
    }
    else if (alwaysShowResult) {
        completionAction = [this]() { this->displayUpdateUnavailable(); };
    }
}

bool UpdateChecker::getHTTPResponseBody(const std::string& url, std::string& responseBody)
{
    juce::URL jUrl{ url };
    //auto res = jUrl.readEntireBinaryStream(memoryBlock);
    auto isOpt = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress).withConnectionTimeoutMs(1000);
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
