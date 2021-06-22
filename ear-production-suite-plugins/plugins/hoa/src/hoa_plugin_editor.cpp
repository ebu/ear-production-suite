#include "hoa_plugin_editor.hpp"

// TODO - remove unrequired components once UI dev complete
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/look_and_feel/shadows.hpp"
#include "hoa_plugin_processor.hpp"
#include "hoa_frontend_connector.hpp"

using namespace ear::plugin::ui;

HoaAudioProcessorEditor::HoaAudioProcessorEditor(
    HoaAudioProcessor* p)
    : AudioProcessorEditor(p),
      p_(p),
      content_(std::make_unique<HoaComponent>(p)),
      viewport_(std::make_unique<Viewport>()) {
  p->getFrontendConnector()->setStatusBarLabel(content_->statusBarLabel);

  p->getFrontendConnector()->setNameTextEditor(
      content_->mainValueBox->getNameTextEditor());
  p->getFrontendConnector()->setColourComboBox(
      content_->mainValueBox->getColourComboBox());
  p->getFrontendConnector()->setRoutingComboBox(
      content_->mainValueBox->getRoutingComboBox());
  //Me add likewise option for chooosin common definition
  p->getFrontendConnector()->setCommonDefinitionComboBox(
      content_->mainValueBox->getCommonDefinitionComboBox());
  /* Old DS Code
  // TODO - we only need to connect to UI components present in HOA plugin.
  Rewrite this.
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
  */
  viewport_->setViewedComponent(content_.get(), false);
  viewport_->setScrollBarsShown(true, true);
  viewport_->getHorizontalScrollBar().setColour(ScrollBar::thumbColourId,
                                                EarColours::Primary);
  viewport_->getVerticalScrollBar().setColour(ScrollBar::thumbColourId,
                                              EarColours::Primary);
  addAndMakeVisible(viewport_.get());

  setResizable(true, false);
  setResizeLimits(0, 0, 726, 930);
  setSize(726, 930);
}

HoaAudioProcessorEditor::~HoaAudioProcessorEditor() {}

void HoaAudioProcessorEditor::paint(Graphics& g) {}

void HoaAudioProcessorEditor::resized() {
  viewport_->setBounds(getLocalBounds());
  content_->setBounds(0, 0, 726, 930);
}
