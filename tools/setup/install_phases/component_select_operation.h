#pragma once

#include "JuceHeader.h"

class ComponentSelectOperation : public Component
{
public:
    ComponentSelectOperation();
    ~ComponentSelectOperation();

    void paint (Graphics&) override;
    void resized() override;

    TextButton* getInstallButton();
    TextButton* getUninstallButton();

private:
    Label welcomeTitle;
    Label instructionLabel;
    TextButton installButton;
    TextButton uninstallButton;
    TextButton exitButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentSelectOperation)
};