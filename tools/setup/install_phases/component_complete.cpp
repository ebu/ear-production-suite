#include "component_complete.h"
#include <components/look_and_feel/colours.hpp>
#include <components/look_and_feel/fonts.hpp>

using namespace ear::plugin::ui;

ComponentComplete::ComponentComplete()
{
    titleLabel.setFont(EarFontsSingleton::instance().HeroHeading);
    titleLabel.setColour(Label::textColourId, EarColours::Heading);
    titleLabel.setJustificationType(Justification::centredBottom);
    addAndMakeVisible(titleLabel);

    descriptionLabel.setFont(EarFontsSingleton::instance().Label);
    descriptionLabel.setColour(Label::textColourId, EarColours::Label);
    descriptionLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(descriptionLabel);

    exitButton.setButtonText("Exit Setup");
    exitButton.setToggleable(false);
    exitButton.setColour(TextButton::ColourIds::buttonColourId, EarColours::Background);
    exitButton.setColour(TextButton::ColourIds::textColourOnId, EarColours::Label);
    exitButton.onClick = []() {JUCEApplicationBase::quit(); };
    addAndMakeVisible(exitButton);
}

ComponentComplete::~ComponentComplete()
{
}

void ComponentComplete::paint (Graphics& g)
{

}

void ComponentComplete::resized()
{
    auto area = getLocalBounds();
    auto sectionHeight = area.getHeight() / 4;

    titleLabel.setBounds(area.removeFromTop(sectionHeight));
    descriptionLabel.setBounds(area.removeFromTop(sectionHeight / 2));

    if (autoUpdateCheck) {
        const int autoUpdateCheckHeight{ 35 };
        auto autoUpdateCheckWidth = autoUpdateCheck->getMinReqWidth(autoUpdateCheckHeight);
        auto autoUpdateCheckArea = area.removeFromTop(sectionHeight / 2).withSizeKeepingCentre(autoUpdateCheckWidth, autoUpdateCheckHeight);
        if (autoUpdateCheckArea.getX() < 0) autoUpdateCheckArea.setX(0);
        autoUpdateCheck->setBounds(autoUpdateCheckArea);
    }

    auto buttonSectionWidth = area.getWidth();
    auto buttonHeight = std::min(area.getHeight(), 40);
    auto buttonWidth = std::min(buttonSectionWidth, 160);
    auto buttonSectionTrimTB = (area.getHeight() - buttonHeight) / 2;
    auto buttonSectionTrimLR = (buttonSectionWidth - buttonWidth) / 2;
    exitButton.setBounds(area.reduced(buttonSectionTrimLR, buttonSectionTrimTB));
}

void ComponentComplete::configureForUninstallPhase()
{
    if (autoUpdateCheck) {
        removeChildComponent(autoUpdateCheck.get());
        autoUpdateCheck.reset();
    }
    titleLabel.setText("Uninstall Complete",
        juce::NotificationType::dontSendNotification);
    descriptionLabel.setText("Uninstallation is complete.",
        juce::NotificationType::dontSendNotification);
}

void ComponentComplete::configureForUninstallUnnecessaryPhase()
{
    if (autoUpdateCheck) {
        removeChildComponent(autoUpdateCheck.get());
        autoUpdateCheck.reset();
    }
    titleLabel.setText("Uninstall",
        juce::NotificationType::dontSendNotification);
    descriptionLabel.setText("Setup did not find any files relating to a previous installation.",
        juce::NotificationType::dontSendNotification);
}

void ComponentComplete::configureForInstallPhase()
{
    if (!autoUpdateCheck) {
        // Only instantiate this on successful install because UpdateCheck will want to write a settings file.
        autoUpdateCheck = std::make_unique<AutoUpdateCheckButton>();
        addAndMakeVisible(autoUpdateCheck.get());
        resized();
    }
    titleLabel.setText("Installation Complete",
        juce::NotificationType::dontSendNotification);
    descriptionLabel.setText("The EAR Production Suite has been installed.\nUse the software via the REAPER digital audio workstation.",
        juce::NotificationType::dontSendNotification);
}

AutoUpdateCheckButton::AutoUpdateCheckButton()
{
    bool success = updateCheckerSettingsFile.canWrite();

    text1.setFont(EarFontsSingleton::instance().Label);
    text1.setColour(Label::textColourId, EarColours::Label.withAlpha(Emphasis::high));
    text1.setJustificationType(Justification::centredLeft);
    text1.setText(success ? "Automatically check for updates on start-up (requires internet connection.)" : "Automatic update checking disabled - cannot write preferences to settings file.", dontSendNotification);
    addAndMakeVisible(text1);

    text2.setFont(EarFontsSingleton::instance().Label);
    text2.setColour(Label::textColourId, EarColours::Label.withAlpha(Emphasis::medium));
    text2.setJustificationType(Justification::centredLeft);
    text2.setText(success ? "You can change your preference at any time from the REAPER Extensions menu." : "You can still perform a manual update check from the REAPER Extensions menu.", dontSendNotification);
    addAndMakeVisible(text2);

    this->setMouseCursor(MouseCursor::PointingHandCursor);
    text1.setMouseCursor(MouseCursor::PointingHandCursor);
    text2.setMouseCursor(MouseCursor::PointingHandCursor);
    text1.setInterceptsMouseClicks(false, false);
    text2.setInterceptsMouseClicks(false, false);
}

AutoUpdateCheckButton::~AutoUpdateCheckButton()
{
}

void AutoUpdateCheckButton::paint(Graphics& g)
{
    auto area = getLocalBounds();
    auto checked = updateCheckerSettingsFile.getAutoCheckEnabled();
    getLookAndFeel().drawTickBox(g, text1, 1, 1, area.getHeight() - 2, area.getHeight() - 2, checked, true, true, false);
}

void AutoUpdateCheckButton::resized()
{
    auto area = getLocalBounds();
    area.removeFromLeft(area.getHeight() + 10);
    text1.setBounds(area.removeFromTop(area.getHeight() / 2));
    text2.setBounds(area);
}

int AutoUpdateCheckButton::getMinReqWidth(int forComponentHeight)
{
    auto text1Width = EarFontsSingleton::instance().Label.getStringWidthFloat(text1.getText());
    auto text2Width = EarFontsSingleton::instance().Label.getStringWidthFloat(text2.getText());
    auto textWidth = std::max(text1Width, text2Width) * 1.05f; // 5% excess
    return forComponentHeight + marginBoxText + textWidth;
}

void AutoUpdateCheckButton::setState(bool checked)
{
    updateCheckerSettingsFile.setAutoCheckEnabled(checked);
    repaint();
}

void AutoUpdateCheckButton::mouseDown(const MouseEvent& event)
{
    setState(!updateCheckerSettingsFile.getAutoCheckEnabled());
}
