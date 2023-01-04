#pragma once

#include <WDL/swell/swell.h>

inline std::string bufferToString(char const *buffer) {
    assert(buffer);
    return buffer;
}

#ifdef WIN32
inline std::string bufferToString(wchar_t const* wstr) {
    int count = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    std::vector<char> buffer(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &buffer[0], count, NULL, NULL);
    return bufferToString(buffer.data());
}
#endif
