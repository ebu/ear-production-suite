#include "component_select_operation.h"
#include <components/look_and_feel/colours.hpp>
#include <components/look_and_feel/fonts.hpp>

using namespace ear::plugin::ui;

ComponentSelectOperation::ComponentSelectOperation()
{
    welcomeTitle.setFont(EarFontsSingleton::instance().HeroHeading);
    welcomeTitle.setColour(Label::textColourId, EarColours::Heading);
    welcomeTitle.setText("Welcome",
        juce::NotificationType::dontSendNotification);
    welcomeTitle.setJustificationType(Justification::centredBottom);
    addAndMakeVisible(welcomeTitle);

    instructionLabel.setFont(EarFontsSingleton::instance().Label);
    instructionLabel.setColour(Label::textColourId, EarColours::Label);
    instructionLabel.setText("Welcome to the EAR Production Suite Setup Application.\n\n"
        "Please ensure REAPER is not running before continuing.\n\n\n"
        "Select an operation:",
        juce::NotificationType::dontSendNotification);
    instructionLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(instructionLabel);

    installButton.setButtonText("Install / Re-install");
    installButton.setToggleable(false);
    installButton.setColour(TextButton::ColourIds::buttonColourId, EarColours::Primary);
    installButton.setColour(TextButton::ColourIds::textColourOnId, EarColours::Label);
    addAndMakeVisible(installButton);

    uninstallButton.setButtonText("Uninstall");
    uninstallButton.setToggleable(false);
    uninstallButton.setColour(TextButton::ColourIds::buttonColourId, EarColours::PrimaryHighlight);
    uninstallButton.setColour(TextButton::ColourIds::textColourOnId, EarColours::Label);
    addAndMakeVisible(uninstallButton);

    exitButton.setButtonText("Exit Setup");
    exitButton.setToggleable(false);
    exitButton.setColour(TextButton::ColourIds::buttonColourId, EarColours::Background);
    exitButton.setColour(TextButton::ColourIds::textColourOnId, EarColours::Label);
    exitButton.onClick = []() {JUCEApplicationBase::quit(); };
    addAndMakeVisible(exitButton);
}

ComponentSelectOperation::~ComponentSelectOperation()
{
}

void ComponentSelectOperation::paint (Graphics& g)
{

}

void ComponentSelectOperation::resized()
{
    auto area = getLocalBounds();
    auto sectionHeight = area.getHeight() / 4;

    welcomeTitle.setBounds(area.removeFromTop(sectionHeight));
    instructionLabel.setBounds(area.removeFromTop(sectionHeight));

    auto buttonSectionWidth = area.getWidth() / 3;
    auto buttonHeight = std::min(area.getHeight(), 40);
    auto buttonWidth = std::min(buttonSectionWidth, 160);
    auto buttonSectionTrimTB = (area.getHeight() - buttonHeight) / 2;
    auto buttonSectionTrimLR = (buttonSectionWidth - buttonWidth) / 2;

    auto leftButtonArea = area.removeFromLeft(buttonSectionWidth);
    leftButtonArea.reduce(buttonSectionTrimLR, buttonSectionTrimTB);
    installButton.setBounds(leftButtonArea);

    auto rightButtonArea = area.removeFromRight(buttonSectionWidth);
    rightButtonArea.reduce(buttonSectionTrimLR, buttonSectionTrimTB);
    exitButton.setBounds(rightButtonArea);

    area.reduce(buttonSectionTrimLR, buttonSectionTrimTB);
    uninstallButton.setBounds(area);

}

TextButton* ComponentSelectOperation::getInstallButton()
{
    return &installButton;
}

TextButton* ComponentSelectOperation::getUninstallButton()
{
    return &uninstallButton;
}
