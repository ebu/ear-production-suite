#pragma once

#include "JuceHeader.h"
#include <vector>

class ComponentErrorLog : public Component
{
public:
    ComponentErrorLog();
    ~ComponentErrorLog();

    void paint (Graphics&) override;
    void resized() override;

    void configureForInstallSourcesPhase();
    void configureForInstallCleanUpPhase();
    void configureForUninstallPhase();

    void setLog(std::vector<String> const& logItems);

private:
    Label title;
    Label description;
    TextButton continueButton;
    TextButton exitButton;
    TextEditor log;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentErrorLog)
};