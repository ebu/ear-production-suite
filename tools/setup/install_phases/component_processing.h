#pragma once

#include "JuceHeader.h"

class ComponentProcessing : public Component
{
public:
    ComponentProcessing();
    ~ComponentProcessing();

    void paint (Graphics&) override;
    void resized() override;

    void configureForInstallPhase();
    void configureForInstallCleanUpPhase();
    void configureForUninstallPhase();

private:
    Label titleLabel;
    Label descriptionLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentProcessing)
};