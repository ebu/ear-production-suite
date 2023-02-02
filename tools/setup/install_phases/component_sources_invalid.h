#pragma once

#include "JuceHeader.h"

class ComponentSourcesInvalid : public Component
{
public:
    ComponentSourcesInvalid();
    ~ComponentSourcesInvalid();

    void paint (Graphics&) override;
    void resized() override;

private:
    Label title;
    Label description;
    TextButton exitButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentSourcesInvalid)
};