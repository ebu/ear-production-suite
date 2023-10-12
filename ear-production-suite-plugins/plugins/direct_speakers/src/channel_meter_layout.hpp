#pragma once

#include "JuceHeader.h"

#include "components/ear_slider.hpp"
#include "components/level_meter.hpp"
#include "components/ear_button.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/look_and_feel/slider.hpp"
#include "speaker_meters_box.hpp"
#include "speaker_setups.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ChannelMeterLayout : public Component {
 public:
  ChannelMeterLayout(
      std::weak_ptr<ear::plugin::LevelMeterCalculator> levelMeter)
      : levelMeter_(levelMeter),
        headingLabel_(std::make_unique<Label>()),
        channelMeterBox1to6_(std::make_unique<SpeakerMetersBox>()),
        channelMeterBox7to12_(std::make_unique<SpeakerMetersBox>()),
        channelMeterBox13to18_(std::make_unique<SpeakerMetersBox>()),
        channelMeterBox19to24_(std::make_unique<SpeakerMetersBox>()) {
    headingLabel_->setFont(EarFontsSingleton::instance().Heading);
    headingLabel_->setColour(Label::textColourId, EarColours::Heading);
    headingLabel_->setText("Channel Meters",
                           juce::NotificationType::dontSendNotification);
    headingLabel_->setJustificationType(Justification::bottomLeft);
    addAndMakeVisible(headingLabel_.get());

    clearSpeakerSetup();

    addAndMakeVisible(channelMeterBox1to6_.get());
    addChildComponent(channelMeterBox7to12_.get());
    addChildComponent(channelMeterBox13to18_.get());
    addChildComponent(channelMeterBox19to24_.get());
  }

  ~ChannelMeterLayout() {}

  void paint(Graphics& g) override { g.fillAll(EarColours::Area01dp); }

  void resized() override {
    auto area = getLocalBounds();
    area.reduce(10, 5);
    headingLabel_->setBounds(area.removeFromTop(30));

    area.removeFromTop(marginBig_);

    channelMeterBox1to6_->setVisible(true);
    channelMeterBox7to12_->setVisible(speakerLevels_.size() > 6);
    channelMeterBox13to18_->setVisible(speakerLevels_.size() > 12);
    channelMeterBox19to24_->setVisible(speakerLevels_.size() > 18);
    int meterRows = 2;
    if(speakerLevels_.size() > 12) meterRows = 3;
    if(speakerLevels_.size() > 18) meterRows = 4;

    auto metersBoxHeight = area.getHeight() / meterRows;
    // 6 meters require 337 px - centre them with left padding
    int leftPad = (area.getWidth() - 337) / 2;
    if (leftPad > 0) area.removeFromLeft(leftPad);

    auto metersBoxArea = area.removeFromTop(metersBoxHeight);
    channelMeterBox1to6_->setBounds(metersBoxArea.reduced(0, 5));
    metersBoxArea = area.removeFromTop(metersBoxHeight);
    channelMeterBox7to12_->setBounds(metersBoxArea.reduced(0, 5));
    if(meterRows >= 3) {
      metersBoxArea = area.removeFromTop(metersBoxHeight);
      channelMeterBox13to18_->setBounds(metersBoxArea.reduced(0, 5));
    }
    if(meterRows >= 4) {
      metersBoxArea = area;
      channelMeterBox19to24_->setBounds(metersBoxArea.reduced(0, 5));
    }
  }

  void clearSpeakerSetup() {
    channelMeterBox1to6_->removeAllLevelMeters();
    channelMeterBox7to12_->removeAllLevelMeters();
    channelMeterBox13to18_->removeAllLevelMeters();
    channelMeterBox19to24_->removeAllLevelMeters();
    speakerLevels_.clear();

    channelMeterBox1to6_->setVisible(true);
    channelMeterBox7to12_->setVisible(false);
    channelMeterBox13to18_->setVisible(false);
    channelMeterBox19to24_->setVisible(false);
  }

  void setSpeakerSetup(SpeakerSetup speakerSetup) {
    clearSpeakerSetup();
    for (int i = 0; i < speakerSetup.speakers.size(); ++i) {
      speakerLevels_.push_back(
          std::make_unique<SpeakerLevel>(speakerSetup.speakers.at(i).spLabel));
      speakerLevels_.back()->getLevelMeter()->setMeter(levelMeter_, i);
      if (i < 6) {
        channelMeterBox1to6_->addLevelMeter(speakerLevels_.back().get());
      } else if (i < 12) {
        channelMeterBox7to12_->addLevelMeter(speakerLevels_.back().get());
      } else if (i < 18) {
        channelMeterBox13to18_->addLevelMeter(speakerLevels_.back().get());
      } else {
        channelMeterBox19to24_->addLevelMeter(speakerLevels_.back().get());
      }
    }

    resized();
  }

 private:
  std::weak_ptr<ear::plugin::LevelMeterCalculator> levelMeter_;

  std::unique_ptr<Label> headingLabel_;

  std::vector<std::unique_ptr<ear::plugin::ui::SpeakerLevel>> speakerLevels_;
  std::unique_ptr<ear::plugin::ui::SpeakerMetersBox> channelMeterBox1to6_;
  std::unique_ptr<ear::plugin::ui::SpeakerMetersBox> channelMeterBox7to12_;
  std::unique_ptr<ear::plugin::ui::SpeakerMetersBox> channelMeterBox13to18_;
  std::unique_ptr<ear::plugin::ui::SpeakerMetersBox> channelMeterBox19to24_;

  const float labelWidth_ = 71.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelMeterLayout)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
