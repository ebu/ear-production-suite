#pragma once

#include "JuceHeader.h"

class ComponentInitial : public Component
{
public:
    ComponentInitial();
    ~ComponentInitial();

    void paint (Graphics&) override;
    void resized() override;

    TextButton* getContinueButton();

private:
    Label welcomeTitle;
    Label instructionLabel;
    TextButton continueButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentInitial)
};