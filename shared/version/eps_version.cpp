#include "eps_version.h"
#include "eps_version_autogen_defines.h"

std::string const& eps::baseVersion() {
    static std::string baseVersion(EPS_VERSION_BASE);
    return baseVersion;
}

std::string const& eps::currentVersion() {
    static std::string currentVersion(EPS_VERSION_CURRENT);
    return currentVersion;
}

bool const eps::versionInfoAvailable() {
    return EPS_VERSION_INFO_AVAILABLE;
}