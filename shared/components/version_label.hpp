#pragma once

#include "JuceHeader.h"

#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"
#include <version/eps_version.h>

inline void configureVersionLabel(juce::Label& versionLabel) {
    versionLabel.setFont(ear::plugin::ui::EarFonts::Version);
    versionLabel.setColour(juce::Label::textColourId, ear::plugin::ui::EarColours::Version);
    versionLabel.setJustificationType(juce::Justification::right);
    versionLabel.setEditable(false);
    if(eps::versionInfoAvailable()) {
        versionLabel.setText(juce::String("v") + eps::currentVersion(), juce::NotificationType::dontSendNotification);
    } else {
        versionLabel.setText("(version information unavailable)", juce::NotificationType::dontSendNotification);
    }
}