#include "manifests.h"
#include "JuceHeader.h"

InstallManifest::InstallManifest()
{
    File pathToExecutable = File::getSpecialLocation(File::SpecialLocationType::currentExecutableFile).getParentDirectory();
    XmlDocument xml(pathToExecutable.getChildFile("install_list.xml"));
    auto installList = xml.getDocumentElementIfTagMatches("InstallList");

    std::string osTag("NONE");
#ifdef WIN32
    osTag = "Windows";
#elif APPLE
    osTag = "MacOS";
#endif

    for (auto* installItem : installList->getChildWithTagNameIterator("InstallItem")) {
        auto source = installItem->getStringAttribute("Source");
        if (source.isNotEmpty()) {
            for (auto* osSpecificItem : installItem->getChildWithTagNameIterator(osTag)) {
                auto destination = osSpecificItem->getStringAttribute("Destination");
                if (destination.isNotEmpty()) {
                    // Test IsBundle first - a bundle is also a directory, but a directory not necessarily a bundle
                    if (osSpecificItem->getBoolAttribute("IsBundle", false)) {
                        installItems.push_back({
                            source.toStdString(),
                            destination.toStdString(),
                            ItemType::BUNDLE
                            });
                    } else if(osSpecificItem->getBoolAttribute("IsDirectory", false)) {
                        installItems.push_back({
                            source.toStdString(),
                            destination.toStdString(),
                            ItemType::DIRECTORY
                            });
                    }
                    else {
                        installItems.push_back({
                            source.toStdString(),
                            destination.toStdString(),
                            ItemType::FILE
                            });
                    }
                }
            }
        }
    }
}

InstallManifest::~InstallManifest()
{
}
