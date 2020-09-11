#pragma once

#include "JuceHeader_Wrapper.h"
#include "readerwriterqueue.h"
#include "PluginProcessor.h"
#include <adm/adm.hpp>

#include "ear_theming/components/customised_ear_combo_box.hpp"
#include "ear_theming/helper/graphics.hpp"
#include "ear_theming/components/look_and_feel/fonts.hpp"

#include <functional>
#include <memory>
#include <map>

class ComboBox_ZeroIndexed : public ComboBox {
public:
    ComboBox_ZeroIndexed() : ComboBox{} {};
    ~ComboBox_ZeroIndexed() {};

    void addItem(const String& newItemText, int newItemId) {
        ComboBox::addItem(newItemText, newItemId + 1);
    }
    int getSelectedId() const {
        return ComboBox::getSelectedId() - 1;
    }
    void setSelectedId(int newItemId, NotificationType notification = sendNotificationAsync) {
        ComboBox::setSelectedId(newItemId + 1, notification);
    }
    bool hasSelection() {
        return ComboBox::getSelectedId() != 0;
    }
    void clearSelection(NotificationType notification = sendNotificationAsync) {
        ComboBox::setSelectedId(0, notification);
    }
    bool selectFirstItem(NotificationType notification = sendNotificationAsync) {
        int itemId = ComboBox::getItemId(0);
        if(itemId != 0) ComboBox::setSelectedId(itemId, notification);
        return (itemId != 0);
    }

};

class Panel : public Component
{
public:

    Panel() {
        headingLabel.setFont(ear::plugin::ui::EarFonts::Heading);
        headingLabel.setColour(Label::textColourId, ear::plugin::ui::EarColours::Heading);
        headingLabel.setText("AudioObject Settings", juce::NotificationType::dontSendNotification);
        headingLabel.setJustificationType(Justification::bottomLeft);
        addAndMakeVisible(headingLabel);
    };

    ~Panel() {};

    void paint(juce::Graphics& g) override {
        g.fillAll(ear::plugin::ui::EarColours::Area01dp);
    };

    void resized() override {
        auto area = getLocalBounds();
        area.reduce(10, 5);
        headingLabel.setBounds(area.removeFromTop(30));
    }

    Label headingLabel;

private:
    const float labelWidth_ = 110.f;
    const float rowHeight_ = 40.f;
    const float marginSmall_ = 5.f;
    const float marginBig_ = 10.f;
};

class AdmStemPluginAudioProcessorEditor  : public AudioProcessorEditor, public AsyncUpdater
{
public:
    AdmStemPluginAudioProcessorEditor (AdmStemPluginAudioProcessor&);
    ~AdmStemPluginAudioProcessorEditor();

    void handleAsyncUpdate() override;
    void syncEditorToProcessor();
    void updateComboBoxOptions(int tdId, int pfId, int cfId);

    void paint (Graphics&) override;
    void resized() override;
    void buttonStateChanged(Button* button);
    void comboBoxStateChanged(ear::plugin::ui::EarComboBox* comboBox);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AdmStemPluginAudioProcessor& processor;

    Label headingLabel;
    Label includeInAdmRenderLabel;
    ToggleButton includeInAdmRender;
    Label typeDefinitionLabel;
    ear::plugin::ui::EarComboBox typeDefinitionSelector;
    Label packFormatLabel;
    ear::plugin::ui::EarComboBox packFormatSelector;
    Label channelFormatLabel;
    ear::plugin::ui::EarComboBox channelFormatSelector;
    Panel bgPanel;

    const float labelWidth_ = 110.f;
    const float labelHeight_ = 20.f;
    const float rowHeight_ = 40.f;
    const float marginSmall_ = 5.f;
    const float marginBig_ = 10.f;

    moodycamel::BlockingReaderWriterQueue<std::function<void()>> asyncTasks;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AdmStemPluginAudioProcessorEditor)
};
