#pragma once

#include "JuceHeader.h"

class ComponentUninstallComplete : public Component
{
public:
    ComponentUninstallComplete();
    ~ComponentUninstallComplete();

    void paint (Graphics&) override;
    void resized() override;

private:
    Label titleLabel;
    Label descriptionLabel;
    TextButton exitButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentUninstallComplete)
};