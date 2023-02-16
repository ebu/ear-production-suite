#pragma once
#include <string>
#include <thread>
#include <memory>
#include "reaperhost.h"
#include "juce_modules.h"

class UpdateChecker {
public:
    UpdateChecker(std::shared_ptr<admplug::ReaperAPI> reaperApi);
    ~UpdateChecker();

    bool autoCheckEnabled();

    void doUpdateCheck(bool alwaysShowResult = false, bool failSilently = true);

private:
    const std::string versionJsonUrl{ "http://localhost:4000/version_info.json" }; //TODO!!!!
    const std::string messageBoxTitles{ "EAR Production Suite Update" };
    std::shared_ptr<admplug::ReaperAPI> api;

    void doUpdateCheckTask(bool alwaysShowResult, bool failSilently);
    std::unique_ptr<std::thread> updateCheckThread;
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