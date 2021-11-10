#pragma once

#include "JuceHeader.h"

#include "components/ear_slider.hpp"
#include "components/level_meter.hpp"
#include "components/ear_button.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/look_and_feel/slider.hpp"
#include "channel_gains_box.hpp"
#include "speaker_setups.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ValueBoxChannelGain : public Component {
 public:
  ValueBoxChannelGain(
      std::weak_ptr<ear::plugin::LevelMeterCalculator> levelMeter)
      : levelMeter_(levelMeter),
        headingLabel_(std::make_unique<Label>()),
        channelGainsBox1to6_(std::make_unique<ChannelGainsBox>()),
        channelGainsBox7to12_(std::make_unique<ChannelGainsBox>()),
        channelGainsBox13to18_(std::make_unique<ChannelGainsBox>()),
        channelGainsBox19to24_(std::make_unique<ChannelGainsBox>()),
        channelLinkButton_(std::make_unique<EarButton>()) {
    headingLabel_->setFont(EarFonts::Heading);
    headingLabel_->setColour(Label::textColourId, EarColours::Heading);
    headingLabel_->setText("Channel Meters",
                           juce::NotificationType::dontSendNotification);
    headingLabel_->setJustificationType(Justification::bottomLeft);
    addAndMakeVisible(headingLabel_.get());

    clearSpeakerSetup();


    addAndMakeVisible(channelGainsBox1to6_.get());
    addChildComponent(channelGainsBox7to12_.get());
    addChildComponent(channelGainsBox13to18_.get());
    addChildComponent(channelGainsBox19to24_.get());

    channelLinkButton_->setButtonText("Channel Link");
    channelLinkButton_->setShape(EarButton::Shape::Toggle);
    channelLinkButton_->setClickingTogglesState(true);
    channelLinkButton_->onClick = [this] { this->toggleChannelLink(); };
    channelLinkButton_->setEnabled(false);
    channelLinkButton_->setAlpha(Emphasis::disabled);
  }

  ~ValueBoxChannelGain() {}

  void paint(Graphics& g) override { g.fillAll(EarColours::Area01dp); }

  void resized() override {
    auto area = getLocalBounds();
    area.reduce(10, 5);
    headingLabel_->setBounds(area.removeFromTop(30));

    area.removeFromTop(marginBig_);

    channelGainsBox1to6_->setVisible(true);
    channelGainsBox7to12_->setVisible(channelGains_.size() > 6);
    channelGainsBox13to18_->setVisible(channelGains_.size() > 12);
    channelGainsBox19to24_->setVisible(channelGains_.size() > 18);
    int meterRows = 2;
    if(channelGains_.size() > 12) meterRows = 3;
    if(channelGains_.size() > 18) meterRows = 4;

    auto gainsBoxHeight = area.getHeight() / meterRows;

    auto gainsBoxArea = area.removeFromTop(gainsBoxHeight);
    channelGainsBox1to6_->setBounds(gainsBoxArea.reduced(0, 5));
    gainsBoxArea = area.removeFromTop(gainsBoxHeight);
    channelGainsBox7to12_->setBounds(gainsBoxArea.reduced(0, 5));
    if(meterRows >= 3) {
      gainsBoxArea = area.removeFromTop(gainsBoxHeight);
      channelGainsBox13to18_->setBounds(gainsBoxArea.reduced(0, 5));
    }
    if(meterRows >= 4) {
      gainsBoxArea = area;
      channelGainsBox19to24_->setBounds(gainsBoxArea.reduced(0, 5));
    }
  }

  void clearSpeakerSetup() {
    channelGainsBox1to6_->removeAllChannelGains();
    channelGainsBox7to12_->removeAllChannelGains();
    channelGainsBox13to18_->removeAllChannelGains();
    channelGainsBox19to24_->removeAllChannelGains();
    channelGains_.clear();

    channelGainsBox1to6_->setVisible(true);
    channelGainsBox7to12_->setVisible(false);
    channelGainsBox13to18_->setVisible(false);
    channelGainsBox19to24_->setVisible(false);

    channelLinkButton_->setEnabled(false);
    channelLinkButton_->setAlpha(Emphasis::disabled);
  }

  void setSpeakerSetup(SpeakerSetup speakerSetup) {
    clearSpeakerSetup();
    for (int i = 0; i < speakerSetup.speakers.size(); ++i) {
      channelGains_.push_back(
          std::make_unique<ChannelGain>(speakerSetup.speakers.at(i).spLabel));
      channelGains_.back()->getLevelMeter()->setMeter(levelMeter_, i);
      if (i < 6) {
        channelGainsBox1to6_->addChannelGain(channelGains_.back().get());
      } else if (i < 12) {
        channelGainsBox7to12_->addChannelGain(channelGains_.back().get());
      } else if (i < 18) {
        channelGainsBox13to18_->addChannelGain(channelGains_.back().get());
      } else {
        channelGainsBox19to24_->addChannelGain(channelGains_.back().get());
      }
    }

    linkChannels();
    resized();
  }

  void toggleChannelLink() {
    if (channelLinkButton_->getToggleState()) {
      linkChannels();
    } else {
      unlinkChannels();
    }
  }

 private:
  void linkChannels() {
    channelLinkButton_->setToggleState(true, dontSendNotification);
  }

  void unlinkChannels() {
    channelLinkButton_->setToggleState(false, dontSendNotification);
  }

  std::weak_ptr<ear::plugin::LevelMeterCalculator> levelMeter_;

  std::unique_ptr<Label> headingLabel_;

  std::vector<std::unique_ptr<ear::plugin::ui::ChannelGain>> channelGains_;
  std::unique_ptr<ear::plugin::ui::ChannelGainsBox> channelGainsBox1to6_;
  std::unique_ptr<ear::plugin::ui::ChannelGainsBox> channelGainsBox7to12_;
  std::unique_ptr<ear::plugin::ui::ChannelGainsBox> channelGainsBox13to18_;
  std::unique_ptr<ear::plugin::ui::ChannelGainsBox> channelGainsBox19to24_;

  std::unique_ptr<ear::plugin::ui::EarButton> channelLinkButton_;

  const float labelWidth_ = 71.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxChannelGain)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
