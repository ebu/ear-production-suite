#include "component_existing_search.h"
#include <components/look_and_feel/colours.hpp>
#include <components/look_and_feel/fonts.hpp>

using namespace ear::plugin::ui;

ComponentExistingSearch::ComponentExistingSearch()
{
    title.setFont(EarFontsSingleton::instance().HeroHeading);
    title.setColour(Label::textColourId, EarColours::Heading);
    title.setJustificationType(Justification::bottomLeft);
    addAndMakeVisible(title);

    description.setFont(EarFontsSingleton::instance().Label);
    description.setColour(Label::textColourId, EarColours::Label);
    description.setJustificationType(Justification::centredLeft);
    addAndMakeVisible(description);

    skipButton.setButtonText("Skip");
    skipButton.setToggleable(false);
    skipButton.setColour(TextButton::ColourIds::buttonColourId, EarColours::Background);
    skipButton.setColour(TextButton::ColourIds::textColourOnId, EarColours::Label);
    addAndMakeVisible(skipButton);

    removeButton.setButtonText("Remove All");
    removeButton.setToggleable(false);
    removeButton.setColour(TextButton::ColourIds::buttonColourId, EarColours::PrimaryHighlight);
    removeButton.setColour(TextButton::ColourIds::textColourOnId, EarColours::Label);
    addAndMakeVisible(removeButton);

    exitButton.setButtonText("Exit Setup");
    exitButton.setToggleable(false);
    exitButton.setColour(TextButton::ColourIds::buttonColourId, EarColours::Background);
    exitButton.setColour(TextButton::ColourIds::textColourOnId, EarColours::Label);
    exitButton.onClick = []() {JUCEApplicationBase::quit(); };
    addAndMakeVisible(exitButton);

    log.setReadOnly(true);
    log.setScrollbarsShown(true);
    log.setMultiLine(true);
    log.setColour(TextEditor::ColourIds::backgroundColourId, EarColours::Area06dp);
    log.setColour(TextEditor::ColourIds::textColourId, EarColours::Text);
    addAndMakeVisible(log);
}

ComponentExistingSearch::~ComponentExistingSearch()
{
}

void ComponentExistingSearch::paint (Graphics& g)
{

}

void ComponentExistingSearch::resized()
{
    auto area = getLocalBounds();

    title.setBounds(area.removeFromTop(50));
    description.setBounds(area.removeFromTop(50));

    auto buttonRow = area.removeFromBottom(60);
    auto buttonSectionWidth = buttonRow.getWidth() / 3;
    auto buttonHeight = std::min(buttonRow.getHeight(), 40);
    auto buttonWidth = std::min(buttonSectionWidth, 160);
    auto buttonSectionTrimTB = (buttonRow.getHeight() - buttonHeight) / 2;
    auto buttonSectionTrimLR = (buttonSectionWidth - buttonWidth) / 2;

    auto leftButtonArea = buttonRow.removeFromLeft(buttonSectionWidth);
    leftButtonArea.reduce(buttonSectionTrimLR, buttonSectionTrimTB);
    skipButton.setBounds(leftButtonArea);

    auto rightButtonArea = buttonRow.removeFromRight(buttonSectionWidth);
    rightButtonArea.reduce(buttonSectionTrimLR, buttonSectionTrimTB);
    exitButton.setBounds(rightButtonArea);

    buttonRow.reduce(buttonSectionTrimLR, buttonSectionTrimTB);
    removeButton.setBounds(buttonRow);

    // Remaining area is flexible for textbox
    log.setBounds(area.reduced(5));
}

void ComponentExistingSearch::configureForInstallPhase()
{
    skipButton.setVisible(true);
    title.setText("Pre-Install Clean-Up",
        juce::NotificationType::dontSendNotification);
    description.setText("Setup found the following files and directories relating to a previous installation. These may conflict with the new installation.\nClick Remove All if you would like Setup to delete these. Note that Setup may overwrite these files during installation in any case.",
        juce::NotificationType::dontSendNotification);
}

void ComponentExistingSearch::configureForUninstallPhase()
{
    skipButton.setVisible(false);
    title.setText("Uninstall",
        juce::NotificationType::dontSendNotification);
    description.setText("Setup found the following files and directories relating to a previous installation.\nClick Remove All if you would like Setup to delete these.",
        juce::NotificationType::dontSendNotification);
}

TextButton* ComponentExistingSearch::getRemoveButton()
{
    return &removeButton;
}

TextButton* ComponentExistingSearch::getSkipButton()
{
    return &skipButton;
}

void ComponentExistingSearch::setLog(std::vector<String> const& logItems)
{
    String logContent;
    for (auto const& item : logItems) {
        logContent += item;
        logContent += "\n";
    }
    log.setText(logContent);
}
