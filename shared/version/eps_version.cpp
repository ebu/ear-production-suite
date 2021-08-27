#include "eps_version.h"

std::string const& eps::baseVersion() {
    static std::string baseVersion("0.6.0");
    return baseVersion;
}

std::string const& eps::currentVersion() {
    static std::string currentVersion("0.6.0.EPS-beta-99-g05c7-dirty");
    return currentVersion;
}

bool const eps::versionInfoAvailable() {
    return true;
}
