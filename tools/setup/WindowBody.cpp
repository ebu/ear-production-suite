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
}

void WindowBody::phaseSourcesInvalid()
{
    removeAllChildren();
    // This might skip to next phase if all sources are valid
    auto invalidSources = installManifest.getInvalidSources();
    if (invalidSources.size() == 0) {
        //TODO: next phase
    }
    else {
        cSourcesInvalid.setLog(invalidSources);
        addAndMakeVisible(cSourcesInvalid);
    }
}

void WindowBody::phaseUninstallConfirm()
{
    removeAllChildren();
    //TODO: capture Confirm click to next phase
    addAndMakeVisible(cUninstallConfirm);
}
