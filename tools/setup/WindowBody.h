#pragma once

#include "JuceHeader.h"
#include "install_phases/component_initial.h"
#include "helpers/manifests.h"

class WindowBody : public Component
{
public:
    WindowBody();
    ~WindowBody();

    void paint (Graphics&) override;
    void resized() override;

private:
    ComponentInitial cInitial;
    InstallManifest installManifest;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowBody)
};