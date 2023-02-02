#pragma once

#include "JuceHeader.h"
#include <vector>

class ComponentUninstallSearch : public Component
{
public:
    ComponentUninstallSearch();
    ~ComponentUninstallSearch();

    void paint (Graphics&) override;
    void resized() override;

    void configureForInstallPhase();
    void configureForUninstallPhase();

    TextButton* getRemoveButton();
    TextButton* getSkipButton();

    void setLog(std::vector<String> const& logItems);

private:
    Label title;
    Label description;
    TextButton removeButton;
    TextButton skipButton;
    TextButton exitButton;
    TextEditor log;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentUninstallSearch)
};