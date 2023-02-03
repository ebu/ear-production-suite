#include "manifests.h"
#include <algorithm>

namespace {
    juce::String getOsStr() {
#ifdef WIN32
        return "Windows";
#elif APPLE
        return "MacOS";
#else
        throw std::runtime_error("Unsupported OS");
#endif
    }

    juce::File getVst3Directory() {
#ifdef WIN32
        // C:\Program Files + \Common Files + \VST3
        return File::getSpecialLocation(File::SpecialLocationType::globalApplicationsDirectory)
            .getChildFile("Common Files").getChildFile("VST3");
#elif APPLE
        // ~/Library + /Audio + /Plug-Ins + /VST3
        return File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory)
            .getChildFile("Audio").getChildFile("Plug-Ins").getChildFile("VST3");
#else
        throw std::runtime_error("Unsupported OS");
#endif
    }

    juce::File getUserPluginsDirectory() {
#ifdef WIN32
        // C:\Users\(username)\AppData\Roaming + \REAPER + \UserPlugins
        return File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory)
            .getChildFile("REAPER").getChildFile("UserPlugins");
#elif APPLE
        // ~/Library + /Application Support + /REAPER + /UserPlugins
        return File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory)
            .getChildFile("Application Support").getChildFile("REAPER").getChildFile("UserPlugins");
#else
        throw std::runtime_error("Unsupported OS");
#endif
    }

    juce::File getWinReaperProgramDirectory() {
#ifdef WIN32
        // Default install dir on Windows, which might not be correct if customised
        // C:\Program Files + \REAPER (x64)
        auto path = File::getSpecialLocation(File::SpecialLocationType::globalApplicationsDirectory).getChildFile("REAPER (x64)");
        // Windows Registry might provide more accurate information for customised installs
        const String key("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\REAPER\\InstallLocation");
        if (WindowsRegistry::valueExists(key)) {
            auto val = WindowsRegistry::getValue(key);
            path = File(val);
        }
        return path;
#else
        throw std::runtime_error("Unsupported OS");
#endif
    }

    void replaceDirectorySymbols(juce::String& path) {
        const String vst3DirectorySymbol("[VST3-INSTALL-DIR]");
        const String userPluginsDirectorySymbol("[USERPLUGINS-INSTALL-DIR]");
        const String winReaperProgramDirectorySymbol("[REAPER-PROGRAM-DIR-WIN]");

        if (path.startsWith(vst3DirectorySymbol)) {
            path = path.replaceFirstOccurrenceOf(vst3DirectorySymbol, getVst3Directory().getFullPathName());
        }
        else if (path.startsWith(userPluginsDirectorySymbol)) {
            path = path.replaceFirstOccurrenceOf(userPluginsDirectorySymbol, getUserPluginsDirectory().getFullPathName());
        }
        else if (path.startsWith(winReaperProgramDirectorySymbol)) {
            path = path.replaceFirstOccurrenceOf(winReaperProgramDirectorySymbol, getWinReaperProgramDirectory().getFullPathName());
        }
    }
}

String Locations::getVst3Directory()
{
    return ::getVst3Directory().getFullPathName();
}

String Locations::getUserPluginsDirectory()
{
    return ::getUserPluginsDirectory().getFullPathName();
}



// ==========================================================================================================================


InstallManifest::InstallManifest()
{
    File setupDirectory = File::getSpecialLocation(File::SpecialLocationType::currentExecutableFile).getParentDirectory();
    XmlDocument xml(setupDirectory.getChildFile("install_list.xml"));
    auto installList = xml.getDocumentElementIfTagMatches("InstallList");

    for (auto* installItem : installList->getChildWithTagNameIterator("InstallItem")) {
        auto source = installItem->getStringAttribute("Source");
        if (source.isNotEmpty()) {
            auto sourcePath = setupDirectory.getChildFile(source);

            for (auto* osSpecificItem : installItem->getChildWithTagNameIterator(getOsStr())) {
                auto destination = osSpecificItem->getStringAttribute("Destination");
                if (destination.isNotEmpty()) {
                    replaceDirectorySymbols(destination);
                   
                    installItems.push_back({
                        sourcePath,
                        File(destination),
                        sourcePath.exists()
                        });
                }
            }
        }
    }
}

InstallManifest::~InstallManifest()
{
}

std::vector<String> InstallManifest::getInvalidSources()
{
    std::vector<String> paths;
    for (auto const& item : installItems) {
        if (!item.sourceValid) {
            paths.push_back(item.source.getFullPathName());
        }
    }
    return paths;
}

void InstallManifest::doInstall()
{
    installLog.clear();
    installErrors.clear();
    for (auto const& item : installItems) {
        if (!item.sourceValid) {
            installErrors.push_back("Not Found:\n    " + item.source.getFullPathName());
            installLog.push_back("Copy: " + item.source.getFullPathName() + " --> " + item.destination.getFullPathName());
            installLog.push_back("!!! Failed last action");
        }
        else {
            if (item.source.isDirectory()) {
                installLog.push_back("Copy Directory: " + item.source.getFullPathName() + " --> " + item.destination.getFullPathName());
                if (!item.source.copyDirectoryTo(item.destination)) {
                    installErrors.push_back("Copy Directory Failed:\n    From: " + item.source.getFullPathName() + "\n    To: " + item.destination.getFullPathName());
                    installLog.push_back("!!! Failed last action");
                }
            }
            else {
                auto parentDir = item.destination.getParentDirectory();
                if (!parentDir.exists()) {
                    installLog.push_back("Create Directories: " + parentDir.getFullPathName());
                    auto res = parentDir.createDirectory();
                    if (res.failed()) {
                        installErrors.push_back("Create Directories Failed - " + res.getErrorMessage() + ":\n    " + parentDir.getFullPathName());
                        installLog.push_back("!!! Failed last action");
                    }
                }
                installLog.push_back("Copy File: " + item.source.getFullPathName() + " --> " + item.destination.getFullPathName());
                if (!item.source.copyFileTo(item.destination)) {
                    installErrors.push_back("Copy File Failed:\n    From: " + item.source.getFullPathName() + "\n    To: " + item.destination.getFullPathName());
                    installLog.push_back("!!! Failed last action");
                }
            }
        }
    }
}

