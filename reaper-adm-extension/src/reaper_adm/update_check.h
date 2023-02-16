#pragma once
#include <string>
#include "reaperhost.h"
#include "juce_modules.h"

class UpdateChecker {
public:
    UpdateChecker();
    ~UpdateChecker();

    bool getAutoCheckEnabled();
    bool setAutoCheckEnabled(bool enabled);

    void doUpdateCheck(bool alwaysShowResult = false, bool failSilently = true, int timeoutMs=3000);

private:
    const std::string versionJsonUrl{ "http://localhost:4000/version_info.json" }; //TODO!!!!
    const std::string messageBoxTitles{ "EAR Production Suite Update" };

    bool getHTTPResponseBody(const std::string& url, std::string& responseBody, int timeoutMs);

    void displayHTTPError();
    void displayJSONParseError();
    void displayJSONVariableError();
    void displayError(const std::string& errorText);
    void displayUpdateAvailable(const std::string& versionText);
    void displayUpdateUnavailable();
    void displayMessageBox(const std::string& title, const std::string& text, long winIcon);

    juce::File settingsFile;
    bool loadSettings();
    bool saveSettings();
    bool settingsFileExists();

    bool settingAutoCheckEnabled{ false };
    int settingLastReportedVersionMajor{ 0 };
    int settingLastReportedVersionMinor{ 0 };
    int settingLastReportedVersionRevision{ 0 };

};
