#pragma once

#include "JuceHeader.h"

#include "components/ear_combo_box.hpp"
#include "components/ear_name_text_editor.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include <daw_channel_count.h>

namespace ear {
namespace plugin {
namespace ui {

class ValueBoxMain : public Component {
 public:
  ValueBoxMain()
      : colourComboBox_(std::make_shared<EarComboBox>()),
        name_(std::make_shared<EarNameTextEditor>()),
        useTrackNameCheckbox_(std::make_shared<ToggleButton>()),
        routingLabel_(std::make_unique<Label>()),
        routingInfoIcon_(std::make_unique<ImageComponent>()),
        routingComboBox_(std::make_shared<EarComboBox>()) {
    setColour(backgroundColourId, EarColours::Area04dp);

    name_->setLabelText("Object Name");
    name_->setText("Object_1");
    name_->setEnabled(false);
    name_->setAlpha(0.38f);
    addAndMakeVisible(name_.get());

    useTrackNameCheckbox_->setButtonText("Use track name");
    useTrackNameCheckbox_->setClickingTogglesState(false); // FrontendConnector controls state
    addAndMakeVisible(useTrackNameCheckbox_.get());

    routingInfoIcon_->setImage(ImageFileFormat::loadFrom(
        binary_data::infologo_png, binary_data::infologo_pngSize));
    routingInfoIcon_->setImagePlacement(RectanglePlacement::centred +
                                      RectanglePlacement::doNotResize);
    routingInfoIcon_->setAlpha(0.8);
    routingInfoIcon_->setMouseCursor(MouseCursor::PointingHandCursor);
    routingInfoIcon_->setTooltip("128 channel support requires REAPER v7 or later.");
    addChildComponent(routingInfoIcon_.get());

    routingLabel_->setFont(EarFontsSingleton::instance().Label);
    routingLabel_->setText("Routing",
                           juce::NotificationType::dontSendNotification);
    routingLabel_->setColour(Label::textColourId, EarColours::Label);
    routingLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(routingLabel_.get());
    // routingComboBox_->setLookAndFeel(&routingLookAndFeel_);
    routingComboBox_->setDefaultText("Select Scene channel");
    for (int i = 1; i <= MAX_DAW_CHANNELS; ++i) {
      routingComboBox_->addTextEntry(String(i), i);
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

    auto descArea = area.removeFromTop(90);
    auto colourArea = descArea.withWidth(labelWidth_);
    colourComboBox_->setBounds(colourArea.removeFromTop(63));
    auto nameArea = descArea.withTrimmedLeft(labelWidth_ + marginBig_);
    name_->setBounds(nameArea.removeFromTop(63).reduced(0, marginSmall_));
    useTrackNameCheckbox_->setBounds(nameArea);

    area.removeFromTop(15.f);

    auto routingArea = area.removeFromTop(rowHeight_);
    auto routingLabelArea = routingArea.withWidth(labelWidth_);
    routingLabel_->setBounds(routingLabelArea);
    if (routingInfoIcon_->isVisible()) {
      auto routingInfoArea = routingArea.removeFromRight(13).withTrimmedTop(1); // Trimming 1 to get an odd height. Icon is odd height too, so ensures an integer number of pixels padding top and bottom to avoid blurring through anti-aliasing. 
      routingInfoIcon_->setBounds(routingInfoArea);
      routingArea.removeFromRight(marginSmall_);
    }
    auto routingComboBoxArea =
        routingArea.withTrimmedLeft(labelWidth_ + marginBig_)
            .reduced(0, marginSmall_);
    routingComboBox_->setBounds(routingComboBoxArea);
  }

  std::shared_ptr<EarNameTextEditor> getNameTextEditor() { return name_; }
  std::shared_ptr<ToggleButton> getUseTrackNameCheckbox() { return useTrackNameCheckbox_; }
  std::shared_ptr<EarComboBox> getColourComboBox() { return colourComboBox_; }
  std::shared_ptr<EarComboBox> getRoutingComboBox() { return routingComboBox_; }

  void setValidRoutingMax(int maxChVal) {
    routingInfoIcon_->setVisible(maxChVal < 128);
    if (auto popup = routingComboBox_->getPopup()) {
      for (int i = 1; i <= MAX_DAW_CHANNELS; ++i) {
        auto entry = popup->getEntryById(i);
        if (entry) {
          entry->setSelectable(i <= maxChVal);
        }
      }
    }
  }

 private:
  const float labelWidth_ = 110.f;
  const float rowHeight_ = 40.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  std::shared_ptr<EarComboBox> colourComboBox_;
  std::shared_ptr<EarNameTextEditor> name_;
  std::unique_ptr<Label> routingLabel_;
  std::shared_ptr<EarComboBox> routingComboBox_;
  std::shared_ptr<ToggleButton> useTrackNameCheckbox_;
  std::shared_ptr<ImageComponent> routingInfoIcon_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxMain)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
