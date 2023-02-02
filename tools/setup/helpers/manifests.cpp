#include "manifests.h"

InstallManifest::InstallManifest()
{
    File pathToExecutable = File::getSpecialLocation(File::SpecialLocationType::currentExecutableFile).getParentDirectory();
    XmlDocument xml(pathToExecutable.getChildFile("install_list.xml"));
    auto installList = xml.getDocumentElementIfTagMatches("InstallList");

    const std::string vst3DirectorySymbol("[VST3-INSTALL-DIR]");
    const std::string userPluginsDirectorySymbol("[USERPLUGINS-INSTALL-DIR]");

    std::string osTag("NONE");
    File installFilesDirectory = pathToExecutable.getChildFile("InstallFiles");
    File vst3Directory;
    File userPluginsDirectory;
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
            auto sourcePath = installFilesDirectory.getChildFile(source);

            for (auto* osSpecificItem : installItem->getChildWithTagNameIterator(osTag)) {

                bool sourceValid = false;
                ItemType itemType = ItemType::FILE;
                // Test IsBundle first - a bundle is also a directory, but a directory not necessarily a bundle
                if (osSpecificItem->getBoolAttribute("IsBundle", false)) {
                    itemType = ItemType::BUNDLE;
                    if (sourcePath.exists() && sourcePath.isDirectory()) sourceValid = true;
                }
                else if (osSpecificItem->getBoolAttribute("IsDirectory", false)) {
                    itemType = ItemType::DIRECTORY;
                    if (sourcePath.exists() && sourcePath.isDirectory()) sourceValid = true;
                }
                else {
                    if (sourcePath.existsAsFile()) sourceValid = true;
                }

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
                        itemType,
                        sourceValid
                        });
                }
            }
        }
    }
}

InstallManifest::~InstallManifest()
{
}
