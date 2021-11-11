#include "value_box_order_display.hpp"

#include "components/level_meter_calculator.hpp"
#include "components/ear_button.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "order_display_box.hpp"
#include "order_box.hpp"
#include "helper/common_definition_helper.h"

namespace ear {
namespace plugin {
namespace ui {

ValueBoxOrderDisplay::ValueBoxOrderDisplay(
      HoaAudioProcessor* p,
      std::weak_ptr<ear::plugin::LevelMeterCalculator> levelMeterCalculator)
      : levelMeterCalculator_(levelMeterCalculator),
        headingLabel_(std::make_unique<Label>()),
        orderDisplayBox_(std::make_unique<OrderDisplayBox>()),
        resetClippingButton_(std::make_shared<EarButton>()),
        p_(p) {
    headingLabel_->setFont(EarFonts::Heading);
    headingLabel_->setColour(Label::textColourId, EarColours::Heading);
    headingLabel_->setText("HOA Order Display",
                           juce::NotificationType::dontSendNotification);
    headingLabel_->setJustificationType(Justification::bottomLeft);
    addAndMakeVisible(headingLabel_.get());

    resetClippingButton_->setButtonText("Reset Clipping Warnings");
    resetClippingButton_->setShape(EarButton::Shape::Rounded);
    resetClippingButton_->setFont(
        font::RobotoSingleton::instance().getRegular(15.f));
    resetClippingButton_->onClick = [&]() {
      auto levelMeterCalculatorLocked_ = levelMeterCalculator_.lock();
      levelMeterCalculatorLocked_->resetClipping();
    };

    clearHoaSetup();

    addAndMakeVisible(orderDisplayBox_.get());
    addChildComponent(resetClippingButton_.get());

}

ValueBoxOrderDisplay::~ValueBoxOrderDisplay() {}

void ValueBoxOrderDisplay::paint(Graphics& g) {
  g.fillAll(EarColours::Area01dp);
}

void ValueBoxOrderDisplay::resized() {
    auto area = getLocalBounds();
    area.reduce(10, 5);

    auto headingArea = area.withHeight(30);
    headingLabel_->setBounds(headingArea.withWidth(300));
    resetClippingButton_->setBounds(headingArea.withLeft(650));

    area.removeFromTop(30);
    area.removeFromTop(2.f * marginBig_);

    orderDisplayBox_->setBounds(area.reduced(0, 10));
}

void ValueBoxOrderDisplay::clearHoaSetup() {
    orderDisplayBox_->removeAllOrderBoxes();
    orderDisplayBox_->setVisible(true);
}

void ValueBoxOrderDisplay::setHoaType(int hoaId) {
    clearHoaSetup();
    auto commonDefinitionHelper = AdmCommonDefinitionHelper::getSingleton();
    auto elementRelationships =
        commonDefinitionHelper->getElementRelationships();
    auto pfData = commonDefinitionHelper->getPackFormatData(4, hoaId);
    size_t cfCount(0);
    if (pfData) {
      cfCount = static_cast<size_t>(pfData->relatedChannelFormats.size());
    }
    size_t orderCount(ceil(sqrt(cfCount)));

    for (int i = 0; i < orderCount; ++i) {
      std::string ordinal = [&]() {
        std::vector<std::string> suffixes = {"th", "st", "nd", "rd", "th"};
        return suffixes.at(std::min(i % 10, 4));
      }();
      orderBoxes_.push_back(std::make_unique<OrderBox>(
          p_, this,
          std::to_string(i) + ordinal, i, orderCount - 1));
      orderDisplayBox_->addOrderBox(orderBoxes_.back().get());
    }
}
std::shared_ptr<EarButton> ValueBoxOrderDisplay::getResetClippingButton() {
  return resetClippingButton_;
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
