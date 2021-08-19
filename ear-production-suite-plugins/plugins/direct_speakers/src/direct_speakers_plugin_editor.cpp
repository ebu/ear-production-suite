#include "direct_speakers_plugin_editor.hpp"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/look_and_feel/shadows.hpp"
#include "direct_speakers_plugin_processor.hpp"
#include "direct_speakers_frontend_connector.hpp"

using namespace ear::plugin::ui;

DirectSpeakersAudioProcessorEditor::DirectSpeakersAudioProcessorEditor(
    DirectSpeakersAudioProcessor* p)
    : AudioProcessorEditor(p),
      p_(p),
      content_(std::make_unique<DirectSpeakersComponent>(p)),
      viewport_(std::make_unique<Viewport>()) {
  p->getFrontendConnector()->setStatusBarLabel(content_->statusBarLabel);
  p->getFrontendConnector()->setNameTextEditor(
      content_->mainValueBox->getNameTextEditor());
  p->getFrontendConnector()->setColourComboBox(
      content_->mainValueBox->getColourComboBox());
  p->getFrontendConnector()->setRoutingComboBox(
      content_->mainValueBox->getRoutingComboBox());
  p->getFrontendConnector()->setSpeakerSetupsComboBox(
      content_->mainValueBox->getSpeakerSetupsComboBox());
  p->getFrontendConnector()->setUpperLayerValueBox(
      content_->upperLayerValueBox);
  p->getFrontendConnector()->setMiddleLayerValueBox(
      content_->middleLayerValueBox);
  p->getFrontendConnector()->setBottomLayerValueBox(
      content_->bottomLayerValueBox);
  p->getFrontendConnector()->setChannelGainsValueBox(
      content_->channelGainValueBox);

  viewport_->setViewedComponent(content_.get(), false);
  viewport_->setScrollBarsShown(true, true);
  viewport_->getHorizontalScrollBar().setColour(ScrollBar::thumbColourId,
                                                EarColours::Primary);
  viewport_->getVerticalScrollBar().setColour(ScrollBar::thumbColourId,
                                              EarColours::Primary);
  addAndMakeVisible(viewport_.get());

  setResizable(true, false);
  setResizeLimits(0, 0, 750, 930);
  setSize(750, 930);
}

DirectSpeakersAudioProcessorEditor::~DirectSpeakersAudioProcessorEditor() {}

void DirectSpeakersAudioProcessorEditor::paint(Graphics& g) {}

void DirectSpeakersAudioProcessorEditor::resized() {
  viewport_->setBounds(getLocalBounds());
  content_->setBounds(0, 0, 750, 930);
}
