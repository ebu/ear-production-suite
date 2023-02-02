#include "manifests.h"

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

    void replaceDirectorySymbols(juce::String& path) {
        auto vst3Directory = getVst3Directory();
        auto userPluginsDirectory = getUserPluginsDirectory();
        const String vst3DirectorySymbol("[VST3-INSTALL-DIR]");
        const String userPluginsDirectorySymbol("[USERPLUGINS-INSTALL-DIR]");

        if (path.startsWith(vst3DirectorySymbol)) {
            path = path.replaceFirstOccurrenceOf(vst3DirectorySymbol, vst3Directory.getFullPathName());
        }
        else if (path.startsWith(userPluginsDirectorySymbol)) {
            path = path.replaceFirstOccurrenceOf(userPluginsDirectorySymbol, userPluginsDirectory.getFullPathName());
        }
    }
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
