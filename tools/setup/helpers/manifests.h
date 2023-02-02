#pragma once

#include <string>
#include <vector>
#include "JuceHeader.h"

/*
* ItemType
For bundles, we'll always want to force delete the whole thing on uninstall,
but for other directory copies (e.g Templates dir), we don't want to delete
anything the user has put in there during an uninstall.
This means that for non-bundles, we must track every file we install and add
it to an uninstall list. During uninstall, delete the files individually and
then only delete the directory if it is subsequently empty.
*/

enum class ItemType {
    NONE,
    FILE,
    DIRECTORY,
    BUNDLE
};

struct CopyItem {
    juce::File from;
    juce::File to;
    ItemType itemType;
    bool fromValid;
};

class InstallManifest {
public:
    InstallManifest();
    ~InstallManifest();

private:
    std::vector<CopyItem> installItems;
};