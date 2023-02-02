#include "component_uninstall_complete.h"
#include <components/look_and_feel/colours.hpp>
#include <components/look_and_feel/fonts.hpp>

using namespace ear::plugin::ui;

ComponentUninstallComplete::ComponentUninstallComplete()
{
    titleLabel.setFont(EarFontsSingleton::instance().HeroHeading);
    titleLabel.setColour(Label::textColourId, EarColours::Heading);
    titleLabel.setText("Uninstall Complete",
        juce::NotificationType::dontSendNotification);
    titleLabel.setJustificationType(Justification::centredBottom);
    addAndMakeVisible(titleLabel);

    descriptionLabel.setFont(EarFontsSingleton::instance().Label);
    descriptionLabel.setColour(Label::textColourId, EarColours::Label);
    descriptionLabel.setText("Uninstallation is complete.",
        juce::NotificationType::dontSendNotification);
    descriptionLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(descriptionLabel);

    exitButton.setButtonText("Exit Setup");
    exitButton.setToggleable(false);
    exitButton.setColour(TextButton::ColourIds::buttonColourId, EarColours::Background);
    exitButton.setColour(TextButton::ColourIds::textColourOnId, EarColours::Label);
    exitButton.onClick = []() {JUCEApplicationBase::quit(); };
    addAndMakeVisible(exitButton);
}

ComponentUninstallComplete::~ComponentUninstallComplete()
{
}

void ComponentUninstallComplete::paint (Graphics& g)
{

}

void ComponentUninstallComplete::resized()
{
    auto area = getLocalBounds();
    auto sectionHeight = area.getHeight() / 4;

    titleLabel.setBounds(area.removeFromTop(sectionHeight));
    descriptionLabel.setBounds(area.removeFromTop(sectionHeight));

    auto buttonSectionWidth = area.getWidth();
    auto buttonHeight = std::min(area.getHeight(), 40);
    auto buttonWidth = std::min(buttonSectionWidth, 160);
    auto buttonSectionTrimTB = (area.getHeight() - buttonHeight) / 2;
    auto buttonSectionTrimLR = (buttonSectionWidth - buttonWidth) / 2;
    exitButton.setBounds(area.reduced(buttonSectionTrimLR, buttonSectionTrimTB));
}