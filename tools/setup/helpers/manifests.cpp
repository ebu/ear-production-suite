#include "manifests.h"

InstallManifest::InstallManifest()
{
    File setupDirectory = File::getSpecialLocation(File::SpecialLocationType::currentExecutableFile).getParentDirectory();
    XmlDocument xml(setupDirectory.getChildFile("install_list.xml"));
    auto installList = xml.getDocumentElementIfTagMatches("InstallList");

    const std::string vst3DirectorySymbol("[VST3-INSTALL-DIR]");
    const std::string userPluginsDirectorySymbol("[USERPLUGINS-INSTALL-DIR]");

    std::string osTag("NONE");
#ifdef WIN32
    osTag = "Windows";
    // C:\Users\(username)\AppData\Roaming + \REAPER + \UserPlugins
    userPluginsDirectory = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory)
        .getChildFile("REAPER").getChildFile("UserPlugins");
    // C:\Program Files + \Common Files + \VST3
    vst3Directory = File::getSpecialLocation(File::SpecialLocationType::globalApplicationsDirectory)
        .getChildFile("Common Files").getChildFile("VST3");
#elif APPLE
    osTag = "MacOS";
    // ~/Library + /Application Support + /REAPER + /UserPlugins
    userPluginsDirectory = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory)
        .getChildFile("Application Support").getChildFile("REAPER").getChildFile("UserPlugins");
    // ~/Library + /Audio + /Plug-Ins + /VST3
    vst3Directory = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory)
        .getChildFile("Audio").getChildFile("Plug-Ins").getChildFile("VST3");
#endif

    for (auto* installItem : installList->getChildWithTagNameIterator("InstallItem")) {
        auto source = installItem->getStringAttribute("Source");
        if (source.isNotEmpty()) {
            auto sourcePath = setupDirectory.getChildFile(source);

            for (auto* osSpecificItem : installItem->getChildWithTagNameIterator(osTag)) {
                auto destination = osSpecificItem->getStringAttribute("Destination");
                if (destination.isNotEmpty()) {
                    if (destination.startsWith(vst3DirectorySymbol)) {
                        destination = destination.replaceFirstOccurrenceOf(vst3DirectorySymbol, vst3Directory.getFullPathName());
                    }
                    else if (destination.startsWith(userPluginsDirectorySymbol)) {
                        destination = destination.replaceFirstOccurrenceOf(userPluginsDirectorySymbol, userPluginsDirectory.getFullPathName());
                    }
                   
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

String InstallManifest::getVst3Directory()
{
    return vst3Directory.getFullPathName();
}

String InstallManifest::getUserPluginsDirectory()
{
    return userPluginsDirectory.getFullPathName();
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
