#include "hoa_plugin_editor.hpp"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/look_and_feel/shadows.hpp"
#include "hoa_frontend_connector.hpp"

namespace{
  const int desiredWidth{ 1000 };
  const int desiredHeight{ 930 };
}

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
  p->getFrontendConnector()->setHoaTypeComboBox(
      content_->mainValueBox->getHoaTypeComboBox());
  p->getFrontendConnector()->setOrderDisplayValueBox(
      content_->orderDisplayValueBox);

  viewport_->setViewedComponent(content_.get(), false);
  viewport_->setScrollBarsShown(true, true);
  viewport_->getHorizontalScrollBar().setColour(ScrollBar::thumbColourId,
                                                EarColours::Primary);
  viewport_->getVerticalScrollBar().setColour(ScrollBar::thumbColourId,
                                              EarColours::Primary);
  addAndMakeVisible(viewport_.get());

  setResizable(true, false);
  setResizeLimits(0, 0, desiredWidth, desiredHeight);
  setSize(desiredWidth, desiredHeight);
}

HoaAudioProcessorEditor::~HoaAudioProcessorEditor() {}

void HoaAudioProcessorEditor::paint(Graphics& g) {}

void HoaAudioProcessorEditor::resized() {
  viewport_->setBounds(getLocalBounds());
  content_->setBounds(0, 0, desiredWidth, desiredHeight);
}
