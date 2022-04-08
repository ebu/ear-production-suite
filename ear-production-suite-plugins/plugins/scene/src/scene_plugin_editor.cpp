#include "scene_plugin_editor.hpp"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "helper/properties_file.hpp"
#include "detail/constants.hpp"
#include "scene_plugin_processor.hpp"
#include "components/version_label.hpp"

#include <memory>
#include <limits>

using namespace ear::plugin::ui;

SceneAudioProcessorEditor::SceneAudioProcessorEditor(SceneAudioProcessor* p)
    : AudioProcessorEditor(p),
      p_(p),
      header_(std::make_unique<EarHeader>()),
      onBoardingButton_(std::make_unique<EarButton>()),
      onBoardingOverlay_(std::make_unique<Overlay>()),
      onBoardingContent_(std::make_unique<Onboarding>()),
      itemsOverlay_(std::make_shared<Overlay>()),
      itemsContainer_(std::make_shared<ItemsContainer>(p_->metadata())),
      programmesContainer_(std::make_shared<ProgrammesContainer>(
              p_->getLevelMeter().lock(),
              p_->metadata())),
      autoModeOverlay_(std::make_shared<AutoModeOverlay>(p->metadata())),
      multipleScenePluginsOverlay_(
          std::make_shared<MultipleScenePluginsOverlay>()),
      propertiesFileLock_(
          std::make_unique<InterProcessLock>("EPS_preferences")),
      propertiesFile_(getPropertiesFile(propertiesFileLock_.get())) {
  header_->setText(" Scene");

  onBoardingButton_->setButtonText("?");
  onBoardingButton_->setShape(EarButton::Shape::Circular);
  onBoardingButton_->setFont(
      font::RobotoSingleton::instance().getRegular(20.f));
  onBoardingButton_->onClick = [&]() { onBoardingOverlay_->setVisible(true); };

  onBoardingOverlay_->setHeaderText(
      String::fromUTF8("Welcome – do you need help?"));
  onBoardingOverlay_->setContent(onBoardingContent_.get());
  onBoardingOverlay_->setWindowSize(706, 596);
  onBoardingOverlay_->onClose = [&]() {
    onBoardingOverlay_->setVisible(false);
    propertiesFile_->setValue("showOnboarding", false);
  };
  if (!propertiesFile_->containsKey("showOnboarding")) {
    onBoardingOverlay_->setVisible(true);
  }
  onBoardingContent_->addListener(this);

  itemsOverlay_->setHeaderText(String::fromUTF8("+ Item"));
  itemsOverlay_->setContent(itemsContainer_.get());
  itemsOverlay_->setWindowSize(706, 596);

  itemsContainer_->addListener(this);
  p_->metadata().addUIListener(itemsContainer_);
  programmesContainer_->addListener(this);
  p_->metadata().addUIListener(programmesContainer_);
  p_->metadata().addUIListener(autoModeOverlay_);
  p_->metadata().addUIListener(multipleScenePluginsOverlay_);

  addAndMakeVisible(header_.get());
  addAndMakeVisible(onBoardingButton_.get());
  addChildComponent(onBoardingOverlay_.get());
  addChildComponent(itemsOverlay_.get());
  addAndMakeVisible(programmesContainer_.get());
  addChildComponent(autoModeOverlay_.get());
  addChildComponent(multipleScenePluginsOverlay_.get());

  configureVersionLabel(versionLabel);
  addAndMakeVisible(versionLabel);

  p_->metadata().refresh();

  setResizable(true, false);
  setResizeLimits(1100, 620, std::numeric_limits<int>::max(),
                  std::numeric_limits<int>::max());
  setSize(1280, 880);
}

SceneAudioProcessorEditor::~SceneAudioProcessorEditor() = default;

void SceneAudioProcessorEditor::paint(Graphics& g) {
  g.fillAll(EarColours::Background);
}

void SceneAudioProcessorEditor::resized() {
  auto area = getLocalBounds();
  autoModeOverlay_->setBounds(area);
  multipleScenePluginsOverlay_->setBounds(area);
  onBoardingOverlay_->setBounds(area);
  itemsOverlay_->setBounds(area);
  area.reduce(5, 5);
  auto headingArea = area.removeFromTop(55);
  onBoardingButton_->setBounds(
      headingArea.removeFromRight(39).removeFromBottom(39));
  header_->setBounds(headingArea);
  area.removeFromTop(10);
  auto bottomLabelsArea = area.removeFromBottom(30);
  versionLabel.setBounds(bottomLabelsArea);
  programmesContainer_->setBounds(area);
}

void SceneAudioProcessorEditor::endButtonClicked(Onboarding*) {
  onBoardingOverlay_->setVisible(false);
  propertiesFile_->setValue("showOnboarding", false);
}
void SceneAudioProcessorEditor::moreButtonClicked(Onboarding*) {
  onBoardingOverlay_->setVisible(false);
  URL(ear::plugin::detail::MORE_INFO_URL).launchInDefaultBrowser();
}

void SceneAudioProcessorEditor::addItemsClicked(ear::plugin::ui::ItemsContainer*,
                     std::vector<ear::plugin::communication::ConnectionId>) {
    itemsOverlay_->setVisible(false);
}

void SceneAudioProcessorEditor::addItemClicked() {
    itemsOverlay_->setVisible(true);
}