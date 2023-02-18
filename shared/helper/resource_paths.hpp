#pragma once

#include <string>

#ifdef WIN32
#include <filesystem>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace ResourcePaths {

    inline bool directoryExists(const std::string& directory) {
#ifdef _WIN32
        std::filesystem::path p(directory);
        if (std::filesystem::exists(p) && std::filesystem::is_directory(p)) {
            return true;
        }
        return false;
#else
        // Apple - filesystem lib not supported until MacOS 10.15 - use stat
        // Fine for linux too
        struct stat info;
        if (stat(directory.c_str(), &info) != 0)
            return false;
        return (info.st_mode & S_IFDIR);
#endif
    }

    inline bool openDirectory(const std::string& directoryPath) {
        if (directoryExists(directoryPath)) {
#ifdef _WIN32
            std::string command = "explorer \"" + directoryPath + "\"";
            // Don't use system() for win - blocks and shows cmd prompt
            auto res = WinExec(command.c_str(), SW_NORMAL);
            // If WinExec succeeds, the return value is greater than 31.
            return res > 31;
#elif __APPLE__
            std::string command = "open \"" + directoryPath + "\"";
#else
            std::string command = "xdg-open \"" + directoryPath + "\"";
#endif
            return std::system(command.c_str()) == 0;
        }
        return false;
    }

    inline std::string getToolsPath(ReaperAPI const& api) {
        std::string path(api.GetResourcePath());
#ifdef _WIN32
        return path + "\\UserPlugins\\EAR Production Suite extras";
#else
        // Apple and Linux
        return path + "/UserPlugins/EAR Production Suite extras";
#endif
    }
}