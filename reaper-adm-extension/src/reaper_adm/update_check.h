#pragma once
#include <string>
#include <AppConfig.h>
#include <juce_core/juce_core.h>
#include <helper/version.hpp>
#include <update_check_settings_file.h>

class UpdateChecker {
public:
    UpdateChecker();
    ~UpdateChecker();

    bool getAutoCheckEnabled();
    bool setAutoCheckEnabled(bool enabled, bool displayConfirmation=false);

    void doUpdateCheck(bool manualCheck=false, int timeoutMs=3000);

private:
    const std::string versionJsonUrl{ "https://ear-production-suite.ebu.io/version_info.json" };
    const std::string messageBoxTitles{ "EAR Production Suite Update" };

    bool getRemoteVersion(bool reportErrors, int timeoutMs, Version& version, std::string& versionStr);
    bool getHTTPResponseBody(const std::string& url, std::string& responseBody, int timeoutMs);

    void displayError(const std::string& errorText);
    void displayUpdateAvailable(const std::string& versionText, bool instigatedManually);
    void displayUpdateUnavailable();
    void displayMessageBox(const std::string& title, const std::string& text, long winIcon);

    UpdateCheckerSettingsFile settingsFile;
    Version currentVersion;

};
