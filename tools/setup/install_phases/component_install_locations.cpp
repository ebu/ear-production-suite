#include "component_install_locations.h"
#include <components/look_and_feel/colours.hpp>
#include <components/look_and_feel/fonts.hpp>

using namespace ear::plugin::ui;

ComponentInstallLocations::ComponentInstallLocations()
{
    title.setFont(EarFontsSingleton::instance().HeroHeading);
    title.setColour(Label::textColourId, EarColours::Heading);
    title.setJustificationType(Justification::bottomLeft);
    title.setText("Install Locations",
        juce::NotificationType::dontSendNotification);
    addAndMakeVisible(title);

    description.setFont(EarFontsSingleton::instance().Label);
    description.setColour(Label::textColourId, EarColours::Label);
    description.setJustificationType(Justification::centredLeft);
    description.setText("Setup will install EAR Production Suite files to the following locations:",
        juce::NotificationType::dontSendNotification);
    addAndMakeVisible(description);

    locations.setFont(EarFontsSingleton::instance().Label);
    locations.setColour(Label::textColourId, EarColours::Label);
    locations.setJustificationType(Justification::topLeft);
    addAndMakeVisible(locations);

    continueButton.setButtonText("Continue");
    continueButton.setToggleable(false);
    continueButton.setColour(TextButton::ColourIds::buttonColourId, EarColours::Primary);
    continueButton.setColour(TextButton::ColourIds::textColourOnId, EarColours::Label);
    addAndMakeVisible(continueButton);

    exitButton.setButtonText("Exit Setup");
    exitButton.setToggleable(false);
    exitButton.setColour(TextButton::ColourIds::buttonColourId, EarColours::Background);
    exitButton.setColour(TextButton::ColourIds::textColourOnId, EarColours::Label);
    exitButton.onClick = []() {JUCEApplicationBase::quit(); };
    addAndMakeVisible(exitButton);
}

ComponentInstallLocations::~ComponentInstallLocations()
{
}

void ComponentInstallLocations::paint (Graphics& g)
{

}

void ComponentInstallLocations::resized()
{
    auto area = getLocalBounds();

    title.setBounds(area.removeFromTop(50));
    description.setBounds(area.removeFromTop(50));

    const int buttonHeight = 40;
    const int buttonWidth = 160;
    const int buttonPadding = 10;
    auto buttonRow = area.removeFromBottom(buttonPadding + buttonHeight + buttonPadding);
    auto buttonSectionWidth = buttonRow.getWidth() / 2;
    auto buttonSectionTrimTB = (buttonRow.getHeight() - buttonHeight) / 2;
    auto buttonSectionTrimLR = (buttonSectionWidth - buttonWidth) / 2;

    auto leftButtonArea = buttonRow.removeFromLeft(buttonSectionWidth);
    leftButtonArea.reduce(buttonSectionTrimLR, buttonSectionTrimTB);
    continueButton.setBounds(leftButtonArea);

    auto rightButtonArea = buttonRow;
    rightButtonArea.reduce(buttonSectionTrimLR, buttonSectionTrimTB);
    exitButton.setBounds(rightButtonArea);

    // Remaining area is centre section - add some Top and Left padding
    auto locationArea = area.reduced(20, 20);
    locations.setBounds(locationArea);

}

void ComponentInstallLocations::setLocations(const String& vst3Location, const String& userPluginsLocation, const String& extrasLocation)
{
    locations.setText(vst3Location + "\n" + userPluginsLocation + "\n" + extrasLocation,
        juce::NotificationType::dontSendNotification);
}

TextButton* ComponentInstallLocations::getContinueButton()
{
    return &continueButton;
}
