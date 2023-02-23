#pragma once

namespace ResourcePaths {
    inline juce::File getVst3Directory() {
#ifdef WIN32
        // C:\Program Files + \Common Files + \VST3
        return juce::File::getSpecialLocation(juce::File::SpecialLocationType::globalApplicationsDirectory)
            .getChildFile("Common Files").getChildFile("VST3");
#elif __APPLE__
        // ~/Library + /Audio + /Plug-Ins + /VST3
        return juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory)
            .getChildFile("Audio").getChildFile("Plug-Ins").getChildFile("VST3");
#else
        throw std::runtime_error("Unsupported OS");
#endif
    }

    inline juce::File getUserPluginsDirectory() {
#ifdef WIN32
        // C:\Users\(username)\AppData\Roaming + \REAPER + \UserPlugins
        return juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory)
            .getChildFile("REAPER").getChildFile("UserPlugins");
#elif __APPLE__
        // ~/Library + /Application Support + /REAPER + /UserPlugins
        return juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory)
            .getChildFile("Application Support").getChildFile("REAPER").getChildFile("UserPlugins");
#else
        throw std::runtime_error("Unsupported OS");
#endif
    }

    inline juce::File getExtrasDirectory() {
#ifdef WIN32
        // C:\Users\(username)\AppData\Roaming + \EAR Production Suite
        return juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory)
            .getChildFile("EAR Production Suite");
#elif __APPLE__
        // ~/Library + /Application Support + /EAR Production Suite
        return juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory)
            .getChildFile("Application Support").getChildFile("EAR Production Suite");
#elif __linux__
        // ~/.config/EAR Production Suite
        return juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory)
            .getChildFile("EAR Production Suite");
#else
        throw std::runtime_error("Unsupported OS");
#endif
    }

    inline juce::File getWinReaperProgramDirectory() {
#ifdef WIN32
        // Default install dir on Windows, which might not be correct if customised
        // C:\Program Files + \REAPER (x64)
        auto path = juce::File::getSpecialLocation(juce::File::SpecialLocationType::globalApplicationsDirectory).getChildFile("REAPER (x64)");
        // Windows Registry might provide more accurate information for customised installs
        const juce::String key("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\REAPER\\InstallLocation");
        if (juce::WindowsRegistry::valueExists(key)) {
            auto val = juce::WindowsRegistry::getValue(key);
            path = juce::File(val);
        }
        return path;
#else
        throw std::runtime_error("Unsupported OS");
#endif
    }

    inline juce::File getLogsDirectory(bool createIfMissing = false) {
        // C:\Users\(username)\AppData\Roaming + \EAR Production Suite + \logs
        // ~/Library + /Application Support + /EAR Production Suite + /logs
        // ~/.config + /EAR Production Suite + /logs
        auto p = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory);
#ifdef __APPLE__
        p = p.getChildFile("Application Support");
#endif
        p = p.getChildFile("EAR Production Suite").getChildFile("logs");
        if (createIfMissing && !p.exists()) {
            p.createDirectory();
        }
        return p;
    }

    inline juce::File getSettingsDirectory(bool createIfMissing = false) {
        // C:\Users\(username)\AppData\Roaming + \EAR Production Suite + \settings
        // ~/Library + /Application Support + /EAR Production Suite + /settings
        // ~/.config + /EAR Production Suite + /settings
        auto p = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory);
#ifdef __APPLE__
        p = p.getChildFile("Application Support");
#endif
        p = p.getChildFile("EAR Production Suite").getChildFile("settings");
        if (createIfMissing && !p.exists()) {
            p.createDirectory();
        }
        return p;
    }
}