#pragma once

#include "JuceHeader.h"
#include "install_phases/component_initial.h"
#include "install_phases/component_sources_invalid.h"
#include "install_phases/component_uninstall_confirm.h"
#include "install_phases/component_uninstall_search.h"
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
    ComponentUninstallConfirm cUninstallConfirm;
    ComponentUninstallSearch cUninstallSearch;

    void phaseSourcesInvalid();
    void phaseUninstallConfirm();
    void phaseInstallCleanupSearch();
    void phaseUninstallSearch();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowBody)
};