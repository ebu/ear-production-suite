#pragma once

#include "JuceHeader.h"

class ComponentComplete : public Component
{
public:
    ComponentComplete();
    ~ComponentComplete();

    void paint (Graphics&) override;
    void resized() override;

    void configureForInstallPhase();
    void configureForUninstallPhase();
    void configureForUninstallUnnecessaryPhase();

private:
    Label titleLabel;
    Label descriptionLabel;
    TextButton exitButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentComplete)
};