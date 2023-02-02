#pragma once

#include "JuceHeader.h"

class ComponentUninstallConfirm : public Component
{
public:
    ComponentUninstallConfirm();
    ~ComponentUninstallConfirm();

    void paint (Graphics&) override;
    void resized() override;

    TextButton* getConfirmButton();

private:
    Label titleLabel;
    Label descriptionLabel;
    TextButton confirmButton;
    TextButton exitButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentUninstallConfirm)
};