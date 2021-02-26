#include "binaural_monitoring_plugin_editor.hpp"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "helper/properties_file.hpp"
#include "detail/constants.hpp"
#include "binaural_monitoring_plugin_processor.hpp"
#include "speaker_setups.hpp"

#include <string>

using namespace ear::plugin::ui;

EarMonitoringAudioProcessorEditor::EarMonitoringAudioProcessorEditor(
    EarMonitoringAudioProcessor* p)
    : AudioProcessorEditor(p),
      p_(p),
      header_(std::make_unique<EarHeader>()),
      onBoardingButton_(std::make_unique<EarButton>()),
      onBoardingOverlay_(std::make_unique<Overlay>()),
      onBoardingContent_(std::make_unique<Onboarding>()),
      propertiesFileLock_(
          std::make_unique<InterProcessLock>("EPS_preferences")),
      propertiesFile_(getPropertiesFile(propertiesFileLock_.get())) {

  header_->setText(" Binaural Monitoring");

  onBoardingButton_->setButtonText("?");
  onBoardingButton_->setShape(EarButton::Shape::Circular);
  onBoardingButton_->setFont(
      font::RobotoSingleton::instance().getRegular(20.f));
  onBoardingButton_->onClick = [&]() { onBoardingOverlay_->setVisible(true); };

  onBoardingOverlay_->setContent(onBoardingContent_.get());
  onBoardingOverlay_->setWindowSize(706, 596);
  onBoardingOverlay_->setHeaderText(
      String::fromUTF8("Welcome – do you need help?"));
  onBoardingOverlay_->onClose = [&]() {
    onBoardingOverlay_->setVisible(false);
    propertiesFile_->setValue("showOnboarding", false);
  };
  if (!propertiesFile_->containsKey("showOnboarding")) {
    onBoardingOverlay_->setVisible(true);
  }
  onBoardingContent_->addListener(this);

  addAndMakeVisible(header_.get());
  addAndMakeVisible(onBoardingButton_.get());
  addChildComponent(onBoardingOverlay_.get());

  auto speakers = ear::plugin::speakerSetupByName(SPEAKER_LAYOUT).speakers;
  for (int i = 0; i < speakers.size(); ++i) {
    headphoneMeters_.push_back(std::make_unique<HeadphoneChannelMeter>(
        String(i + 1), speakers.at(i).spLabel));
    headphoneMeters_.back()->getLevelMeter()->setMeter(p->getLevelMeter(), i);
    addAndMakeVisible(headphoneMeters_.back().get());
  }
  setSize(600, 400);
}

EarMonitoringAudioProcessorEditor::~EarMonitoringAudioProcessorEditor() {}

void EarMonitoringAudioProcessorEditor::paint(Graphics& g) {
  g.fillAll(EarColours::Background);
}

void EarMonitoringAudioProcessorEditor::resized() {
  auto area = getLocalBounds();
  onBoardingOverlay_->setBounds(area);
  area.reduce(5, 5);
  auto headingArea = area.removeFromTop(55);
  onBoardingButton_->setBounds(
      headingArea.removeFromRight(39).removeFromBottom(39));
  header_->setBounds(headingArea);

  area.removeFromTop(10); // Padding between header and content

  // All content to go below and to be fitted within `area`

  auto meterArea = area.withHeight(200).reduced(5, 5);
  for (int i = 0; i < 12 && i < headphoneMeters_.size(); ++i) {
    headphoneMeters_.at(i)->setBounds(meterArea.removeFromLeft(50));
    meterArea.removeFromLeft(5);
  }
}

void EarMonitoringAudioProcessorEditor::endButtonClicked(
    Onboarding* onboarding) {
  onBoardingOverlay_->setVisible(false);
  propertiesFile_->setValue("showOnboarding", false);
}
void EarMonitoringAudioProcessorEditor::moreButtonClicked(
    Onboarding* onboarding) {
  onBoardingOverlay_->setVisible(false);
  URL(ear::plugin::detail::MORE_INFO_URL).launchInDefaultBrowser();
}
