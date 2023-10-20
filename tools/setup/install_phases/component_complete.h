#pragma once

#include "JuceHeader.h"
#include <memory>
#include <update_check_settings_file.h>

class AutoUpdateCheckButton : public Component
{
public:
    AutoUpdateCheckButton();
    ~AutoUpdateCheckButton();

    void paint(Graphics&) override;
    void resized() override;

    int getMinReqWidth(int forComponentHeight);

    void setState(bool checked);

    void mouseDown(const MouseEvent& event) override;

private:
    Label text1;
    Label text2;

    const int marginBoxText{ 10 };

    UpdateCheckerSettingsFile updateCheckerSettingsFile;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutoUpdateCheckButton)
};

class ComponentComplete : public Component
{
public:
    ComponentComplete();
    ~ComponentComplete();

    void paint (Graphics&) override;
    void resized() override;

    void configureForInstallPhase();
    void configureForUninstallPhase();
    void configureForUninstallUnnecessaryPhase();

private:
    Label titleLabel;
    Label descriptionLabel;
    TextButton exitButton;
    std::unique_ptr<AutoUpdateCheckButton> autoUpdateCheck;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentComplete)
};