std::vector<String> InstallManifest::getInstallErrors()
{
    return installErrors;
}


// ==========================================================================================================================


UninstallManifest::UninstallManifest()
{
    File setupDirectory = File::getSpecialLocation(File::SpecialLocationType::currentExecutableFile).getParentDirectory();
    XmlDocument xml(setupDirectory.getChildFile("uninstall_list.xml"));
    auto uninstallList = xml.getDocumentElementIfTagMatches("UninstallList");

    populateVectorsFromElement(uninstallList.get());
    for (auto* osSpecificSection : uninstallList->getChildWithTagNameIterator(getOsStr() + "Only")) {
        populateVectorsFromElement(osSpecificSection);
    }
}

UninstallManifest::~UninstallManifest()
{
}

std::vector<String> UninstallManifest::getFoundFiles()
{
    std::vector<String> paths;
    for (auto const& item : uninstallDirectories) {
        paths.push_back(item.path.getFullPathName());
    }
    for (auto const& item : uninstallFiles) {
        paths.push_back(item.path.getFullPathName());
    }
    return paths;
}

void UninstallManifest::doUninstall()
{
    uninstallLog.clear();
    uninstallErrors.clear();
    // Work through files first
    for (auto const& item : uninstallFiles) {
        uninstallLog.push_back("Deleting File: " + item.path.getFullPathName());
        if (!item.path.deleteFile()) {
            uninstallErrors.push_back("Delete File Failed:\n    " + item.path.getFullPathName());
            uninstallLog.push_back("!!! Failed last action");
        }
    }
    // Now work through directories.
    // We need to delete deepest first, otherwise subdirectories might cause failure if we attempt to delete the parent first.
    sortDirectoriesDeepestFirst();
    for (auto const& item : uninstallDirectories) {
        if (item.deleteAllContents) {
            uninstallLog.push_back("Deleting Directory Recursively: " + item.path.getFullPathName());
            if (!item.path.deleteRecursively()) {
                uninstallErrors.push_back("Delete Directory Recursively Failed:\n    " + item.path.getFullPathName());
                uninstallLog.push_back("!!! Failed last action");
            }
        }
        else {
            uninstallLog.push_back("Deleting Directory: " + item.path.getFullPathName());
            if (item.path.getNumberOfChildFiles(File::TypesOfFileToFind::findFilesAndDirectories) > 0) {
                uninstallErrors.push_back("Delete Directory Failed - not empty:\n    " + item.path.getFullPathName());
                uninstallLog.push_back("!!! Failed last action");
            } else if(!item.path.deleteFile()) {
                uninstallErrors.push_back("Delete Directory Failed:\n    " + item.path.getFullPathName());
                uninstallLog.push_back("!!! Failed last action");
            }
        }
    }
}

std::vector<String> UninstallManifest::getUninstallErrors()
{
    return uninstallErrors;
}

void UninstallManifest::populateVectorsFromElement(juce::XmlElement* elm)
{
    if (!elm) return;

    for (auto* item : elm->getChildWithTagNameIterator("File")) {
        auto path = pathFromElement(item);
        if (path.has_value() && path->existsAsFile()) {
            uninstallFiles.push_back({ path.value() });
        }
    }

    for (auto* item : elm->getChildWithTagNameIterator("Directory")) {
        auto path = pathFromElement(item);
        if (path.has_value() && path->exists() && path->isDirectory()) {
            uninstallDirectories.push_back({ path.value(), false }); // Don't delete all contents - only delete if empty in case user has put something else in there
        }
    }

    for (auto* item : elm->getChildWithTagNameIterator("Bundle")) {
        auto path = pathFromElement(item);
        if (path.has_value() && path->exists() && path->isDirectory()) {
            uninstallDirectories.push_back({ path.value(), true }); // It's a bundle so delete all contents as well });
        }
    }
}

std::optional<juce::File> UninstallManifest::pathFromElement(juce::XmlElement* elm)
{
    auto pathStr = elm->getStringAttribute("Path");
    if (pathStr.isNotEmpty()) {
        replaceDirectorySymbols(pathStr);
        return File(pathStr);
    }
    return std::optional<juce::File>();
}

void UninstallManifest::sortDirectoriesDeepestFirst()
{
    std::sort(uninstallDirectories.begin(), uninstallDirectories.end(),
        [](const UninstallDirectory& a, const UninstallDirectory& b)
    {
        auto sep = File::getSeparatorString();
        auto aPath = a.path;
        int aDepth = 0;
        while (!aPath.isRoot()) {
            aPath = aPath.getParentDirectory();
            aDepth++;
        }
        auto bPath = b.path;
        int bDepth = 0;
        while (!bPath.isRoot()) {
            bPath = bPath.getParentDirectory();
            bDepth++;
        }
        return aDepth > bDepth;
    });
}
