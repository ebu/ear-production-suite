#include "binaural_monitoring_plugin_editor.hpp"
#include "orientation_osc.hpp"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/version_label.hpp"
#include "helper/properties_file.hpp"
#include "detail/constants.hpp"
#include "binaural_monitoring_plugin_processor.hpp"
#include "binaural_monitoring_frontend_connector.hpp"

#include <string>

using namespace ear::plugin;
using namespace ear::plugin::ui;

EarBinauralMonitoringAudioProcessorEditor::
    EarBinauralMonitoringAudioProcessorEditor(
        EarBinauralMonitoringAudioProcessor* p)
    : AudioProcessorEditor(p),
      p_(p),
      dataFileValueBox(std::make_shared<ValueBoxDataFile>()),
      oscValueBox(std::make_unique<ValueBoxOsc>()),
      orientationValueBox(std::make_unique<ValueBoxOrientation>()),
      header_(std::make_unique<EarHeader>()),
      onBoardingButton_(std::make_unique<EarButton>()),
      onBoardingOverlay_(std::make_unique<Overlay>()),
      onBoardingContent_(std::make_unique<Onboarding>()),
      errorOverlay_(std::make_shared<BinauralRendererErrorOverlay>()),
      headphoneMeterBox_(std::make_unique<HeadphoneChannelMeterBox>(
          String::fromUTF8("Output"))),
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
  onBoardingOverlay_->setWindowSize(
      700, 550);  // Min size to display contents correctly
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
  addAndMakeVisible(headphoneMeterBox_.get());

  for (int i = 0; i < 2; ++i) {
    headphoneMeters_.push_back(std::make_unique<HeadphoneChannelMeter>(
        String(i + 1), i == 0 ? "L" : "R"));
    headphoneMeters_.back()->getLevelMeter()->setMeter(p->getLevelMeter(), i);
    addAndMakeVisible(headphoneMeters_.back().get());
  }

  configureVersionLabel(versionLabel);
  addAndMakeVisible(versionLabel);

  statusLabel = std::make_shared<Label>();
  statusLabel->setColour(juce::Label::textColourId, ear::plugin::ui::EarColours::Version);
  statusLabel->setText("Status", juce::NotificationType::dontSendNotification);
  statusLabel->setFont(ear::plugin::ui::EarFontsSingleton::instance().Version);
  statusLabel->setJustificationType(juce::Justification::left);
  statusLabel->setEditable(false);
  addAndMakeVisible(statusLabel.get());

  /* clang-format off */
  p->getFrontendConnector()->setRendererStatusLabel(statusLabel);
  p->getFrontendConnector()->setDataFileComponent(dataFileValueBox);
  p->getFrontendConnector()->setDataFileComboBox(dataFileValueBox->getDataFileComboBox());
  p->getFrontendConnector()->setYawView(orientationValueBox->getYawControl());
  p->getFrontendConnector()->setPitchView(orientationValueBox->getPitchControl());
  p->getFrontendConnector()->setRollView(orientationValueBox->getRollControl());
  p->getFrontendConnector()->setOscPortControl(oscValueBox->getPortControl());
  p->getFrontendConnector()->setOscEnableButton(oscValueBox->getEnableButton());
  p->getFrontendConnector()->setOscInvertYawButton(oscValueBox->getInvertYawButton());
  p->getFrontendConnector()->setOscInvertPitchButton(oscValueBox->getInvertPitchButton());
  p->getFrontendConnector()->setOscInvertRollButton(oscValueBox->getInvertRollButton());
  p->getFrontendConnector()->setOscInvertQuatWButton(oscValueBox->getInvertQuatWButton());
  p->getFrontendConnector()->setOscInvertQuatXButton(oscValueBox->getInvertQuatXButton());
  p->getFrontendConnector()->setOscInvertQuatYButton(oscValueBox->getInvertQuatYButton());
  p->getFrontendConnector()->setOscInvertQuatZButton(oscValueBox->getInvertQuatZButton());
  /* clang-format on */

  addAndMakeVisible(dataFileValueBox.get());
  addAndMakeVisible(orientationValueBox.get());
  addAndMakeVisible(oscValueBox.get());
  addChildComponent(errorOverlay_.get());

  oscValueBox->getPortControl()->setValue(p->getOscPort()->get(),
                                          dontSendNotification);
  oscValueBox->getEnableButton()->setToggleState(p->getOscEnable()->get(),
                                                 dontSendNotification);

  orientationValueBox->getYawControl()->setValue(p->getYaw()->get(),
                                                 dontSendNotification);
  orientationValueBox->getPitchControl()->setValue(p->getPitch()->get(),
                                                   dontSendNotification);
  orientationValueBox->getRollControl()->setValue(p->getRoll()->get(),
                                                  dontSendNotification);

  // setSize(750, 600); // Min size to fit onboarding window.
  // setSize(850, 425); // Min size for UI controls
  setSize(850, 600);

  p_->oscReceiver.onInputTypeChange = [this](ListenerOrientationOscReceiver::InputType inputType) {
    oscValueBox->setInputTypeHighlight(inputType);
  };

  p_->oscReceiver.onStatusChange = [this](std::string status) {
    oscValueBox->getStatusLabel()->setText(
        status, juce::NotificationType::dontSendNotification);
  };
  oscValueBox->getStatusLabel()->setText(
      p_->oscReceiver.getStatus(),
      juce::NotificationType::dontSendNotification);
}

