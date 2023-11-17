#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "components/version_label.hpp"

#include <chrono>

using namespace ear::plugin::ui;

AdmStemPluginAudioProcessorEditor::AdmStemPluginAudioProcessorEditor (AdmStemPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), AsyncUpdater(), processor (p)
{
    CRT_SET

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 360);

    addAndMakeVisible(bgPanel);

    headingLabel.setFont(EarFontsSingleton::instance().HeroHeading);
    headingLabel.setColour(Label::textColourId, EarColours::Text);
    headingLabel.setText("ADM Export Source", dontSendNotification);
    headingLabel.setEditable(false);
    addAndMakeVisible(headingLabel);

    includeInAdmRenderLabel.setText(juce::String("Include in ADM Export (Render)"), juce::NotificationType::dontSendNotification);
    includeInAdmRenderLabel.setEditable(false);
    includeInAdmRenderLabel.setFont(EarFontsSingleton::instance().Label);
    includeInAdmRenderLabel.setColour(Label::textColourId, EarColours::Label);
    addAndMakeVisible(includeInAdmRenderLabel);

    includeInAdmRender.setToggleState(processor.getIncludeInAdmRender(), NotificationType::dontSendNotification);
    includeInAdmRender.onClick = [this] { buttonStateChanged(&includeInAdmRender);   };
    addAndMakeVisible(includeInAdmRender);

    typeDefinitionLabel.setText(juce::String("Type Definition"), juce::NotificationType::dontSendNotification);
    typeDefinitionLabel.setEditable(false);
    typeDefinitionLabel.setFont(EarFontsSingleton::instance().Label);
    typeDefinitionLabel.setColour(Label::textColourId, EarColours::Label);
    typeDefinitionLabel.setJustificationType(Justification::right);
    addAndMakeVisible(typeDefinitionLabel);

    packFormatLabel.setText(juce::String("Pack Format"), juce::NotificationType::dontSendNotification);
    packFormatLabel.setEditable(false);
    packFormatLabel.setFont(EarFontsSingleton::instance().Label);
    packFormatLabel.setColour(Label::textColourId, EarColours::Label);
    packFormatLabel.setJustificationType(Justification::right);
    addAndMakeVisible(packFormatLabel);

    channelFormatLabel.setText(juce::String("Channel Format"), juce::NotificationType::dontSendNotification);
    channelFormatLabel.setEditable(false);
    channelFormatLabel.setFont(EarFontsSingleton::instance().Label);
    channelFormatLabel.setColour(Label::textColourId, EarColours::Label);
    channelFormatLabel.setJustificationType(Justification::right);
    addAndMakeVisible(channelFormatLabel);

    auto elementRelationships = processor.admPresetDefinitions.getElementRelationships();

    for(auto const&[id, tdData] : elementRelationships) {
        typeDefinitionSelector.addTextEntry(tdData->name, id);
    }
    typeDefinitionSelector.onValueChange = [this](int){ comboBoxStateChanged(&typeDefinitionSelector);  };
    typeDefinitionSelector.setDefaultText("Select Type Definition"); // Should never see this text!!
    addAndMakeVisible(typeDefinitionSelector);

    packFormatSelector.onValueChange = [this](int){ comboBoxStateChanged(&packFormatSelector);  };
    addAndMakeVisible(packFormatSelector);

    channelFormatSelector.onValueChange = [this](int){ comboBoxStateChanged(&channelFormatSelector);  };
    addAndMakeVisible(channelFormatSelector);

    configureVersionLabel(versionLabel);
    addAndMakeVisible(versionLabel);

    syncEditorToProcessor();
}

AdmStemPluginAudioProcessorEditor::~AdmStemPluginAudioProcessorEditor()
{

}

void AdmStemPluginAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (EarColours::Background);
}

void AdmStemPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto area = getLocalBounds();

    area.reduce(5, 5);

    auto headingArea = area.removeFromTop(55);
    headingLabel.setBounds(headingArea);

    auto bottomLabelsArea = area.removeFromBottom(30);
    versionLabel.setBounds(bottomLabelsArea);

    bgPanel.setBounds(area.reduced(5, 5));

    area.reduce(10, 5);
    area.removeFromTop(30); // heading space
    area.removeFromTop(3.f * marginBig_);

    auto checkboxArea = area.removeFromTop(labelHeight_).withTrimmedLeft(2.f * marginBig_);
    includeInAdmRender.setBounds(checkboxArea);
    includeInAdmRenderLabel.setBounds(checkboxArea.withTrimmedLeft(labelHeight_ + marginSmall_));

    area.removeFromTop(2.f * marginBig_);

    auto tdArea = area.removeFromTop(rowHeight_);
    auto tdLabelArea = tdArea.withWidth(labelWidth_);
    auto tdComboBoxArea = tdArea.withTrimmedLeft(labelWidth_ + marginBig_).reduced(0, marginSmall_);
    typeDefinitionLabel.setBounds(tdLabelArea);
    typeDefinitionSelector.setBounds(tdComboBoxArea);

    area.removeFromTop(marginBig_);

    auto pfArea = area.removeFromTop(rowHeight_);
    auto pfLabelArea = pfArea.withWidth(labelWidth_);
    auto pfComboBoxArea = pfArea.withTrimmedLeft(labelWidth_ + marginBig_).reduced(0, marginSmall_);
    packFormatLabel.setBounds(pfLabelArea);
    packFormatSelector.setBounds(pfComboBoxArea);

    area.removeFromTop(marginBig_);

    auto cfArea = area.removeFromTop(rowHeight_);
    auto cfLabelArea = cfArea.withWidth(labelWidth_);
    auto cfComboBoxArea = cfArea.withTrimmedLeft(labelWidth_ + marginBig_).reduced(0, marginSmall_);
    channelFormatLabel.setBounds(cfLabelArea);
    channelFormatSelector.setBounds(cfComboBoxArea);
}

