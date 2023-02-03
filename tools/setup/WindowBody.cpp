#include "WindowBody.h"
#include <components/look_and_feel/colours.hpp>
#include <components/look_and_feel/fonts.hpp>

WindowBody::WindowBody()
{
    cInitial.getInstallButton()->onClick = [this]() { phaseSourcesInvalid(); };
    cInitial.getUninstallButton()->onClick = [this]() { phaseUninstallConfirm(); };
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
    cErrorLog.setBounds(area);
    cUninstallConfirm.setBounds(area);
    cExistingSearch.setBounds(area);
    cComplete.setBounds(area);
    cProcessing.setBounds(area);
    cInstallLocations.setBounds(area);

}

void WindowBody::phaseSourcesInvalid()
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
        cExistingSearch.getRemoveButton()->onClick = [this]() { phaseInstallCleanupProcess(); };
        cExistingSearch.configureForInstallPhase();
        cExistingSearch.setLog(foundFiles);
        addAndMakeVisible(cExistingSearch);
    }
}

void WindowBody::phaseInstallCleanupProcess()
{
    removeAllChildren();
    cProcessing.configureForInstallCleanUpPhase();
    addAndMakeVisible(cProcessing);
    uninstallManifest.doUninstall();
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
    cInstallLocations.setLocations(Locations::getVst3Directory(), Locations::getUserPluginsDirectory());
    cInstallLocations.getContinueButton()->onClick = [this]() { phaseInstallProcess(); };
    addAndMakeVisible(cInstallLocations);
}

void WindowBody::phaseInstallProcess()
{
    removeAllChildren();
    cProcessing.configureForInstallPhase();
    addAndMakeVisible(cProcessing);
    installManifest.doInstall();
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
    cExistingSearch.getRemoveButton()->onClick = [this]() { phaseUninstallProcess(); };
    addAndMakeVisible(cExistingSearch);
}

void WindowBody::phaseUninstallProcess()
{
    removeAllChildren();
    cProcessing.configureForUninstallPhase();
    addAndMakeVisible(cProcessing);
    uninstallManifest.doUninstall();
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
