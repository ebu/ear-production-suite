#include "component_initial.h"
#include <components/look_and_feel/colours.hpp>
#include <components/look_and_feel/fonts.hpp>

using namespace ear::plugin::ui;

ComponentInitial::ComponentInitial()
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
        "Please ensure REAPER is not running before continuing.",
        juce::NotificationType::dontSendNotification);
    instructionLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(instructionLabel);

    continueButton.setButtonText("Continue");
    continueButton.setToggleable(false);
    continueButton.setColour(TextButton::ColourIds::buttonColourId, EarColours::Primary);
    continueButton.setColour(TextButton::ColourIds::textColourOnId, EarColours::Label);
    addAndMakeVisible(continueButton);
}

ComponentInitial::~ComponentInitial()
{
}

void ComponentInitial::paint (Graphics& g)
{

}

void ComponentInitial::resized()
{
    auto area = getLocalBounds();
    auto sectionHeight = area.getHeight() / 4;

    welcomeTitle.setBounds(area.removeFromTop(sectionHeight));
    instructionLabel.setBounds(area.removeFromTop(sectionHeight));

    auto buttonSectionWidth = area.getWidth();
    auto buttonHeight = std::min(area.getHeight(), 40);
    auto buttonWidth = std::min(buttonSectionWidth, 160);
    auto buttonSectionTrimTB = (area.getHeight() - buttonHeight) / 2;
    auto buttonSectionTrimLR = (buttonSectionWidth - buttonWidth) / 2;
    continueButton.setBounds(area.reduced(buttonSectionTrimLR, buttonSectionTrimTB));
}

TextButton* ComponentInitial::getContinueButton()
{
    return &continueButton;
}