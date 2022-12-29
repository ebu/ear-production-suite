#pragma once

#include "object_plugin_processor.hpp"

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/look_and_feel/shadows.hpp"
#include "components/ear_combo_box.hpp"
#include "components/onboarding.hpp"
#include "components/overlay.hpp"
#include "components/ear_header.hpp"
#include "helper/properties_file.hpp"
#include "detail/constants.hpp"
#include "value_box_extent.hpp"
#include "value_box_gain.hpp"
#include "value_box_main.hpp"
#include "value_box_metadata.hpp"
#include "value_box_panning.hpp"
#include "value_box_panning_view.hpp"
#include "components/version_label.hpp"
#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class ObjectsComponent : public Component,
                         private ear::plugin::ui::Onboarding::Listener {
 public:
  ObjectsComponent(ObjectsAudioProcessor* p)
      : p_(p),
        header(std::make_unique<EarHeader>()),
        onBoardingButton(std::make_unique<EarButton>()),
        onBoardingOverlay(std::make_unique<Overlay>()),
        onBoardingContent(std::make_unique<Onboarding>()),
        mainValueBox(std::make_unique<ValueBoxMain>()),
        gainValueBox(std::make_unique<ValueBoxGain>()),
        metadataValueBox(std::make_unique<ValueBoxMetadata>()),
        panningValueBox(std::make_unique<ValueBoxPanning>()),
        extentValueBox(std::make_unique<ValueBoxExtent>()),
        panningViewValueBox(std::make_unique<ValueBoxPanningView>()),
        statusBarLabel(std::make_unique<Label>()),
        propertiesFileLock(
            std::make_unique<InterProcessLock>("EPS_preferences")),
        propertiesFile(getPropertiesFile(propertiesFileLock.get())) {
    header->setText(" Object");
    onBoardingButton->setButtonText("?");
    onBoardingButton->setShape(EarButton::Shape::Circular);
    onBoardingButton->setFont(
        font::RobotoSingleton::instance().getRegular(20.f));
    onBoardingButton->onClick = [&]() { onBoardingOverlay->setVisible(true); };

    onBoardingOverlay->setContent(onBoardingContent.get());
    onBoardingOverlay->setWindowSize(706, 596);
    onBoardingOverlay->setHeaderText(
        String::fromUTF8("Welcome – do you need help?"));
    onBoardingOverlay->onClose = [&]() {
      onBoardingOverlay->setVisible(false);
      propertiesFile->setValue("showOnboarding", false);
    };
    if (!propertiesFile->containsKey("showOnboarding")) {
      onBoardingOverlay->setVisible(true);
    }
    onBoardingContent->addListener(this);

    addAndMakeVisible(header.get());
    addAndMakeVisible(onBoardingButton.get());
    addChildComponent(onBoardingOverlay.get());

    //addAndMakeVisible(metadataValueBox.get());
    addAndMakeVisible(mainValueBox.get());
    addAndMakeVisible(gainValueBox.get());
    addAndMakeVisible(panningValueBox.get());
    addAndMakeVisible(extentValueBox.get());
    addAndMakeVisible(panningViewValueBox.get());

    metadataValueBox->setEnabled(false);
    metadataValueBox->setAlpha(0.38f);

    statusBarLabel->setFont(EarFontsSingleton::instance().Measures);
    addAndMakeVisible(statusBarLabel.get());

    configureVersionLabel(versionLabel);
    addAndMakeVisible(versionLabel);

    panningViewValueBox->getPannerTopView()->setAzimuth(p->getAzimuth()->get(),
                                                        dontSendNotification);
    panningViewValueBox->getPannerSideView()->setElevation(
        p->getElevation()->get(), dontSendNotification);
    panningViewValueBox->getPannerTopView()->setDistance(
        p->getDistance()->get(), dontSendNotification);

    gainValueBox->getLevelMeter()->setMeter(p->getLevelMeter(), 0);

    mainValueBox->getRoutingComboBox()->grabKeyboardFocus();
  }

  ~ObjectsComponent() {}

  void paint(Graphics& g) override {
    g.fillAll(EarColours::Background);

    Shadows::elevation04dp.drawForRectangle(g, mainValueBox->getBounds());
    //Shadows::elevation04dp.drawForRectangle(g, metadataValueBox->getBounds());
    Shadows::elevation01dp.drawForRectangle(g, gainValueBox->getBounds());
    Shadows::elevation01dp.drawForRectangle(g, panningValueBox->getBounds());
    Shadows::elevation01dp.drawForRectangle(g, extentValueBox->getBounds());
    Shadows::elevation01dp.drawForRectangle(g,
                                            panningViewValueBox->getBounds());
  }

  void resized() override {
    auto area = getLocalBounds();
    onBoardingOverlay->setBounds(area);
    area.reduce(5, 5);
    auto headingArea = area.removeFromTop(55);
    auto bottomLabelsArea = area.removeFromBottom(30);
    statusBarLabel->setBounds(bottomLabelsArea.removeFromLeft(bottomLabelsArea.getWidth() / 2));
    versionLabel.setBounds(bottomLabelsArea);
    onBoardingButton->setBounds(
        headingArea.removeFromRight(39).removeFromBottom(39));
    header->setBounds(headingArea);

    auto leftColumn = area.withTrimmedRight(area.getWidth() / 2);
    auto rightColumn = area.withTrimmedLeft(area.getWidth() / 2);

    // left column
    mainValueBox->setBounds(leftColumn.removeFromTop(187).reduced(5, 5));
    gainValueBox->setBounds(leftColumn.removeFromTop(103).reduced(5, 5));
    // TODO - panningValueBox original position - reinstate when metadata implemented
    //panningValueBox->setBounds(leftColumn.removeFromTop(219).reduced(5, 5));
    // TODO - extent component reduced by 135px to hide divergence (currently not implemented)
    extentValueBox->setBounds(leftColumn.removeFromTop(372-135).reduced(5, 5));

    // right column
    // TODO - panningValueBox temp new position whilst metadata not implemented
    panningValueBox->setBounds(rightColumn.removeFromTop(219).reduced(5, 5));
    //metadataValueBox->setBounds(rightColumn.removeFromTop(161).reduced(5, 5));
    panningViewValueBox->setBounds(
        rightColumn.removeFromTop(358).reduced(5, 5));
  }

  void endButtonClicked(Onboarding* onboarding) override {
    onBoardingOverlay->setVisible(false);
    propertiesFile->setValue("showOnboarding", false);
  }
  void moreButtonClicked(Onboarding* onboarding) override {
    onBoardingOverlay->setVisible(false);
    URL(ear::plugin::detail::MORE_INFO_URL).launchInDefaultBrowser();
  }

  std::unique_ptr<ear::plugin::ui::EarHeader> header;
  std::unique_ptr<ear::plugin::ui::EarButton> onBoardingButton;
  std::unique_ptr<ear::plugin::ui::Overlay> onBoardingOverlay;
  std::unique_ptr<ear::plugin::ui::Onboarding> onBoardingContent;
  std::unique_ptr<ear::plugin::ui::ValueBoxMain> mainValueBox;
  std::unique_ptr<ear::plugin::ui::ValueBoxGain> gainValueBox;
  std::unique_ptr<ear::plugin::ui::ValueBoxPanning> panningValueBox;
  std::unique_ptr<ear::plugin::ui::ValueBoxMetadata> metadataValueBox;
  std::unique_ptr<ear::plugin::ui::ValueBoxExtent> extentValueBox;
  std::unique_ptr<ear::plugin::ui::ValueBoxPanningView> panningViewValueBox;
  std::shared_ptr<Label> statusBarLabel;
  Label versionLabel;

  std::unique_ptr<InterProcessLock> propertiesFileLock;
  std::unique_ptr<PropertiesFile> propertiesFile;

 private:
  ObjectsAudioProcessor* p_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ObjectsComponent)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
