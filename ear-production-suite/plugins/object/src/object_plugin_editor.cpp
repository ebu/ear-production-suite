#include "object_plugin_editor.hpp"

#include "object_frontend_connector.hpp"
#include "object_plugin_processor.hpp"

using namespace ear::plugin::ui;

ObjectAudioProcessorEditor::ObjectAudioProcessorEditor(ObjectsAudioProcessor* p)
    : AudioProcessorEditor(p),
      p_(p),
      content_(std::make_unique<ObjectsComponent>(p)),
      viewport_(std::make_unique<Viewport>()) {
  /* clang-format off */
  p->getFrontendConnector()->setStatusBarLabel(content_->statusBarLabel);
  p->getFrontendConnector()->setNameTextEditor(content_->mainValueBox->getNameTextEditor());
  p->getFrontendConnector()->setColourComboBox(content_->mainValueBox->getColourComboBox());
  p->getFrontendConnector()->setRoutingComboBox(content_->mainValueBox->getRoutingComboBox());

  p->getFrontendConnector()->setGainSlider(content_->gainValueBox->getGainSlider());
  p->getFrontendConnector()->setAzimuthSlider(content_->panningValueBox->getAzimuthSlider());
  p->getFrontendConnector()->setElevationSlider(content_->panningValueBox->getElevationSlider());
  p->getFrontendConnector()->setDistanceSlider(content_->panningValueBox->getDistanceSlider());

  p->getFrontendConnector()->setLinkSizeButton(content_->extentValueBox->getLinkSizeButton());
  p->getFrontendConnector()->setSizeLabel(content_->extentValueBox->getSizeLabel());
  p->getFrontendConnector()->setSizeSlider(content_->extentValueBox->getSizeSlider());
  p->getFrontendConnector()->setWidthLabel(content_->extentValueBox->getWidthLabel());
  p->getFrontendConnector()->setWidthSlider(content_->extentValueBox->getWidthSlider());
  p->getFrontendConnector()->setHeightLabel(content_->extentValueBox->getHeightLabel());
  p->getFrontendConnector()->setHeightSlider(content_->extentValueBox->getHeightSlider());
  p->getFrontendConnector()->setDepthLabel(content_->extentValueBox->getDepthLabel());
  p->getFrontendConnector()->setDepthSlider(content_->extentValueBox->getDepthSlider());
  p->getFrontendConnector()->setDiffuseSlider(content_->extentValueBox->getDiffuseSlider());
  p->getFrontendConnector()->setDivergenceButton(content_->extentValueBox->getDivergenceButton());
  p->getFrontendConnector()->setFactorLabel(content_->extentValueBox->getFactorLabel());
  p->getFrontendConnector()->setFactorSlider(content_->extentValueBox->getFactorSlider());
  p->getFrontendConnector()->setRangeLabel(content_->extentValueBox->getRangeLabel());
  p->getFrontendConnector()->setRangeSlider(content_->extentValueBox->getRangeSlider());

  p->getFrontendConnector()->setPannerTopView(content_->panningViewValueBox->getPannerTopView());
  p->getFrontendConnector()->setPannerSideView(content_->panningViewValueBox->getPannerSideView());
  /* clang-format on */

  viewport_->setViewedComponent(content_.get(), false);
  viewport_->setScrollBarsShown(true, true);
  viewport_->getHorizontalScrollBar().setColour(ScrollBar::thumbColourId,
                                                EarColours::Primary);
  viewport_->getVerticalScrollBar().setColour(ScrollBar::thumbColourId,
                                              EarColours::Primary);
  addAndMakeVisible(viewport_.get());

  setResizable(true, false);
  // TODO - old size (with metadata box and divergence) setResizeLimits(0, 0, 726, 950);
  setResizeLimits(0, 0, 726, 672);
  // TODO - old size (with metadata box and divergence) setSize(726, 950);
  setSize(726, 672);
}

ObjectAudioProcessorEditor::~ObjectAudioProcessorEditor() {}

void ObjectAudioProcessorEditor::paint(Graphics& g) {}

void ObjectAudioProcessorEditor::resized() {
  viewport_->setBounds(getLocalBounds());
  // TODO - old size (with metadata box and divergence) content_->setBounds(0, 0, 726, 950);
  content_->setBounds(0, 0, 726, 672);
}
