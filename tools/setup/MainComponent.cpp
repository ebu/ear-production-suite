#include "MainComponent.h"
#include <components/version_label.hpp>
#include <components/look_and_feel/colours.hpp>
#include <components/look_and_feel/fonts.hpp>

MainComponent::MainComponent()
{
    header.setText(" Setup");
    addAndMakeVisible(header);
    configureVersionLabel(versionLabel);
    addAndMakeVisible(versionLabel);
    setSize (600, 400);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint (Graphics& g)
{
    g.fillAll (EarColours::Background);
}

void MainComponent::resized()
{
    header.setBounds(getLocalBounds().removeFromTop(50));
    versionLabel.setBounds(getLocalBounds().removeFromBottom(30));
}