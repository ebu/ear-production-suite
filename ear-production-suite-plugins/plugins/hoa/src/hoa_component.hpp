#pragma once

#include "JuceHeader.h"

#include "components/onboarding.hpp"
#include "components/overlay.hpp"
#include "helper/properties_file.hpp"
#include "components/ear_header.hpp"
#include "detail/constants.hpp"
#include "value_box_main.hpp"
#include "value_box_order_display.hpp"
#include "components/version_label.hpp"

namespace ear {
namespace plugin {
namespace ui {

class HoaComponent : public Component,
                     private ear::plugin::ui::Onboarding::Listener {
 public:
  HoaComponent(HoaAudioProcessor* p)
      : p_(p),
        header(std::make_unique<EarHeader>()),
        onBoardingButton(std::make_unique<EarButton>()),
        onBoardingOverlay(std::make_unique<Overlay>()),
        onBoardingContent(std::make_unique<Onboarding>()),
        mainValueBox(std::make_unique<ValueBoxMain>()),
        orderDisplayValueBox(
            std::make_shared<ValueBoxOrderDisplay>(p, p->getLevelMeter())),
        statusBarLabel(std::make_shared<Label>()),
        propertiesFileLock(
            std::make_unique<InterProcessLock>("EPS_preferences")),
        propertiesFile(getPropertiesFile(propertiesFileLock.get())) {

    header->setName("EarHeader (HoaComponent::header)");
    onBoardingButton->setName("EarButton (HoaComponent::onBoardingButton)");
    onBoardingOverlay->setName("Overlay (HoaComponent::onBoardingOverlay)");
    onBoardingContent->setName("Onboarding (HoaComponent::onBoardingContent)");
    mainValueBox->setName("ValueBoxMain (HoaComponent::mainValueBox)");
    orderDisplayValueBox->setName("ValueBoxOrderDisplay (HoaComponent::orderDisplayValueBox)");
    statusBarLabel->setName("Label (HoaComponent::statusBarLabel)");

    header->setText(" HOA");

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

    addAndMakeVisible(mainValueBox.get());
    addAndMakeVisible(orderDisplayValueBox.get());

    statusBarLabel->setFont(EarFonts::Measures);
    addAndMakeVisible(statusBarLabel.get());

    configureVersionLabel(versionLabel);
    addAndMakeVisible(versionLabel);
  }

  ~HoaComponent() {}

  void paint(Graphics& g) override {
    g.fillAll(EarColours::Background);
    Shadows::elevation04dp.drawForRectangle(g, mainValueBox->getBounds());
  }

  void resized() override {
    auto area = getLocalBounds();
    onBoardingOverlay->setBounds(area);
    area.reduce(5, 5);
    auto headingArea = area.removeFromTop(55);
    auto bottomLabelsArea = area.removeFromBottom(30);
    statusBarLabel->setBounds(
        bottomLabelsArea.removeFromLeft(bottomLabelsArea.getWidth() / 2));
    versionLabel.setBounds(bottomLabelsArea);
    onBoardingButton->setBounds(
        headingArea.removeFromRight(39).removeFromBottom(39));
    header->setBounds(headingArea);

    auto topArea = area.removeFromTop(223);
    auto leftColumn = topArea.withTrimmedRight(area.getWidth() / 2);
    auto rightColumn = topArea.withTrimmedLeft(area.getWidth() / 2);
    mainValueBox->setBounds(leftColumn.reduced(5, 5));
    orderDisplayValueBox->setBounds(area.reduced(5, 5));
  }

  std::unique_ptr<EarHeader> header;
  std::unique_ptr<ear::plugin::ui::EarButton> onBoardingButton;
  std::unique_ptr<ear::plugin::ui::Overlay> onBoardingOverlay;
  std::unique_ptr<ear::plugin::ui::Onboarding> onBoardingContent;

  std::unique_ptr<ear::plugin::ui::ValueBoxMain> mainValueBox;

  std::shared_ptr<ear::plugin::ui::ValueBoxOrderDisplay> orderDisplayValueBox;
  std::shared_ptr<Label> statusBarLabel;
  Label versionLabel;

  std::unique_ptr<InterProcessLock> propertiesFileLock;
  std::unique_ptr<PropertiesFile> propertiesFile;

  // --- Onboarding::Listener
  void endButtonClicked(Onboarding* onboarding) override {
    onBoardingOverlay->setVisible(false);
    propertiesFile->setValue("showOnboarding", false);
  }
  void moreButtonClicked(Onboarding* onboarding) override {
    onBoardingOverlay->setVisible(false);
    URL(detail::MORE_INFO_URL).launchInDefaultBrowser();
  }

 private:
  HoaAudioProcessor* p_;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HoaComponent)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
