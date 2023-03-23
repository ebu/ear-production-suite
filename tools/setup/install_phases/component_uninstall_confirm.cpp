#include "component_uninstall_confirm.h"
#include <components/look_and_feel/colours.hpp>
#include <components/look_and_feel/fonts.hpp>

using namespace ear::plugin::ui;

ComponentUninstallConfirm::ComponentUninstallConfirm()
{
    titleLabel.setFont(EarFontsSingleton::instance().HeroHeading);
    titleLabel.setColour(Label::textColourId, EarColours::Heading);
    titleLabel.setText("Uninstall",
        juce::NotificationType::dontSendNotification);
    titleLabel.setJustificationType(Justification::centredBottom);
    addAndMakeVisible(titleLabel);

    descriptionLabel.setFont(EarFontsSingleton::instance().Label);
    descriptionLabel.setColour(Label::textColourId, EarColours::Label);
    descriptionLabel.setText("Are you sure you want to uninstall the EAR Production Suite?",
        juce::NotificationType::dontSendNotification);
    descriptionLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(descriptionLabel);

    confirmButton.setButtonText("Confirm Uninstall");
    confirmButton.setToggleable(false);
    confirmButton.setColour(TextButton::ColourIds::buttonColourId, EarColours::PrimaryHighlight);
    confirmButton.setColour(TextButton::ColourIds::textColourOnId, EarColours::Label);
    addAndMakeVisible(confirmButton);

    exitButton.setButtonText("Exit Setup");
    exitButton.setToggleable(false);
    exitButton.setColour(TextButton::ColourIds::buttonColourId, EarColours::Background);
    exitButton.setColour(TextButton::ColourIds::textColourOnId, EarColours::Label);
    exitButton.onClick = []() {JUCEApplicationBase::quit(); };
    addAndMakeVisible(exitButton);
}

ComponentUninstallConfirm::~ComponentUninstallConfirm()
{
}

void ComponentUninstallConfirm::paint (Graphics& g)
{

}

void ComponentUninstallConfirm::resized()
{
    auto area = getLocalBounds();
    auto sectionHeight = area.getHeight() / 4;

    titleLabel.setBounds(area.removeFromTop(sectionHeight));
    descriptionLabel.setBounds(area.removeFromTop(sectionHeight));

    auto buttonSectionWidth = area.getWidth() / 2;
    auto buttonHeight = std::min(area.getHeight(), 40);
    auto buttonWidth = std::min(buttonSectionWidth, 160);
    auto buttonSectionTrimTB = (area.getHeight() - buttonHeight) / 2;
    auto buttonSectionTrimLR = (buttonSectionWidth - buttonWidth) / 2;

    auto leftButtonArea = area.removeFromLeft(buttonSectionWidth);
    leftButtonArea.reduce(buttonSectionTrimLR, buttonSectionTrimTB);
    confirmButton.setBounds(leftButtonArea);

    auto rightButtonArea = area;
    rightButtonArea.reduce(buttonSectionTrimLR, buttonSectionTrimTB);
    exitButton.setBounds(rightButtonArea);

}

TextButton* ComponentUninstallConfirm::getConfirmButton()
{
    return &confirmButton;
}
