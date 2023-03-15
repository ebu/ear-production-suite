#include "component_license.h"
#include <components/look_and_feel/colours.hpp>
#include <components/look_and_feel/fonts.hpp>

#include <helpers/license.h>

using namespace ear::plugin::ui;

ComponentLicense::ComponentLicense()
{
    title.setFont(EarFontsSingleton::instance().HeroHeading);
    title.setColour(Label::textColourId, EarColours::Heading);
    title.setJustificationType(Justification::bottomLeft);
    title.setText("License Agreement",
        juce::NotificationType::dontSendNotification);
    addAndMakeVisible(title);

    description.setFont(EarFontsSingleton::instance().Label);
    description.setColour(Label::textColourId, EarColours::Label);
    description.setJustificationType(Justification::centredLeft);
    description.setText("The EAR Production Suite carries the GPLv3 License.\n"
        "Please read the following license agreement. You must agree to the terms of this agreement before continuing.",
        juce::NotificationType::dontSendNotification);
    addAndMakeVisible(description);

    continueButton.setButtonText("I Agree");
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

    license.setReadOnly(true);
    license.setScrollbarsShown(true);
    license.setMultiLine(true, false);
    license.setColour(TextEditor::ColourIds::backgroundColourId, EarColours::Area06dp);
    license.setColour(TextEditor::ColourIds::textColourId, EarColours::Text);
    license.setText(licenseText);
    addAndMakeVisible(license);
}

ComponentLicense::~ComponentLicense()
{
}

void ComponentLicense::paint (Graphics& g)
{

}

void ComponentLicense::resized()
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

    // Remaining area is flexible for textbox
    auto sideTrim = (area.getWidth() - 600) / 2;
    if (sideTrim < 10) sideTrim = 10;
    license.setBounds(area.reduced(sideTrim, 5));
}

TextButton* ComponentLicense::getContinueButton()
{
    return &continueButton;
}
