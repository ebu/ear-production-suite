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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentSourcesInvalid)
};