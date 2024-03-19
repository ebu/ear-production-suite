#pragma once
#include <string>

namespace eps {

    // baseVersion is the numeric-only version string.
    /// It consists of 3 parts (or 4 if "tweak" portion is available and is purely numeric)
    /// [major].[minor].[patch(revision)] (or [major].[minor].[patch(revision)][tweak])
    extern std::string const& baseVersion();

    // currentVersion is the complete, descriptive version string.
    /// It includes the last release version, including any alphanumeric information in the "tweak" portion
    /// It also includes a partial commit hash, and a dirty flag if necessary
    extern std::string const& currentVersion();
	
	// Version fragments as individual ints
	extern int const versionMajor();
	extern int const versionMinor();
	extern int const versionRevision();

    // versionInfoAvailable states whether version information was successfully obtained from Git.
    extern bool const versionInfoAvailable();

}