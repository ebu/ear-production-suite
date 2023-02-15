#include "update_check.h"
#include <version/eps_version.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <iostream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "Wininet.lib")
#else
#include <curl/curl.h>
#endif

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
    auto res = getHTTPResponseBody(versionJsonUrl, body);
    json j;

    if (res == SUCCESS) {
        try {
            j = json::parse(body);
        }
        catch (...) {
            // Parse failed
            if (!failSilently) {
                displayJSONParseError();
            }
            return;
        }
    }
    else if (!failSilently) {
        displayHTTPError(res);
    }

    if (j.find("version_major") == j.end() ||
        j.find("version_minor") == j.end() ||
        j.find("version_revision") == j.end() ||
        !j["version_major"].is_number_integer() ||
        !j["version_minor"].is_number_integer() ||
        !j["version_revision"].is_number_integer()) {
        if (!failSilently) {
            displayJSONVariableError();
        }
        return;
    }

    int versionMajor = j["version_major"].get<int>();
    int versionMinor = j["version_minor"].get<int>();
    int versionRevision = j["version_revision"].get<int>();

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
        if (j.find("version") != j.end() && j["version"].is_string()) {
            versionText = j["version"].get<std::string>();
        }
        displayUpdateAvailable(versionText);
    }
    else if (alwaysShowResult) {
        displayUpdateUnavailable();
    }
}

UpdateChecker::HTTPResult UpdateChecker::getHTTPResponseBody(const std::string& url, std::string& responseBody)
{
#ifdef _WIN32
    HINTERNET internet = InternetOpenA("EPSUserAgent", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (internet == NULL)
    {
        return NO_INTERNET;
    }

    HINTERNET httpRequest = InternetOpenUrlA(internet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (httpRequest == NULL)
    {
        InternetCloseHandle(internet);
        return URL_OPEN_FAIL;
    }

    char buffer[1024];
    DWORD bytesRead;
    while (InternetReadFile(httpRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0)
    {
        responseBody.append(buffer, bytesRead);
    }

    InternetCloseHandle(httpRequest);
    InternetCloseHandle(internet);
#else
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        return CURL_INIT_FAIL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* buffer, size_t size, size_t nmemb, void* userData) -> size_t {
        std::string& responseBody = *reinterpret_cast<std::string*>(userData);
        responseBody.append(reinterpret_cast<char*>(buffer), size * nmemb);
        return size * nmemb;
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        curl_easy_cleanup(curl);
        return CURL_REQUEST_FAIL;
    }

    curl_easy_cleanup(curl);
#endif
    return SUCCESS;
}

void UpdateChecker::displayHTTPError(HTTPResult res)
{
    switch (res) {
    case NO_INTERNET:
        //Error: Failed to open internet connection.
        displayError("Failed to open internet connection.");
        break;
    case URL_OPEN_FAIL:
        //Error: Failed to open URL.
        displayError("Failed to open URL.");
        break;
    case CURL_INIT_FAIL:
        //Error: Failed to initialize cURL.
        displayError("Failed to initialize cURL.");
        break;
    case CURL_REQUEST_FAIL:
        //Error: Failed to perform cURL request
        displayError("Failed to perform cURL request.");
        break;
    default:
        displayError("(Unknown Error)");
        break;
    }
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
