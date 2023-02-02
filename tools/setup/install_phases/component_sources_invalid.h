#pragma once

#include "JuceHeader.h"
#include <vector>

class ComponentSourcesInvalid : public Component
{
public:
    ComponentSourcesInvalid();
    ~ComponentSourcesInvalid();

    void paint (Graphics&) override;
    void resized() override;

    void setLog(std::vector<String> const& logItems);

private:
    Label title;
    Label description;
    TextButton exitButton;
    TextEditor log;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentSourcesInvalid)
};