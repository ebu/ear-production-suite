#pragma once

#include "JuceHeader.h"

#include "components/ear_slider.hpp"
#include "components/level_meter.hpp"
#include "components/ear_button.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/look_and_feel/slider.hpp"
#include "order_display_box.hpp"
//#include "speaker_setups.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ValueBoxOrderDisplay : public Component {
 public:
  ValueBoxOrderDisplay(
      HoaAudioProcessor* p,
      std::weak_ptr<ear::plugin::LevelMeterCalculator> levelMeter)
      : levelMeter_(levelMeter),//NOT sure about whether this needs to be kept
        headingLabel_(std::make_unique<Label>()),
        orderDisplayBox_(std::make_unique<OrderDisplayBox>()),
        resetClippingButton(std::make_unique<EarButton>()),
        p_(p)  {
    headingLabel_->setFont(EarFonts::Heading);
    headingLabel_->setColour(Label::textColourId, EarColours::Heading);
    headingLabel_->setText("HOA Order Display",
                           juce::NotificationType::dontSendNotification);
    headingLabel_->setJustificationType(Justification::bottomLeft);
    addAndMakeVisible(headingLabel_.get());

    resetClippingButton->setButtonText("Reset Clipping Warnings");
    resetClippingButton->setShape(EarButton::Shape::Rounded);
    resetClippingButton->setFont(
        font::RobotoSingleton::instance().getRegular(15.f));
    resetClippingButton->onClick = [&]() {
      auto levelMeterCalculatorLocked_ = levelMeter_.lock();
      levelMeterCalculatorLocked_->resetClipping();
    };
    addAndMakeVisible(resetClippingButton.get());

    clearHoaSetup();

    addAndMakeVisible(orderDisplayBox_.get());
  }

  ~ValueBoxOrderDisplay() {}

  void paint(Graphics& g) override {
    g.fillAll(EarColours::Area01dp);
  }

  void resized() override {//Here we sort out what is inside the channel gain box (e.g. all the different meters)
    auto area = getLocalBounds();
    area.reduce(10, 5);

    auto headingArea = area.withHeight(30);
    headingLabel_->setBounds(headingArea.withWidth(300));
    resetClippingButton->setBounds(headingArea.withLeft(600));


    area.removeFromTop(30);
    area.removeFromTop(2.f * marginBig_);

    orderDisplayBox_->setBounds(
        area.reduced(0, 10));
  }

  void clearHoaSetup() {
    orderDisplayBox_->removeAllOrderBoxes();
    orderDisplayBox_->setVisible(true);
  }

  void setHoaType(int hoaId) {
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
      std::string ordinal = [&](){
        std::vector<std::string> suffixes = {"th", "st", "nd", "rd", "th"};
        return suffixes.at(std::min(i % 10,4));
      }();
      orderBoxes_.push_back(std::make_unique<OrderBox>(p_,//this,
          std::to_string(i) + ordinal, i, orderCount-1));
      orderDisplayBox_->addOrderBox(orderBoxes_.back().get()); 
      //orderBoxes_.back()->getLevelMeter()->setMeter(levelMeter_, i);
    }
    
  }

 private:
  void linkChannels() {

  }

  void unlinkChannels() {

    float lastValue = 0.f;

  }
  HoaAudioProcessor* p_;
  std::weak_ptr<ear::plugin::LevelMeterCalculator> levelMeter_;

  std::unique_ptr<Label> headingLabel_;

  std::vector<std::unique_ptr<ear::plugin::ui::OrderBox>> orderBoxes_;//DO WE WANT THIS HERE OF IN ORDER DISPLAY BOX, SURELY NOT BOTH?
  std::unique_ptr<ear::plugin::ui::OrderDisplayBox> orderDisplayBox_;
  std::unique_ptr<ear::plugin::ui::EarButton> resetClippingButton;

  const float labelWidth_ = 71.f;
  const float labelPaddingBottom_ = 0.f;
  const float sliderHeight_ = 40.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxOrderDisplay)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
