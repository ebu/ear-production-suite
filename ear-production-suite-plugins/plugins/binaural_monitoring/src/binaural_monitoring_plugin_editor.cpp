#include "binaural_monitoring_plugin_editor.hpp"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "helper/properties_file.hpp"
#include "detail/constants.hpp"
#include "binaural_monitoring_plugin_processor.hpp"

#include <string>

using namespace ear::plugin::ui;

EarBinauralMonitoringAudioProcessorEditor::EarBinauralMonitoringAudioProcessorEditor(
    EarBinauralMonitoringAudioProcessor* p)
    : AudioProcessorEditor(p),
      p_(p),
      /*
      oscValueBox(std::make_unique<ValueBoxOsc>()),
      orientationViewValueBox(std::make_unique<ValueBoxOrientation>()),
      */
      header_(std::make_unique<EarHeader>()),
      onBoardingButton_(std::make_unique<EarButton>()),
      onBoardingOverlay_(std::make_unique<Overlay>()),
      onBoardingContent_(std::make_unique<Onboarding>()),
      propertiesFileLock_(
          std::make_unique<InterProcessLock>("EPS_preferences")),
      propertiesFile_(getPropertiesFile(propertiesFileLock_.get())) {

  /* clang-format off */
  /*
  p->getFrontendConnector()->setOscPortLabel(oscValueBox->getOscPortLabel());
  p->getFrontendConnector()->setOscEnableButton(oscValueBox->getOscEnableButton());
  p->getFrontendConnector()->setOscStatusLabel(oscValueBox->getOscStatusLabel());
  p->getFrontendConnector()->setYawControl(orientationViewValueBox->getYawView());
  p->getFrontendConnector()->setPitchControl(orientationViewValueBox->getPitchView());
  p->getFrontendConnector()->setRollControl(orientationViewValueBox->getRollView());
  */
  /* clang-format on */

  header_->setText(" Binaural Monitoring");

  onBoardingButton_->setButtonText("?");
  onBoardingButton_->setShape(EarButton::Shape::Circular);
  onBoardingButton_->setFont(
      font::RobotoSingleton::instance().getRegular(20.f));
  onBoardingButton_->onClick = [&]() { onBoardingOverlay_->setVisible(true); };

  onBoardingOverlay_->setContent(onBoardingContent_.get());
  onBoardingOverlay_->setWindowSize(700, 550); // Min size to display contents correctly
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
  //addAndMakeVisible(onBoardingButton_.get()); // TODO: Commented out for now as doesn't fit in MVP window
  addChildComponent(onBoardingOverlay_.get());

  for (int i = 0; i < 2; ++i) {
    headphoneMeters_.push_back(std::make_unique<HeadphoneChannelMeter>(
        String(i + 1), i == 0? "L" : "R"));
    headphoneMeters_.back()->getLevelMeter()->setMeter(p->getLevelMeter(), i);
    addAndMakeVisible(headphoneMeters_.back().get());
  }

  test = std::make_unique<OrientationView>();
  addAndMakeVisible(test.get());


  /*
  addAndMakeVisible(oscValueBox.get());
  addAndMakeVisible(orientationViewValueBox.get());

  oscValueBox->getOscPortLabel()->setText(p->getOscPort()->get(), dontSendNotification);
  oscValueBox->getOscEnableButton()->setValue(p->getOscEnable()->get(), dontSendNotification);
  oscValueBox->getOscStatusLabel()->setText(p->getOscStatus(), dontSendNotification);
  orientationViewValueBox->getYawView()->setYaw(p->getYaw()->get(), dontSendNotification);
  orientationViewValueBox->getPitchView()->setPitch(p->getPitch()->get(), dontSendNotification);
  orientationViewValueBox->getRollView()->setRoll(p->getRoll()->get(), dontSendNotification);
  */

  //setSize(750, 600); // Min size to fit onboarding window.
  //setSize(380, 280); // ideal size for MVP (meters only) but doesn't allow enough space for onboarding window
  setSize(580, 480);
}

EarBinauralMonitoringAudioProcessorEditor::~EarBinauralMonitoringAudioProcessorEditor() {}

void EarBinauralMonitoringAudioProcessorEditor::paint(Graphics& g) {
  g.fillAll(EarColours::Background);

  /*
  Shadows::elevation01dp.drawForRectangle(g, oscValueBox->getBounds());
  Shadows::elevation01dp.drawForRectangle(g, orientationViewValueBox->getBounds());
  */
}

void EarBinauralMonitoringAudioProcessorEditor::resized() {
  auto area = getLocalBounds();
  onBoardingOverlay_->setBounds(area);
  area.reduce(5, 5);
  auto headingArea = area.removeFromTop(55);
  onBoardingButton_->setBounds(
      headingArea.removeFromRight(39).removeFromBottom(39));
  header_->setBounds(headingArea);

  area.removeFromTop(10); // Padding between header and content
  // All content to go below and to be fitted within `area`
  auto leftColumn = area.withWidth(120);
  auto rightColumn = area.withTrimmedLeft(120);

  auto meterArea = leftColumn.withHeight(200).reduced(5, 5);
  for (int i = 0; i < headphoneMeters_.size(); ++i) {
    headphoneMeters_.at(i)->setBounds(meterArea.removeFromLeft(50));
    meterArea.removeFromLeft(5);
  }

  test->setBounds(rightColumn.withHeight(400).withWidth(400));

  /*
  orientationViewValueBox->setBounds(rightColumn.removeFromTop(200).reduced(5, 5));
  oscValueBox->setBounds(rightColumn.removeFromTop(100).reduced(5, 5));
  */
}

void EarBinauralMonitoringAudioProcessorEditor::endButtonClicked(
    Onboarding* onboarding) {
  onBoardingOverlay_->setVisible(false);
  propertiesFile_->setValue("showOnboarding", false);
}
void EarBinauralMonitoringAudioProcessorEditor::moreButtonClicked(
    Onboarding* onboarding) {
  onBoardingOverlay_->setVisible(false);
  URL(ear::plugin::detail::MORE_INFO_URL).launchInDefaultBrowser();
}
