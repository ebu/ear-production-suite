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
        useTrackNameCheckbox_(std::make_shared<ToggleButton>()),
        hoaTypeLabel_(std::make_unique<Label>()),
        hoaTypeComboBox_(std::make_shared<EarComboBox>()),
        routingLabel_(std::make_unique<Label>()),
        routingComboBox_(std::make_shared<EarComboBox>()) {

    colourComboBox_->setName("EarComboBox (ValueBoxMain::colourComboBox_)");
    name_->setName("EarNameTextEditor (ValueBoxMain::name_)");
    hoaTypeLabel_->setName("Label (ValueBoxMain::hoaTypeLabel_)");
    hoaTypeComboBox_->setName("EarComboBox (ValueBoxMain::hoaTypeComboBox_)");
    routingLabel_->setName("Label (ValueBoxMain::routingLabel_)");
    routingComboBox_->setName("EarComboBox (ValueBoxMain::routingComboBox_)");

    name_->setLabelText("Object Name");
    name_->setText("HOA_1");
    name_->setEnabled(false);
    name_->setAlpha(0.38f);
    addAndMakeVisible(name_.get());

    useTrackNameCheckbox_->setButtonText("Use track name");
    useTrackNameCheckbox_->setClickingTogglesState(false); // FrontendConnector controls state
    addAndMakeVisible(useTrackNameCheckbox_.get());

    routingLabel_->setFont(EarFonts::Label);
    routingLabel_->setText("Routing",
                           juce::NotificationType::dontSendNotification);
    routingLabel_->setColour(Label::textColourId, EarColours::Label);
    routingLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(routingLabel_.get());

    routingComboBox_->setDefaultText("Select Scene channel range");
    addAndMakeVisible(routingComboBox_.get());

    hoaTypeLabel_->setFont(EarFonts::Label);
    hoaTypeLabel_->setText("HOA type",
                           juce::NotificationType::dontSendNotification);
    hoaTypeLabel_->setColour(Label::textColourId, EarColours::Label);
    hoaTypeLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(hoaTypeLabel_.get());

    auto commonDefinitionHelper = AdmCommonDefinitionHelper::getSingleton();
    auto elementRelationships =
        commonDefinitionHelper->getElementRelationships();
    for (auto const& [id, tdData] : elementRelationships) {
      if (id == 4) {
        auto packData = tdData->relatedPackFormats;
        for (auto const& pfData : packData) {
          hoaTypeComboBox_->addTextEntry(pfData->niceName, pfData->id);
        }
      }
    }

    hoaTypeComboBox_->setDefaultText("Select HOA Type");
    addAndMakeVisible(hoaTypeComboBox_.get());

    for (auto colour : EarColours::Items) {
      colourComboBox_->addColourEntry(colour);
    }
    colourComboBox_->setEnabled(false);
    colourComboBox_->setAlpha(0.38f);
    addAndMakeVisible(colourComboBox_.get());
  }

  ~ValueBoxMain() {}

  void paint(Graphics& g) override { g.fillAll(EarColours::Area04dp); }

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

    auto hoaTypeArea = area.removeFromTop(rowHeight_);
    auto hoaTypeLabelArea = hoaTypeArea.withWidth(labelWidth_);
    auto hoaTypeComboBoxArea =
        hoaTypeArea.withTrimmedLeft(labelWidth_ + marginBig_)
            .reduced(0, marginSmall_);
    hoaTypeLabel_->setBounds(hoaTypeLabelArea);
    hoaTypeComboBox_->setBounds(hoaTypeComboBoxArea);

    auto routingArea = area.removeFromTop(rowHeight_);
    auto routingLabelArea = routingArea.withWidth(labelWidth_);
    auto routingComboBoxArea =
        routingArea.withTrimmedLeft(labelWidth_ + marginBig_)
            .reduced(0, marginSmall_);
    routingLabel_->setBounds(routingLabelArea);
    routingComboBox_->setBounds(routingComboBoxArea);
  }

  std::shared_ptr<EarNameTextEditor> getNameTextEditor() { return name_; }
  std::shared_ptr<EarComboBox> getRoutingComboBox() { return routingComboBox_; }
  std::shared_ptr<EarComboBox> getColourComboBox() { return colourComboBox_; }
  std::shared_ptr<EarComboBox> getHoaTypeComboBox() { return hoaTypeComboBox_; }
  std::shared_ptr<ToggleButton> getUseTrackNameCheckbox() { return useTrackNameCheckbox_; }

 private:
  const float labelWidth_ = 110.f;
  const float rowHeight_ = 40.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  std::shared_ptr<EarComboBox> colourComboBox_;
  std::shared_ptr<EarNameTextEditor> name_;
  std::shared_ptr<ToggleButton> useTrackNameCheckbox_;

  std::unique_ptr<Label> hoaTypeLabel_;
  std::shared_ptr<EarComboBox> hoaTypeComboBox_;

  std::unique_ptr<Label> routingLabel_;
  std::shared_ptr<EarComboBox> routingComboBox_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxMain)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
