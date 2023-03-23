#include "component_processing.h"
#include <components/look_and_feel/colours.hpp>
#include <components/look_and_feel/fonts.hpp>

using namespace ear::plugin::ui;

ComponentProcessing::ComponentProcessing()
{
    titleLabel.setFont(EarFontsSingleton::instance().HeroHeading);
    titleLabel.setColour(Label::textColourId, EarColours::Heading);
    titleLabel.setJustificationType(Justification::centredBottom);
    addAndMakeVisible(titleLabel);

    descriptionLabel.setFont(EarFontsSingleton::instance().Label);
    descriptionLabel.setColour(Label::textColourId, EarColours::Label);
    descriptionLabel.setText("Processing. Please wait...",
        juce::NotificationType::dontSendNotification);
    descriptionLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(descriptionLabel);

}

ComponentProcessing::~ComponentProcessing()
{
}

void ComponentProcessing::paint (Graphics& g)
{

}

void ComponentProcessing::resized()
{
    auto area = getLocalBounds();
    auto sectionHeight = area.getHeight() / 4;

    titleLabel.setBounds(area.removeFromTop(sectionHeight));
    descriptionLabel.setBounds(area.removeFromTop(sectionHeight));

}

void ComponentProcessing::configureForInstallPhase()
{
    titleLabel.setText("Installing",
        juce::NotificationType::dontSendNotification);
}

void ComponentProcessing::configureForInstallCleanUpPhase()
{
    titleLabel.setText("Pre-Install Clean-Up",
        juce::NotificationType::dontSendNotification);
}

void ComponentProcessing::configureForUninstallPhase()
{
    titleLabel.setText("Uninstalling",
        juce::NotificationType::dontSendNotification);
}
