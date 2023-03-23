#pragma once

#include "JuceHeader.h"
#include <vector>

class ComponentLicense : public Component
{
public:
    ComponentLicense();
    ~ComponentLicense();

    void paint (Graphics&) override;
    void resized() override;

    TextButton* getContinueButton();

private:
    Label title;
    Label description;
    TextButton continueButton;
    TextButton exitButton;
    TextEditor license;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentLicense)
};