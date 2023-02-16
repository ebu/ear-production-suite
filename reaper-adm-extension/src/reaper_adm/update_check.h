#pragma once
#include <string>
#include <atomic>
#include <functional>
#include "reaperhost.h"
#include "juce_modules.h"

class UpdateChecker {
public:
    UpdateChecker();
    ~UpdateChecker();

    bool autoCheckEnabled();

    void doUpdateCheck(bool alwaysShowResult = false, bool failSilently = true);

private:
    const std::string versionJsonUrl{ "http://localhost:4000/version_info.json" }; //TODO!!!!
    const std::string messageBoxTitles{ "EAR Production Suite Update" };

    void doUpdateCheckTask(bool alwaysShowResult, bool failSilently);
    std::atomic<bool> completed;
    std::function<void()> completionAction;

    bool getHTTPResponseBody(const std::string& url, std::string& responseBody);

    void displayHTTPError();
    void displayJSONParseError();
    void displayJSONVariableError();
    void displayError(const std::string& errorText);
    void displayUpdateAvailable(const std::string& versionText);
    void displayUpdateUnavailable();
    void displayMessageBox(const std::string& title, const std::string& text, long winIcon);

};
