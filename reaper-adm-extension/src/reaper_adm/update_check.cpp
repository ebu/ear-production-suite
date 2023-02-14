#include "update_check.h"

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
    // TODO: We should probably do this in a thread to prevent holding up load of REAPER
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

    // TODO compare

}

UpdateChecker::HTTPResult UpdateChecker::getHTTPResponseBody(const std::string& url, std::string& responseBody)
{
#ifdef _WIN32
    HINTERNET internet = InternetOpenA("MyUserAgent", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (internet == NULL)
    {
        std::cerr << "Error: Failed to open internet connection." << std::endl;
        return NO_INTERNET;
    }

    HINTERNET httpRequest = InternetOpenUrlA(internet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (httpRequest == NULL)
    {
        std::cerr << "Error: Failed to open URL." << std::endl;
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
        std::cerr << "Error: Failed to initialize cURL." << std::endl;
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
        std::cerr << "Error: Failed to perform cURL request: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return CURL_REQUEST_FAIL;
    }

    curl_easy_cleanup(curl);
#endif
    return SUCCESS;
}

void UpdateChecker::displayHTTPError(HTTPResult res)
{
    // TODO
    switch (res) {
    case NO_INTERNET:
        break;
    case URL_OPEN_FAIL:
        break;
    case CURL_INIT_FAIL:
        break;
    case CURL_REQUEST_FAIL:
        break;
    default:
        break;
    }
}

void UpdateChecker::displayJSONParseError()
{
    // TODO
}

void UpdateChecker::displayJSONVariableError()
{
    // TODO
}
