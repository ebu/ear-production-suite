#include "component_sources_invalid.h"
#include <components/look_and_feel/colours.hpp>
#include <components/look_and_feel/fonts.hpp>

using namespace ear::plugin::ui;

ComponentSourcesInvalid::ComponentSourcesInvalid()
{
    title.setFont(EarFontsSingleton::instance().HeroHeading);
    title.setColour(Label::textColourId, EarColours::Heading);
    title.setText("Invalid Resources",
        juce::NotificationType::dontSendNotification);
    title.setJustificationType(Justification::bottomLeft);
    addAndMakeVisible(title);

    description.setFont(EarFontsSingleton::instance().Label);
    description.setColour(Label::textColourId, EarColours::Label);
    description.setText("Setup encountered problems locating the following installation resources.\nPlease try redownloading the latest release.",
        juce::NotificationType::dontSendNotification);
    description.setJustificationType(Justification::centredLeft);
    addAndMakeVisible(description);

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

ComponentSourcesInvalid::~ComponentSourcesInvalid()
{
}

void ComponentSourcesInvalid::paint (Graphics& g)
{

}

void ComponentSourcesInvalid::resized()
{
    auto area = getLocalBounds();

    title.setBounds(area.removeFromTop(50));
    description.setBounds(area.removeFromTop(50));

    const int buttonHeight = 40;
    const int  buttonWidth = 160;
    const int  buttonPadding = 10;
    auto buttonRow = area.removeFromBottom(buttonPadding + buttonHeight + buttonPadding);
    auto buttonArea = buttonRow.removeFromRight(buttonPadding + buttonWidth + buttonPadding).reduced(buttonPadding);
    exitButton.setBounds(buttonArea);

    // Remaining area is flexible for textbox
    log.setBounds(area.reduced(5));
}

void ComponentSourcesInvalid::setLog(std::vector<String> const& logItems)
{
    String logContent;
    for (auto const& item : logItems) {
        logContent += item;
        logContent += "\n";
    }
    log.setText(logContent);
}