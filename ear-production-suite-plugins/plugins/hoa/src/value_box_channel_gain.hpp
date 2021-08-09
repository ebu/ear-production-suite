#pragma once

#include "JuceHeader.h"

#include "components/ear_slider.hpp"
#include "components/level_meter.hpp"
#include "components/ear_button.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/look_and_feel/slider.hpp"
#include "channel_gains_box.hpp"
//#include "speaker_setups.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ValueBoxChannelGain : public Component {
 public:
  ValueBoxChannelGain(
      std::weak_ptr<ear::plugin::LevelMeterCalculator> levelMeter)
      : levelMeter_(levelMeter),
        headingLabel_(std::make_unique<Label>()),
        channels1to12Button_(std::make_unique<EarButton>()),
        channels13to24Button_(std::make_unique<EarButton>()),
        channels25to36Button_(std::make_unique<EarButton>()),
        channels37to48Button_(std::make_unique<EarButton>()),
        channels49to60Button_(std::make_unique<EarButton>()),
        /*channels1to6Button_(std::make_unique<EarButton>()),
        channels7to12Button_(std::make_unique<EarButton>()),
        channels13to18Button_(std::make_unique<EarButton>()),
        channels19to24Button_(std::make_unique<EarButton>()),*/
        channelGainsBox1to12_(std::make_unique<ChannelGainsBox>()),
        channelGainsBox13to24_(std::make_unique<ChannelGainsBox>()),
        channelGainsBox25to36_(std::make_unique<ChannelGainsBox>()),
        channelGainsBox37to48_(std::make_unique<ChannelGainsBox>()),
        channelGainsBox49to60_(std::make_unique<ChannelGainsBox>())/*,
        channelGainsBox1to6_(std::make_unique<ChannelGainsBox>()),
        channelGainsBox7to12_(std::make_unique<ChannelGainsBox>()),
        channelGainsBox13to18_(std::make_unique<ChannelGainsBox>()),
        channelGainsBox19to24_(std::make_unique<ChannelGainsBox>()),*/
        /*channelLinkButton_(std::make_unique<EarButton>())*/ {
    headingLabel_->setFont(EarFonts::Heading);
    headingLabel_->setColour(Label::textColourId, EarColours::Heading);
    headingLabel_->setText("Channel Gain",
                           juce::NotificationType::dontSendNotification);
    headingLabel_->setJustificationType(Justification::bottomLeft);
    addAndMakeVisible(headingLabel_.get());

    channels1to12Button_->setButtonText(String::fromUTF8("1–12"));
    channels13to24Button_->setButtonText(String::fromUTF8("13–24"));
    channels25to36Button_->setButtonText(String::fromUTF8("25–36"));
    channels37to48Button_->setButtonText(String::fromUTF8("37-48"));
    channels49to60Button_->setButtonText(String::fromUTF8("49-60"));
    /*channels1to6Button_->setButtonText(String::fromUTF8("1–6"));
    channels7to12Button_->setButtonText(String::fromUTF8("7–12"));
    channels13to18Button_->setButtonText(String::fromUTF8("13–18"));
    channels19to24Button_->setButtonText(String::fromUTF8("19–24"));*/

    clearHoaSetup();

    channels1to12Button_->onClick = [this]() { this->selectChannelGainsTab(0); };
    channels13to24Button_->onClick = [this]() {      this->selectChannelGainsTab(1);    };
    channels25to36Button_->onClick = [this]() {
      this->selectChannelGainsTab(2);
    };
    channels37to48Button_->onClick = [this]() {
      this->selectChannelGainsTab(3);
    };
    channels49to60Button_->onClick = [this]() {
      this->selectChannelGainsTab(4);
    };
    /*channels1to6Button_->onClick = [this]() {
      this->selectChannelGainsTab(0);
    };
    channels7to12Button_->onClick = [this]() {
      this->selectChannelGainsTab(1);
    };
    channels13to18Button_->onClick = [this]() {
      this->selectChannelGainsTab(2);
    };
    channels19to24Button_->onClick = [this]() {
      this->selectChannelGainsTab(3);
    };*/

    addAndMakeVisible(channels1to12Button_.get());
    addAndMakeVisible(channels13to24Button_.get());
    addAndMakeVisible(channels25to36Button_.get());
    addAndMakeVisible(channels37to48Button_.get());
    addAndMakeVisible(channels49to60Button_.get()); /*
    addAndMakeVisible(channels1to6Button_.get());
    addAndMakeVisible(channels7to12Button_.get());
    addAndMakeVisible(channels13to18Button_.get());
    addAndMakeVisible(channels19to24Button_.get());*/

    addAndMakeVisible(channelGainsBox1to12_.get());
    addChildComponent(channelGainsBox13to24_.get());
    addChildComponent(channelGainsBox25to36_.get());
    addChildComponent(channelGainsBox37to48_.get());
    addChildComponent(channelGainsBox49to60_.get()); /*
    addAndMakeVisible(channelGainsBox1to6_.get());
    addChildComponent(channelGainsBox7to12_.get());
    addChildComponent(channelGainsBox13to18_.get());
    addChildComponent(channelGainsBox19to24_.get());*/

    /*channelLinkButton_->setButtonText("Channel Link");
    channelLinkButton_->setShape(EarButton::Shape::Toggle);
    channelLinkButton_->setClickingTogglesState(true);
    channelLinkButton_->onClick = [this] { this->toggleChannelLink(); };
    channelLinkButton_->setEnabled(false);
    channelLinkButton_->setAlpha(Emphasis::disabled);

    addAndMakeVisible(channelLinkButton_.get());*/
  }

  ~ValueBoxChannelGain() {}

  void paint(Graphics& g) override {
    g.fillAll(EarColours::Area01dp);
  }

  void resized() override {
    auto area = getLocalBounds();
    area.reduce(10, 5);
    headingLabel_->setBounds(area.removeFromTop(30));
    area.removeFromTop(2.f * marginBig_);

    auto channelsButtonArea =
        area.removeFromTop(40).withSizeKeepingCentre(260, 40);
    channels1to12Button_->setBounds(
        channelsButtonArea.removeFromLeft(65).reduced(5, 5));
    channels13to24Button_->setBounds(
        channelsButtonArea.removeFromLeft(65).reduced(5, 5));
    channels25to36Button_->setBounds(
        channelsButtonArea.removeFromLeft(65).reduced(5, 5));
    channels37to48Button_->setBounds(
        channelsButtonArea.removeFromLeft(65).reduced(5, 5));
    channels49to60Button_->setBounds(
        channelsButtonArea.removeFromLeft(65).reduced(5, 5)); /*
    channels1to6Button_->setBounds(
        channelsButtonArea.removeFromLeft(65).reduced(5, 5));
    channels7to12Button_->setBounds(
        channelsButtonArea.removeFromLeft(65).reduced(5, 5));
    channels13to18Button_->setBounds(
        channelsButtonArea.removeFromLeft(65).reduced(5, 5));
    channels19to24Button_->setBounds(
        channelsButtonArea.removeFromLeft(65).reduced(5, 5));*/

    //channelLinkButton_->setBounds(area.removeFromBottom(40));
    channelGainsBox1to12_->setBounds(area.reduced(0, 10));
    channelGainsBox13to24_->setBounds(area.reduced(0, 10));
    channelGainsBox25to36_->setBounds(area.reduced(0, 10));
    channelGainsBox37to48_->setBounds(area.reduced(0, 10));
    channelGainsBox49to60_->setBounds(area.reduced(0, 10)); /*
    channelGainsBox1to6_->setBounds(area.reduced(0, 10));
    channelGainsBox7to12_->setBounds(area.reduced(0, 10));
    channelGainsBox13to18_->setBounds(area.reduced(0, 10));
    channelGainsBox19to24_->setBounds(area.reduced(0, 10));*/
  }

  void clearHoaSetup() {
    channelGainsBox1to12_->removeAllChannelGains();
    channelGainsBox13to24_->removeAllChannelGains();
    channelGainsBox25to36_->removeAllChannelGains();
    channelGainsBox37to48_->removeAllChannelGains();
    channelGainsBox49to60_->removeAllChannelGains(); /*
    channelGainsBox1to6_->removeAllChannelGains();
    channelGainsBox7to12_->removeAllChannelGains();
    channelGainsBox13to18_->removeAllChannelGains();
    channelGainsBox19to24_->removeAllChannelGains();*/

    channels1to12Button_->setEnabled(false);
    channels1to12Button_->setAlpha(Emphasis::disabled);
    channels13to24Button_->setEnabled(false);
    channels13to24Button_->setAlpha(Emphasis::disabled);
    channels25to36Button_->setEnabled(false);
    channels25to36Button_->setAlpha(Emphasis::disabled);
    channels37to48Button_->setEnabled(false);
    channels37to48Button_->setAlpha(Emphasis::disabled);
    channels49to60Button_->setEnabled(false);
    channels49to60Button_->setAlpha(Emphasis::disabled); /*
    channels1to6Button_->setEnabled(false);
    channels1to6Button_->setAlpha(Emphasis::disabled);
    channels7to12Button_->setEnabled(false);
    channels7to12Button_->setAlpha(Emphasis::disabled);
    channels13to18Button_->setEnabled(false);
    channels13to18Button_->setAlpha(Emphasis::disabled);
    channels19to24Button_->setEnabled(false);
    channels19to24Button_->setAlpha(Emphasis::disabled);*/

    channels1to12Button_->setToggleState(false, dontSendNotification);
    channels13to24Button_->setToggleState(false, dontSendNotification);
    channels25to36Button_->setToggleState(false, dontSendNotification);
    channels37to48Button_->setToggleState(false, dontSendNotification);
    channels49to60Button_->setToggleState(false, dontSendNotification); /*
    channels1to6Button_->setToggleState(false, dontSendNotification);
    channels7to12Button_->setToggleState(false, dontSendNotification);
    channels13to18Button_->setToggleState(false, dontSendNotification);
    channels19to24Button_->setToggleState(false, dontSendNotification);*/
    channelGainsBox1to12_->setVisible(true);
    channelGainsBox13to24_->setVisible(false);
    channelGainsBox25to36_->setVisible(false);
    channelGainsBox37to48_->setVisible(false);
    channelGainsBox49to60_->setVisible(false); /*
    channelGainsBox1to6_->setVisible(true);
    channelGainsBox7to12_->setVisible(false);
    channelGainsBox13to18_->setVisible(false);
    channelGainsBox19to24_->setVisible(false);*/

    //channelLinkButton_->setEnabled(false);
    //channelLinkButton_->setAlpha(Emphasis::disabled);
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

    for (int i = 0; i < cfCount; ++i) {
      channelGains_.push_back(
          std::make_unique<ChannelGain>(std::to_string(i+1)));
      channelGains_.back()->getLevelMeter()->setMeter(levelMeter_, i);
      if (i < 12) {
        channelGainsBox1to12_->addChannelGain(channelGains_.back().get());
      } else if (i<24){
        channelGainsBox13to24_->addChannelGain(channelGains_.back().get());
      } else if (i < 36) {
        channelGainsBox25to36_->addChannelGain(channelGains_.back().get());
      } else if (i < 48) {
        channelGainsBox37to48_->addChannelGain(channelGains_.back().get());
      } else  {
        channelGainsBox49to60_->addChannelGain(channelGains_.back().get());
      } /*
      if (i < 6) {
        channelGainsBox1to6_->addChannelGain(channelGains_.back().get());
      } else if (i < 12) {
        channelGainsBox7to12_->addChannelGain(channelGains_.back().get());
      } else  if (i < 18) {
        channelGainsBox13to18_->addChannelGain(channelGains_.back().get());
      } else {
        channelGainsBox19to24_->addChannelGain(channelGains_.back().get());
      }*/
    }
    channels1to12Button_->setEnabled(true);
    channels1to12Button_->setAlpha(Emphasis::full);
    if (cfCount > 12) {
      channels13to24Button_->setEnabled(true);
      channels13to24Button_->setAlpha(Emphasis::full);
    }
    if (cfCount > 24) {
      channels25to36Button_->setEnabled(true);
      channels25to36Button_->setAlpha(Emphasis::full);
    }
    if (cfCount > 36) {
      channels37to48Button_->setEnabled(true);
      channels37to48Button_->setAlpha(Emphasis::full);
    }
    if (cfCount > 48) {
      channels49to60Button_->setEnabled(true);
      channels49to60Button_->setAlpha(Emphasis::full);
    } /*
    channels1to6Button_->setEnabled(true);
    channels1to6Button_->setAlpha(Emphasis::full);
    if (cfCount > 6) {
      channels7to12Button_->setEnabled(true);
      channels7to12Button_->setAlpha(Emphasis::full);
    }
    if (cfCount > 12) {
      channels13to18Button_->setEnabled(true);
      channels13to18Button_->setAlpha(Emphasis::full);
    }
    if (cfCount > 18) {
      channels19to24Button_->setEnabled(true);
      channels19to24Button_->setAlpha(Emphasis::full);
    }*/

    //channelLinkButton_->setEnabled(true);
    //channelLinkButton_->setAlpha(Emphasis::full);
    //linkChannels();

    selectChannelGainsTab(0);
  }

  void selectChannelGainsTab(int tabIndex) {
    channels1to12Button_->setToggleState(false, dontSendNotification);
    channels13to24Button_->setToggleState(false, dontSendNotification);
    channels25to36Button_->setToggleState(false, dontSendNotification);
    channels37to48Button_->setToggleState(false, dontSendNotification);
    channels49to60Button_->setToggleState(false, dontSendNotification);
    channelGainsBox1to12_->setVisible(false);
    channelGainsBox13to24_->setVisible(false);
    channelGainsBox25to36_->setVisible(false);
    channelGainsBox37to48_->setVisible(false);
    channelGainsBox49to60_->setVisible(false);
    /*
    channels1to6Button_->setToggleState(false, dontSendNotification);
    channels7to12Button_->setToggleState(false, dontSendNotification);
    channels13to18Button_->setToggleState(false, dontSendNotification);
    channels19to24Button_->setToggleState(false, dontSendNotification);
    channelGainsBox1to6_->setVisible(false);
    channelGainsBox7to12_->setVisible(false);
    channelGainsBox13to18_->setVisible(false);
    channelGainsBox19to24_->setVisible(false);*/

    switch (tabIndex) {
      case 0:
        channels1to12Button_->setToggleState(true, dontSendNotification);
        channelGainsBox1to12_->setVisible(true);
        break;
      case 1:
        channels13to24Button_->setToggleState(true, dontSendNotification);
        channelGainsBox13to24_->setVisible(true);
        break;
      case 2:
        channels25to36Button_->setToggleState(true, dontSendNotification);
        channelGainsBox25to36_->setVisible(true);
        break;
      case 3:
        channels37to48Button_->setToggleState(true, dontSendNotification);
        channelGainsBox37to48_->setVisible(true);
        break;
      case 4:
        channels49to60Button_->setToggleState(true, dontSendNotification);
        channelGainsBox49to60_->setVisible(true);
        break; /*
      case 0:
        channels1to6Button_->setToggleState(true, dontSendNotification);
        channelGainsBox1to6_->setVisible(true);
        break;
      case 1:
        channels7to12Button_->setToggleState(true, dontSendNotification);
        channelGainsBox7to12_->setVisible(true);
        break;
      case 2:
        channels13to18Button_->setToggleState(true, dontSendNotification);
        channelGainsBox13to18_->setVisible(true);
        break;
      case 3:
        channels19to24Button_->setToggleState(true, dontSendNotification);
        channelGainsBox19to24_->setVisible(true);
        break;*/
      default:
        break;
    }
  }

  void toggleChannelLink() {
    //if (channelLinkButton_->getToggleState()) {
     // linkChannels();
    //} else {
    //  unlinkChannels();
    //}
  }

 private:
  void linkChannels() {
    //channelLinkButton_->setToggleState(true, dontSendNotification);
    for (const auto& channelGain : channelGains_) {
      channelGain->getGainSlider()->getValueObject().referTo(
          channelGains_.at(0)->getGainSlider()->getValueObject());
    }
  }

  void unlinkChannels() {
    //channelLinkButton_->setToggleState(false, dontSendNotification);
    float lastValue = 0.f;
    if (!channelGains_.empty()) {
      lastValue = channelGains_.at(0)->getGainSlider()->getValue();
    }
    for (const auto& channelGain : channelGains_) {
      channelGain->getGainSlider()->getValueObject().referTo({});
      channelGain->getGainSlider()->setValue(lastValue, dontSendNotification);
    }
  }

  std::weak_ptr<ear::plugin::LevelMeterCalculator> levelMeter_;

  std::unique_ptr<Label> headingLabel_;
  std::unique_ptr<ear::plugin::ui::EarButton> channels1to12Button_;
  std::unique_ptr<ear::plugin::ui::EarButton> channels13to24Button_;
  std::unique_ptr<ear::plugin::ui::EarButton> channels25to36Button_;
  std::unique_ptr<ear::plugin::ui::EarButton> channels37to48Button_;
  std::unique_ptr<ear::plugin::ui::EarButton> channels49to60Button_; /*
  std::unique_ptr<ear::plugin::ui::EarButton> channels1to6Button_;
  std::unique_ptr<ear::plugin::ui::EarButton> channels7to12Button_;
  std::unique_ptr<ear::plugin::ui::EarButton> channels13to18Button_;
  std::unique_ptr<ear::plugin::ui::EarButton> channels19to24Button_;*/

  std::vector<std::unique_ptr<ear::plugin::ui::ChannelGain>> channelGains_;
  std::unique_ptr<ear::plugin::ui::ChannelGainsBox> channelGainsBox1to12_;
  std::unique_ptr<ear::plugin::ui::ChannelGainsBox> channelGainsBox13to24_;
  std::unique_ptr<ear::plugin::ui::ChannelGainsBox> channelGainsBox25to36_;
  std::unique_ptr<ear::plugin::ui::ChannelGainsBox> channelGainsBox37to48_;
  std::unique_ptr<ear::plugin::ui::ChannelGainsBox> channelGainsBox49to60_; /*
  std::unique_ptr<ear::plugin::ui::ChannelGainsBox> channelGainsBox1to6_;
  std::unique_ptr<ear::plugin::ui::ChannelGainsBox> channelGainsBox7to12_;
  std::unique_ptr<ear::plugin::ui::ChannelGainsBox> channelGainsBox13to18_;
  std::unique_ptr<ear::plugin::ui::ChannelGainsBox> channelGainsBox19to24_;*/

  //std::unique_ptr<ear::plugin::ui::EarButton> channelLinkButton_;

  //std::mutex commonDefinitionHelperMutex_;

  const float labelWidth_ = 71.f;
  const float labelPaddingBottom_ = 0.f;
  const float sliderHeight_ = 40.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxChannelGain)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
