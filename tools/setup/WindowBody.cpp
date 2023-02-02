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
    cSourcesInvalid.setBounds(area);
    cUninstallConfirm.setBounds(area);
    cUninstallSearch.setBounds(area);
    cUninstallComplete.setBounds(area);
    cUninstallUnnecessary.setBounds(area);

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
        cSourcesInvalid.setLog(invalidSources);
        addAndMakeVisible(cSourcesInvalid);
    }
}

void WindowBody::phaseUninstallConfirm()
{
    removeAllChildren();
    // This might skip to "unnecessary" phase if no existing files found
    //auto foundFiles = uninstallManifest.getFoundFiles();
    //if (foundFiles.size() == 0) {
        phaseUninstallUnnecessary();
    //}
    //else {
        //cUninstallConfirm.getConfirmButton()->onClick = [this]() { phaseUninstallSearch(); };
        //addAndMakeVisible(cUninstallConfirm);
    //}
}

void WindowBody::phaseInstallCleanupSearch()
{
    removeAllChildren();
    // This might skip to next phase if no existing files found
    //auto foundFiles = uninstallManifest.getFoundFiles();
    //if (foundFiles.size() == 0) {
        //TODO: next phase
    //}
    //else {
        // TODO: capture remove click
        // TODO: capture skip click
        cUninstallSearch.configureForInstallPhase();
    //    cSourcesInvalid.setLog(foundFiles);
        addAndMakeVisible(cUninstallSearch);
    //}
}

void WindowBody::phaseUninstallSearch()
{
    removeAllChildren();
    cUninstallSearch.configureForUninstallPhase();
    addAndMakeVisible(cUninstallSearch);
}

void WindowBody::phaseUninstallComplete()
{
    removeAllChildren();
    addAndMakeVisible(cUninstallComplete);
}

void WindowBody::phaseUninstallUnnecessary()
{
    removeAllChildren();
    addAndMakeVisible(cUninstallUnnecessary);
}
