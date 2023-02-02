#pragma once

#include "JuceHeader.h"
#include "install_phases/component_initial.h"
#include "install_phases/component_sources_invalid.h"
#include "helpers/manifests.h"

class WindowBody : public Component
{
public:
    WindowBody();
    ~WindowBody();

    void paint (Graphics&) override;
    void resized() override;

private:
    InstallManifest installManifest;

    ComponentInitial cInitial;
    ComponentSourcesInvalid cSourcesInvalid;

    void sourcesInvalidPhase();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowBody)
};