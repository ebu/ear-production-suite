#pragma once

#include "JuceHeader.h"
#include <vector>

class ComponentInstallLocations : public Component
{
public:
    ComponentInstallLocations();
    ~ComponentInstallLocations();

    void paint (Graphics&) override;
    void resized() override;

    void setLocations(const String& vst3Location, const String& userPluginsLocation);

    TextButton* getContinueButton();

private:
    Label title;
    Label description;
    Label locations;
    TextButton continueButton;
    TextButton exitButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentInstallLocations)
};