#include "WindowBody.h"
#include <components/look_and_feel/colours.hpp>
#include <components/look_and_feel/fonts.hpp>

WindowBody::WindowBody()
{
    cInitial.getInstallButton()->onClick = [this]() { sourcesInvalidPhase(); };
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
}

void WindowBody::sourcesInvalidPhase()
{
    removeAllChildren();
    // This might skip to next phase if all sources are valid
    auto invalidSources = installManifest.getInvalidSources();
    if (invalidSources.size() == 0) {
        //GOTO next phase
    }
    else {
        addAndMakeVisible(cSourcesInvalid);
    }
}
