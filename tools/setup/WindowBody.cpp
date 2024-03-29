#include "WindowBody.h"
#include <components/look_and_feel/colours.hpp>
#include <components/look_and_feel/fonts.hpp>

WindowBody::WindowBody()
{
    cInitial.getContinueButton()->onClick = [this]() { phaseLicense(); };
    addAndMakeVisible(cInitial);
}

WindowBody::~WindowBody()
{
}

void WindowBody::paint (Graphics& g)
{

}
 
void WindowBody::resized()
{
    auto area = getLocalBounds();

    cInitial.setBounds(area);
    cLicense.setBounds(area);
    cSelectOperation.setBounds(area);
    cErrorLog.setBounds(area);
    cUninstallConfirm.setBounds(area);
    cExistingSearch.setBounds(area);
    cComplete.setBounds(area);
    cProcessing.setBounds(area);
    cInstallLocations.setBounds(area);

}

void WindowBody::phaseLicense()
{
    removeAllChildren();
    cLicense.getContinueButton()->onClick = [this]() { phaseSelectOperation(); };
    addAndMakeVisible(cLicense);
}

void WindowBody::phaseSelectOperation()
{
    removeAllChildren();
    cSelectOperation.getInstallButton()->onClick = [this]() { phaseSourcesCheck(); };
    cSelectOperation.getUninstallButton()->onClick = [this]() { phaseUninstallConfirm(); };
    addAndMakeVisible(cSelectOperation);
}

void WindowBody::phaseSourcesCheck()
{
    removeAllChildren();
    // This might skip to next phase if all sources are valid
    auto invalidSources = installManifest.getInvalidSources();
    if (invalidSources.size() == 0) {
        phaseInstallCleanupSearch();
    }
    else {
        cErrorLog.configureForInstallSourcesCheckPhase();
        cErrorLog.setLog(invalidSources);
        addAndMakeVisible(cErrorLog);
        // END OF USER JOURNEY
    }
}

void WindowBody::phaseInstallCleanupSearch()
{
    removeAllChildren();
    // This might skip to next phase if no existing files found
    auto foundFiles = uninstallManifest.getFoundFiles();
    if (foundFiles.size() == 0) {
        phaseInstallLocations();
    }
    else {
        cExistingSearch.getSkipButton()->onClick = [this]() { phaseInstallLocations(); };
        cExistingSearch.getRemoveButton()->onClick = [this]() { phaseInstallCleanupProcessStart(); };
        cExistingSearch.configureForInstallPhase();
        cExistingSearch.setLog(foundFiles);
        addAndMakeVisible(cExistingSearch);
    }
}

void WindowBody::phaseInstallCleanupProcessStart()
{
    removeAllChildren();
    cProcessing.configureForInstallCleanUpPhase();
    addAndMakeVisible(cProcessing);
    uninstallManifest.doUninstall([this]() { phaseInstallCleanupProcessFinished(); });
}

void WindowBody::phaseInstallCleanupProcessFinished()
{
    auto errorLog = uninstallManifest.getUninstallErrors();
    if (errorLog.size() == 0) {
        phaseInstallLocations();
    }
    else {
        removeAllChildren();
        cErrorLog.configureForInstallCleanUpPhase();
        cErrorLog.getContinueButton()->onClick = [this]() { phaseInstallLocations(); };
        cErrorLog.setLog(errorLog);
        addAndMakeVisible(cErrorLog);
    }
}

void WindowBody::phaseInstallLocations()
{
    removeAllChildren();
    cInstallLocations.setLocations(Locations::getVst3Directory(), Locations::getUserPluginsDirectory(), Locations::getExtrasDirectory());
    cInstallLocations.getContinueButton()->onClick = [this]() { phaseInstallProcessStart(); };
    addAndMakeVisible(cInstallLocations);
}

void WindowBody::phaseInstallProcessStart()
{
    removeAllChildren();
    cProcessing.configureForInstallPhase();
    addAndMakeVisible(cProcessing);
    installManifest.doInstall([this]() { phaseInstallProcessFinished(); });
}

void WindowBody::phaseInstallProcessFinished()
{
    auto errorLog = installManifest.getInstallErrors();
    if (errorLog.size() == 0) {
        removeAllChildren();
        cComplete.configureForInstallPhase();
        addAndMakeVisible(cComplete);
        // END OF USER JOURNEY
    }
    else {
        removeAllChildren();
        cErrorLog.configureForInstallPhase();
        cErrorLog.setLog(errorLog);
        addAndMakeVisible(cErrorLog);
        // END OF USER JOURNEY
    }
}

void WindowBody::phaseUninstallConfirm()
{
    removeAllChildren();
    // This might show "unnecessary" screen rather than confirmation if no existing files found
    auto foundFiles = uninstallManifest.getFoundFiles();
    if (foundFiles.size() == 0) {
        removeAllChildren();
        cComplete.configureForUninstallUnnecessaryPhase();
        addAndMakeVisible(cComplete);
        // END OF USER JOURNEY
    }
    else {
        cUninstallConfirm.getConfirmButton()->onClick = [this]() { phaseUninstallSearch(); };
        addAndMakeVisible(cUninstallConfirm);
    }
}

void WindowBody::phaseUninstallSearch()
{
    removeAllChildren();
    cExistingSearch.configureForUninstallPhase();
    auto foundFiles = uninstallManifest.getFoundFiles();
    cExistingSearch.setLog(foundFiles);
    cExistingSearch.getRemoveButton()->onClick = [this]() { phaseUninstallProcessStart(); };
    addAndMakeVisible(cExistingSearch);
}

void WindowBody::phaseUninstallProcessStart()
{
    removeAllChildren();
    cProcessing.configureForUninstallPhase();
    addAndMakeVisible(cProcessing);
    uninstallManifest.doUninstall([this]() { phaseUninstallProcessFinished(); });
}

void WindowBody::phaseUninstallProcessFinished()
{
    auto errorLog = uninstallManifest.getUninstallErrors();
    if (errorLog.size() == 0) {
        removeAllChildren();
        cComplete.configureForUninstallPhase();
        addAndMakeVisible(cComplete);
        // END OF USER JOURNEY
    }
    else {
        removeAllChildren();
        cErrorLog.configureForUninstallPhase();
        cErrorLog.setLog(errorLog);
        addAndMakeVisible(cErrorLog);
        // END OF USER JOURNEY
    }
}
