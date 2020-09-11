#pragma once
#include <string>
namespace admplug {
namespace file {
bool fileExists(std::string path);

constexpr const char* dirChar() {
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}

}
}
