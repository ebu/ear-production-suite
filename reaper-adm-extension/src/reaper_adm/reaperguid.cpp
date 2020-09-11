#include <algorithm>
#include <stdexcept>
#include <cctype>

#include "reaperguid.h"

using namespace admplug;

ReaperGUID::ReaperGUID(GUID* origGuidPtr)
{
    if(!origGuidPtr) {
        throw std::runtime_error("ReaperGUID: Cannot construct from null guid pointer");
    }
    memcpy(&guid, origGuidPtr, sizeof(GUID));
}

ReaperGUID::ReaperGUID(std::string guidStr)
{
    if (!isValid(guidStr)) {
        throw std::runtime_error("String passed to ReaperGUID was not a valid GUID");
    }
    assignFrom(guidStr);
}

GUID * ReaperGUID::get()
{
    return &guid;
}

std::string removeCurlyBraces(std::string guidStr) {
    guidStr.erase(
        std::remove_if(guidStr.begin(), guidStr.end(),
                       [](char const c) {
                           return c == '{' || c == '}';
                       }),
        guidStr.end());
    return guidStr;
}

std::string removeSeparators(std::string guidStr) {
    guidStr.erase(
        std::remove_if(guidStr.begin(), guidStr.end(),
                       [](char const c) {
                           return c == '-';
                       }),
        guidStr.end());
    return guidStr;
}

void ReaperGUID::assignFrom(std::string guidStr)
{
    guidStr = removeCurlyBraces(guidStr);
    guidStr = removeSeparators(guidStr);
    guid.Data1 = static_cast<unsigned int>(std::stoul(guidStr.substr(0, 8), nullptr, 16));
    guid.Data2 = static_cast<unsigned short>(std::stoul(guidStr.substr(8, 4), nullptr, 16));
    guid.Data3 = static_cast<unsigned short>(std::stoul(guidStr.substr(12, 4), nullptr, 16));
    guid.Data4[0] = static_cast<unsigned char>(std::stoul(guidStr.substr(16, 2), nullptr, 16));
    guid.Data4[1] = static_cast<unsigned char>(std::stoul(guidStr.substr(18, 2), nullptr, 16));
    guid.Data4[2] = static_cast<unsigned char>(std::stoul(guidStr.substr(20, 2), nullptr, 16));
    guid.Data4[3] = static_cast<unsigned char>(std::stoul(guidStr.substr(22, 2), nullptr, 16));
    guid.Data4[4] = static_cast<unsigned char>(std::stoul(guidStr.substr(24, 2), nullptr, 16));
    guid.Data4[5] = static_cast<unsigned char>(std::stoul(guidStr.substr(26, 2), nullptr, 16));
    guid.Data4[6] = static_cast<unsigned char>(std::stoul(guidStr.substr(28, 2), nullptr, 16));
    guid.Data4[7] = static_cast<unsigned char>(std::stoul(guidStr.substr(30, 2), nullptr, 16));
}

bool ReaperGUID::isValid(std::string guidStr) {
    guidStr = removeCurlyBraces(guidStr);
    if (guidStr.length() != 36) return false;
    if (guidStr.at(8) != '-') return false;
    if (guidStr.at(13) != '-') return false;
    if (guidStr.at(18) != '-') return false;
    if (guidStr.at(23) != '-') return false;
    guidStr = removeSeparators(guidStr);
    if (guidStr.length() != 32) return false;
    for (auto chr : guidStr) {
        if(std::isxdigit(chr) == 0) return false;
    }
    return true;
}

