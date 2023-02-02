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
    setSize (900, 600);

    resetToBeginning();
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
    auto area = getLocalBounds();
    header.setBounds(area.removeFromTop(50));
    versionLabel.setBounds(area.removeFromBottom(30));
    if(windowBody)
        windowBody->setBounds(area);
}

void MainComponent::resetToBeginning()
{
    if (windowBody) {
        removeChildComponent(windowBody.get());
        windowBody.reset();
    }

    windowBody = std::make_unique<WindowBody>();
    addAndMakeVisible(windowBody.get());
    resized();
}
