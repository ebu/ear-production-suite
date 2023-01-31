#pragma once

#include "JuceHeader.h"
#include <components/ear_header.hpp>

using namespace ear::plugin::ui;

class MainComponent   : public Component
{
public:
    MainComponent();
    ~MainComponent();

    void paint (Graphics&) override;
    void resized() override;

private:
    Label versionLabel;
    EarHeader header;
    bool processing{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};