#pragma once

#include "JuceHeader.h"
#include "install_phases/component_initial.h"
#include "install_phases/component_license.h"
#include "install_phases/component_select_operation.h"
#include "install_phases/component_error_log.h"
#include "install_phases/component_uninstall_confirm.h"
#include "install_phases/component_existing_search.h"
#include "install_phases/component_complete.h"
#include "install_phases/component_processing.h"
#include "install_phases/component_install_locations.h"
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
    UninstallManifest uninstallManifest;

    ComponentInitial cInitial;
    ComponentLicense cLicense;
    ComponentSelectOperation cSelectOperation;
    ComponentErrorLog cErrorLog;
    ComponentUninstallConfirm cUninstallConfirm;
    ComponentExistingSearch cExistingSearch;
    ComponentComplete cComplete;
    ComponentProcessing cProcessing;
    ComponentInstallLocations cInstallLocations;

    void phaseLicense();
    void phaseSelectOperation();
    void phaseSourcesCheck();
    void phaseInstallCleanupSearch();
    void phaseInstallCleanupProcessStart();
    void phaseInstallCleanupProcessFinished();
    void phaseInstallLocations();
    void phaseInstallProcessStart();
    void phaseInstallProcessFinished();
    void phaseUninstallConfirm();
    void phaseUninstallSearch();
    void phaseUninstallProcessStart();
    void phaseUninstallProcessFinished();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowBody)
};