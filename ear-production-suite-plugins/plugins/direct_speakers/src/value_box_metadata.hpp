#pragma once

#include "JuceHeader.h"

#include "components/ear_combo_box.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "helper/iso_lang_codes.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ValueBoxMetadata : public Component {
 public:
  ValueBoxMetadata()
      : headingLabel_(std::make_unique<Label>()),
        langLabel_(std::make_unique<Label>()),
        langComboBox_(std::make_unique<EarComboBox>()),
        contentKindLabel_(std::make_unique<Label>()),
        contentKindComboBox_(std::make_unique<EarComboBox>()) {
    headingLabel_->setFont(EarFonts::Heading);
    headingLabel_->setColour(Label::textColourId, EarColours::Heading);
    headingLabel_->setText("Metadata",
                           juce::NotificationType::dontSendNotification);
    headingLabel_->setJustificationType(Justification::bottomLeft);
    addAndMakeVisible(headingLabel_.get());

    langLabel_->setFont(EarFonts::Label);
    langLabel_->setText("Language",
                        juce::NotificationType::dontSendNotification);
    langLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(langLabel_.get());

    for (std::size_t i = 0; i < LANGUAGES.size(); ++i) {
      langComboBox_->addTextEntry(
          String::fromUTF8(LANGUAGES[i].english.c_str()));
    }
    // langComboBox_->onChange = [this] { ... };
    langComboBox_->setDefaultText("select language");
    addAndMakeVisible(langComboBox_.get());

    contentKindLabel_->setFont(EarFonts::Label);
    contentKindLabel_->setText("Content Kind",
                               juce::NotificationType::dontSendNotification);
    contentKindLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(contentKindLabel_.get());

    contentKindComboBox_->addSectionEntry("Non Dialogue");
    contentKindComboBox_->addTextEntry("Undefined");
    contentKindComboBox_->addTextEntry("Music");
    contentKindComboBox_->addTextEntry("Effect");
    contentKindComboBox_->addSectionEntry("Dialogue");
    contentKindComboBox_->addTextEntry("Undefined");
    contentKindComboBox_->addTextEntry("Storyline");
    contentKindComboBox_->addTextEntry("Voiceover");
    contentKindComboBox_->addTextEntry("Spoken Subtitle");
    contentKindComboBox_->addTextEntry("Audio Description");
    contentKindComboBox_->addTextEntry("Commentary");
    contentKindComboBox_->addTextEntry("Emergency");
    contentKindComboBox_->addSectionEntry("Mixed");
    contentKindComboBox_->addTextEntry("Undefined");
    contentKindComboBox_->addTextEntry("Complete Main");
    contentKindComboBox_->addTextEntry("Mixed");
    contentKindComboBox_->addTextEntry("Hearing Impaired");
    // contentKindComboBox_->onChange = [this] { ... };
    contentKindComboBox_->setDefaultText("select content kind");
    addAndMakeVisible(contentKindComboBox_.get());
  }

  ~ValueBoxMetadata() {}

  void paint(Graphics& g) override { g.fillAll(EarColours::Area04dp); }

  void resized() override {
    auto area = getLocalBounds();
    area.reduce(10, 5);
    headingLabel_->setBounds(area.removeFromTop(30));

    area.removeFromTop(2.f * marginBig_);

    float comboBoxWidth = area.getWidth() - labelWidth_ - marginBig_;
    auto langArea = area.removeFromTop(rowHeight_);
    auto langLabelArea = langArea.withWidth(labelWidth_);
    auto langComboBoxArea = langArea.withTrimmedLeft(labelWidth_ + marginBig_)
                                .reduced(0, marginSmall_);
    langLabel_->setBounds(langLabelArea);
    langComboBox_->setBounds(langComboBoxArea);

    auto contentKindArea = area.removeFromTop(rowHeight_);
    auto contentKindLabelArea = contentKindArea.withWidth(labelWidth_);
    auto contentKindComboBoxArea =
        contentKindArea.withTrimmedLeft(labelWidth_ + marginBig_)
            .reduced(0, marginSmall_);
    contentKindLabel_->setBounds(contentKindLabelArea);
    contentKindComboBox_->setBounds(contentKindComboBoxArea);
  }

 private:
  const float labelWidth_ = 110.f;
  const float rowHeight_ = 40.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  Value testValue_;

  std::unique_ptr<Label> headingLabel_;
  std::unique_ptr<Label> langLabel_;
  std::unique_ptr<EarComboBox> langComboBox_;
  std::unique_ptr<Label> contentKindLabel_;
  std::unique_ptr<EarComboBox> contentKindComboBox_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxMetadata)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