void AdmStemPluginAudioProcessorEditor::buttonStateChanged(Button* button){
    if(button == &includeInAdmRender){
        processor.setIncludeInAdmRender(includeInAdmRender.getToggleState());
    }
}

void AdmStemPluginAudioProcessorEditor::comboBoxStateChanged(EarComboBox* comboBox){
    if(comboBox == &typeDefinitionSelector && typeDefinitionSelector.hasSelection()) {
        processor.setTypeDefinition(typeDefinitionSelector.getSelectedId());
    }

    if (comboBox == &packFormatSelector && packFormatSelector.hasSelection()) {
        processor.setPackFormat(packFormatSelector.getSelectedId());
    }

    if (comboBox == &channelFormatSelector && channelFormatSelector.hasSelection()) {
        processor.setChannelFormat(channelFormatSelector.getSelectedId());
    }

    syncEditorToProcessor();
}

void AdmStemPluginAudioProcessorEditor::updateComboBoxOptions(int tdId, int pfId, int cfId)
{
    packFormatSelector.clearEntries();
    channelFormatSelector.clearEntries();
    typeDefinitionSelector.setSelectedId(tdId, juce::NotificationType::dontSendNotification);

    if(tdId == adm::TypeDefinition::UNDEFINED.get()) {
        packFormatSelector.setDefaultText("");
        channelFormatSelector.setDefaultText("");
    } else if(tdId == adm::TypeDefinition::OBJECTS.get()) {
        packFormatSelector.setDefaultText("(Auto-generated - no selection required)");
        channelFormatSelector.setDefaultText("(Auto-generated - no selection required)");
    } else {
        packFormatSelector.setDefaultText("Select Pack Format");
        channelFormatSelector.setDefaultText("Select Pack Format");
    }

    auto typeDefinitionData = processor.admPresetDefinitions.getTypeDefinitionData(tdId);

    if(typeDefinitionData) {
        for(auto& pfData : typeDefinitionData->relatedPackFormats) {
            packFormatSelector.addTextEntry(pfData->niceName, pfData->idValue);
            if(pfData->idValue == pfId) {
                packFormatSelector.setSelectedId(pfId, juce::NotificationType::dontSendNotification);
                channelFormatSelector.setDefaultText("Select Channel Format");
                channelFormatSelector.addTextEntry("[All Channels]", CHANNELFORMAT_ALLCHANNELS_ID);
                if(cfId == CHANNELFORMAT_ALLCHANNELS_ID) channelFormatSelector.setSelectedId(CHANNELFORMAT_ALLCHANNELS_ID, juce::NotificationType::dontSendNotification);
                for(auto& cfData : pfData->relatedChannelFormats) {
                    channelFormatSelector.addTextEntry(cfData->name, cfData->idValue);
                    if(cfData->idValue == cfId) {
                        channelFormatSelector.setSelectedId(cfId, juce::NotificationType::dontSendNotification);
                    }
                }
            }
        }
    }

}

void AdmStemPluginAudioProcessorEditor::syncEditorToProcessor()
{
    if(asyncTasks.size_approx() > 0) return;
    asyncTasks.enqueue([=] {
        updateComboBoxOptions(processor.getTypeDefinition(), processor.getPackFormat(), processor.getChannelFormat());
    });
    triggerAsyncUpdate();
}

void AdmStemPluginAudioProcessorEditor::handleAsyncUpdate()
{
    // Parameter updates can consequently call component update methods, but these changes can not be enacted immediately.
    // This is because the param change occurs on a reaper thread and thus those methods are called on the reaper thread.
    // The message manager does not like this because it wouldn't be thread safe.
    // We need to either lock the message manager or use AsyncUpdater to do the updates on the message manager thread itself.
    // We can't lock the message manager using MessageManagerLock because it takes a target thread as an argument and resolving this seems to return null (presumably because it's not a juce-spawned thread?)
    // Thus, all updates need to be done with AsyncUpdater, which will call this method on the message manager thread when it is ready.

    std::function<void()> task;
    while (asyncTasks.wait_dequeue_timed(task, std::chrono::milliseconds(5))) task();

    repaint();
}
