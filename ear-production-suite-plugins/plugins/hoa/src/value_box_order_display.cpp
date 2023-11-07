#include "value_box_order_display.hpp"

#include "components/level_meter_calculator.hpp"
#include "components/ear_button.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "order_display_box.hpp"
#include "order_box.hpp"
#include "helper/adm_preset_definitions_helper.h"

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
  headingLabel_->setName("Label (ValueBoxOrderDisplay::headingLabel_)");
  orderDisplayBox_->setName(
      "OrderDisplayBox (ValueBoxOrderDisplay::orderDisplayBox_)");
  resetClippingButton_->setName(
      "EarButton (ValueBoxOrderDisplay::resetClippingButton_)");

  headingLabel_->setFont(EarFontsSingleton::instance().Heading);
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
  area.reduce(marginBig_, 0);

  auto headingArea = area.removeFromTop(30 + marginSmall_);
  headingLabel_->setBounds(headingArea.withWidth(300));
  headingArea.removeFromTop(marginBig_);
  resetClippingButton_->setBounds(headingArea.withLeft(500));

  area.removeFromTop(marginBig_ + marginSmall_);
  area.removeFromBottom(marginBig_);
  orderDisplayBox_->setBounds(area.reduced(0, 0));
}

void ValueBoxOrderDisplay::clearHoaSetup() {
  orderDisplayBox_->removeAllOrderBoxes();
  orderDisplayBox_->setVisible(true);
}

void ValueBoxOrderDisplay::setHoaType(int hoaId) {
  clearHoaSetup();
  auto commonDefinitionHelper = AdmPresetDefinitionsHelper::getSingleton();
  auto elementRelationships = commonDefinitionHelper->getElementRelationships();
  auto pfData = commonDefinitionHelper->getPackFormatData(4, hoaId);
  size_t cfCount(0);
  if (pfData) {
    cfCount = static_cast<size_t>(pfData->relatedChannelFormats.size());
  }
  size_t orderCount(ceil(sqrt(cfCount)));

  orderDisplayBox_->addOrderBoxes(p_, this, orderCount);
}

std::shared_ptr<EarButton> ValueBoxOrderDisplay::getResetClippingButton() {
  return resetClippingButton_;
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
