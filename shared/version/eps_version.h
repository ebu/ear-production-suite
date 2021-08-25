// epsBaseVersion is the numeric-only version string.
/// It consists of 3 parts (or 4 if "patch" portion is available and is purely numeric)
/// [major].[minor].[revision] (or [major].[minor].[revision].[patch])
extern const char epsBaseVersion[];

// epsCurrentVersion is the complete, descriptive version string.
/// It includes the last release version, including any alphanumeric information in the "patch" portion
/// It also includes a partial commit hash, and a dirty flag if necessary
extern const char epsCurrentVersion[];

// epsVersionInfoAvailable states whether version information was successfully obtained from Git.
extern bool epsVersionInfoAvailable;