EarBinauralMonitoringAudioProcessorEditor::
    ~EarBinauralMonitoringAudioProcessorEditor() {
  p_->oscReceiver.onStatusChange = nullptr;
  p_->oscReceiver.onInputTypeChange = nullptr;
}

void EarBinauralMonitoringAudioProcessorEditor::paint(Graphics& g) {
  g.fillAll(EarColours::Background);

  Shadows::elevation01dp.drawForRectangle(g, orientationValueBox->getBounds());
  Shadows::elevation01dp.drawForRectangle(g, oscValueBox->getBounds());
}

void EarBinauralMonitoringAudioProcessorEditor::resized() {
  auto area = getLocalBounds();
  onBoardingOverlay_->setBounds(area);
  errorOverlay_->setBounds(area);
  auto p = static_cast<EarBinauralMonitoringAudioProcessor*>(p_);
  //errorOverlay_->setVisible(p->rendererError());
  area.reduce(5, 5);
  auto headingArea = area.removeFromTop(55);
  onBoardingButton_->setBounds(
      headingArea.removeFromRight(39).removeFromBottom(39));
  header_->setBounds(headingArea);

  area.removeFromTop(10);  // Padding between header and content

  // TODO: Condition around showing this section
  { 
    dataFileValueBox->setBounds(area.removeFromTop(60).reduced(5, 5));
  }

  auto body = area.removeFromTop(390);
  // All content to go below and to be fitted within `area`
  auto leftColumn = body.removeFromLeft(145);
  auto rightColumn = body.withWidth(695);

  auto meterArea = leftColumn.reduced(5, 5);
  headphoneMeterBox_->setBounds(meterArea);
  meterArea.removeFromTop(40);
  meterArea = meterArea.reduced(15, 15);

  for (int i = 0; i < headphoneMeters_.size(); ++i) {
    headphoneMeters_.at(i)->setBounds(meterArea.removeFromLeft(50));
    meterArea.removeFromLeft(5);
  }

  orientationValueBox->setBounds(rightColumn.removeFromTop(300).reduced(5, 5));
  oscValueBox->setBounds(rightColumn.reduced(5, 5));

  auto footerArea = area.removeFromTop(30);
  versionLabel.setBounds(footerArea.removeFromRight(150));
  statusLabel->setBounds(footerArea);
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
