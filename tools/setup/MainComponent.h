#pragma once

#include "JuceHeader.h"
#include <components/ear_header.hpp>
#include "WindowBody.h"
#include <memory>

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

    void resetToBeginning();

    std::unique_ptr<WindowBody> windowBody;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};