#include "monitoring_plugin_editor.hpp"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/version_label.hpp"
#include "helper/properties_file.hpp"
#include <helper/common_definition_helper.h>
#include "detail/constants.hpp"
#include "monitoring_plugin_processor.hpp"
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
      speakerMeterBoxTop_(
          std::make_unique<SpeakerMeterBox>(String::fromUTF8("1–12"))),
      speakerMeterBoxBottom_(
          std::make_unique<SpeakerMeterBox>(String::fromUTF8("13–24"))),
      propertiesFileLock_(
          std::make_unique<InterProcessLock>("EPS_preferences")),
      propertiesFile_(getPropertiesFile(propertiesFileLock_.get())) {
  String headingText = String::fromUTF8(" Monitoring – ");
  headingText += SPEAKER_LAYOUT_NAME;
  headingText += " (";
  headingText += SPEAKER_LAYOUT;
  headingText += ")";
  header_->setText(headingText.toStdString());

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

  configureVersionLabel(versionLabel);
  addAndMakeVisible(versionLabel);

  addAndMakeVisible(speakerMeterBoxTop_.get());
  addAndMakeVisible(speakerMeterBoxBottom_.get());

  auto apfid = adm::parseAudioPackFormatId(AUDIO_PACK_FORMAT_ID);
  auto idValue = apfid.get<adm::AudioPackFormatIdValue>().get();
  auto typeDef = apfid.get<adm::TypeDescriptor>().get();
  auto pfData = AdmCommonDefinitionHelper::getSingleton()->getPackFormatData(
      typeDef, idValue);
  if (pfData) {
    for (int i = 0; i < pfData->relatedChannelFormats.size(); ++i) {
      auto cfData = pfData->relatedChannelFormats[i];
      auto spLabel = cfData->legacySpeakerLabel.has_value()
                         ? cfData->legacySpeakerLabel.value()
                         : std::string();
      auto ituLabel = cfData->ituLabel.has_value()
                         ? cfData->ituLabel.value()
                         : std::string();
      speakerMeters_.push_back(std::make_unique<SpeakerMeter>(
          String(i + 1), spLabel, ituLabel));
      speakerMeters_.back()->getLevelMeter()->setMeter(p->getLevelMeter(), i);
      addAndMakeVisible(speakerMeters_.back().get());
    }
  }
  setSize(735, 655);
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

  area.removeFromTop(10);
  auto bottomLabelsArea = area.removeFromBottom(30);
  versionLabel.setBounds(bottomLabelsArea);

  auto speakerMeterBoxHeight = area.getHeight() / 2;
  auto topArea = area.removeFromTop(speakerMeterBoxHeight).reduced(5, 5);
  speakerMeterBoxTop_->setBounds(topArea);
  auto bottomArea = area.removeFromTop(speakerMeterBoxHeight).reduced(5, 5);
  speakerMeterBoxBottom_->setBounds(bottomArea);
  topArea.removeFromTop(45);
  topArea.removeFromLeft(30);
  topArea.removeFromBottom(15);
  for (int i = 0; i < 12 && i < speakerMeters_.size(); ++i) {
    speakerMeters_.at(i)->setBounds(topArea.removeFromLeft(50));
    topArea.removeFromLeft(5);
  }
  bottomArea.removeFromTop(45);
  bottomArea.removeFromLeft(30);
  bottomArea.removeFromBottom(15);
  for (int i = 12; i < 24 && i < speakerMeters_.size(); ++i) {
    speakerMeters_.at(i)->setBounds(bottomArea.removeFromLeft(50));
    bottomArea.removeFromLeft(5);
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
