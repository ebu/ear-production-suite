#pragma once
#include <string>

class UpdateChecker {
public:
    UpdateChecker();
    ~UpdateChecker();

    bool autoCheckEnabled();

    void doUpdateCheck(bool alwaysShowResult = false, bool failSilently = false);

private:
    const std::string versionJsonUrl{ "http://localhost:4000/version_info.json" };

    enum HTTPResult {
        SUCCESS = 0,
        NO_INTERNET,
        URL_OPEN_FAIL,
        CURL_INIT_FAIL,
        CURL_REQUEST_FAIL
    };

    HTTPResult getHTTPResponseBody(const std::string& url, std::string& responseBody);

    void displayHTTPError(HTTPResult res);
    void displayJSONParseError();
    void displayJSONVariableError();

};