#pragma once

#include "JuceHeader.h"

#include "components/ear_combo_box.hpp"
#include "components/ear_name_text_editor.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ValueBoxMain : public Component {
 public:
  ValueBoxMain()
      : colourComboBox_(std::make_shared<EarComboBox>()),
        name_(std::make_shared<EarNameTextEditor>()),
        routingLabel_(std::make_unique<Label>()),
        routingComboBox_(std::make_shared<EarComboBox>()) {
    setColour(backgroundColourId, EarColours::Area04dp);

    name_->setLabelText("Name");
    name_->setText("Object_1");
    name_->setEnabled(false);
    name_->setAlpha(0.38f);
    addAndMakeVisible(name_.get());

    routingLabel_->setFont(EarFonts::Label);
    routingLabel_->setText("Routing",
                           juce::NotificationType::dontSendNotification);
    routingLabel_->setColour(Label::textColourId, EarColours::Label);
    routingLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(routingLabel_.get());
    // routingComboBox_->setLookAndFeel(&routingLookAndFeel_);
    routingComboBox_->setDefaultText("Select Scene channel");
    for (int i = 1; i <= 64; ++i) {
      routingComboBox_->addTextEntry(String(i));
    }
    addAndMakeVisible(routingComboBox_.get());

    for (auto colour : EarColours::Items) {
      colourComboBox_->addColourEntry(colour);
    }
    colourComboBox_->setEnabled(false);
    addAndMakeVisible(colourComboBox_.get());
  }

  ~ValueBoxMain() {}

  void paint(Graphics& g) override {
    // background
    g.fillAll(findColour(backgroundColourId));
  }

  enum ColourIds {
    backgroundColourId = 0x00010001,
  };

  void resized() override {
    auto area = getLocalBounds();
    area.reduce(10, 5);
    float comboBoxWidth = area.getWidth() - labelWidth_ - marginBig_;

    area.removeFromTop(marginBig_);

    auto descArea = area.removeFromTop(63);
    auto colourArea = descArea.withWidth(labelWidth_);
    colourComboBox_->setBounds(colourArea);
    auto nameArea = descArea.withTrimmedLeft(labelWidth_ + marginBig_)
                        .reduced(0, marginSmall_);
    name_->setBounds(nameArea);

    area.removeFromTop(15.f);

    auto routingArea = area.removeFromTop(rowHeight_);
    auto routingLabelArea = routingArea.withWidth(labelWidth_);
    auto routingComboBoxArea =
        routingArea.withTrimmedLeft(labelWidth_ + marginBig_)
            .reduced(0, marginSmall_);
    routingLabel_->setBounds(routingLabelArea);
    routingComboBox_->setBounds(routingComboBoxArea);
  }

  std::shared_ptr<EarNameTextEditor> getNameTextEditor() { return name_; }
  std::shared_ptr<EarComboBox> getColourComboBox() { return colourComboBox_; }
  std::shared_ptr<EarComboBox> getRoutingComboBox() { return routingComboBox_; }

 private:
  const float labelWidth_ = 110.f;
  const float rowHeight_ = 40.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  std::shared_ptr<EarComboBox> colourComboBox_;
  std::shared_ptr<EarNameTextEditor> name_;
  std::unique_ptr<Label> routingLabel_;
  std::shared_ptr<EarComboBox> routingComboBox_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxMain)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
