#pragma once

#include "JuceHeader.h"

#include "components/ear_combo_box.hpp"
#include "components/ear_name_text_editor.hpp"
#include "components/routing_info_icon.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include <helper/adm_preset_definitions_helper.h>

#include <algorithm>

namespace ear {
namespace plugin {
namespace ui {

class ValueBoxMain : public Component {
 public:
  ValueBoxMain()
      : colourComboBox_(std::make_shared<EarComboBox>()),
        name_(std::make_shared<EarNameTextEditor>()),
        useTrackNameCheckbox_(std::make_shared<ToggleButton>()),
        speakerSetupLabel_(std::make_unique<Label>()),
        speakerSetupsComboBox_(std::make_shared<EarComboBox>()),
        routingLabel_(std::make_unique<Label>()),
        routingInfoIcon_(createRoutingInfoIcon()),
        routingComboBox_(std::make_shared<EarComboBox>()) {
    name_->setLabelText("Object Name");
    name_->setText("Object_1");
    name_->setEnabled(false);
    name_->setAlpha(0.38f);
    addAndMakeVisible(name_.get());

    useTrackNameCheckbox_->setButtonText("Use track name");
    useTrackNameCheckbox_->setClickingTogglesState(false); // FrontendConnector controls state
    addAndMakeVisible(useTrackNameCheckbox_.get());

    routingLabel_->setFont(EarFontsSingleton::instance().Label);
    routingLabel_->setText("Routing",
                           juce::NotificationType::dontSendNotification);
    routingLabel_->setColour(Label::textColourId, EarColours::Label);
    routingLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(routingLabel_.get());

    addChildComponent(routingInfoIcon_.get());

    routingComboBox_->setDefaultText("Select Scene channel range");
    addAndMakeVisible(routingComboBox_.get());

    speakerSetupLabel_->setFont(EarFontsSingleton::instance().Label);
    speakerSetupLabel_->setColour(Label::textColourId, EarColours::Label);
    speakerSetupLabel_->setText("Layout",
                                juce::NotificationType::dontSendNotification);
    speakerSetupLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(speakerSetupLabel_.get());
    speakerSetupsComboBox_->setDefaultText("Select speaker layout");

    auto tdData = AdmPresetDefinitionsHelper::getSingleton()->getTypeDefinitionData(1);
    auto pfDatas = tdData->relatedPackFormats;
    std::stable_sort(
        pfDatas.begin(), pfDatas.end(),
        [](const std::shared_ptr<AdmPresetDefinitionsHelper::PackFormatData>& a,
           const std::shared_ptr<AdmPresetDefinitionsHelper::PackFormatData>& b) {
          if (a->isCommonDefinition() != b->isCommonDefinition()) {
            return a->isCommonDefinition() > b->isCommonDefinition();
          }
          return a->relatedChannelFormats.size() <
                 b->relatedChannelFormats.size();
    });

    for (auto const& pfData : pfDatas) {
      auto entry = speakerSetupsComboBox_->addTextEntry(pfData->niceName, pfData->idValue);
      entry->setLightFont(!pfData->isCommonDefinition());
    }
    addAndMakeVisible(speakerSetupsComboBox_.get());

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

    auto speakerSetupArea = area.removeFromTop(rowHeight_);
    auto speakerSetupLabelArea = speakerSetupArea.withWidth(labelWidth_);
    auto speakerSetupComboBoxArea =
        speakerSetupArea.withTrimmedLeft(labelWidth_ + marginBig_)
            .reduced(0, marginSmall_);
    speakerSetupLabel_->setBounds(speakerSetupLabelArea);
    speakerSetupsComboBox_->setBounds(speakerSetupComboBoxArea);

    auto routingArea = area.removeFromTop(rowHeight_);
    auto routingLabelArea = routingArea.withWidth(labelWidth_);
    routingLabel_->setBounds(routingLabelArea);
    if (routingInfoIcon_->isVisible()) {
      placeRoutingInfoIconRight(routingArea, routingInfoIcon_.get());
      routingArea.removeFromRight(marginSmall_);
    }
    auto routingComboBoxArea =
        routingArea.withTrimmedLeft(labelWidth_ + marginBig_)
            .reduced(0, marginSmall_);
    routingComboBox_->setBounds(routingComboBoxArea);
  }

  void showRoutingTooltip(bool visible) {
    routingInfoIcon_->setVisible(visible);
  }

  std::shared_ptr<EarNameTextEditor> getNameTextEditor() { return name_; }
  std::shared_ptr<ToggleButton> getUseTrackNameCheckbox() { return useTrackNameCheckbox_; }
  std::shared_ptr<EarComboBox> getRoutingComboBox() { return routingComboBox_; }
  std::shared_ptr<EarComboBox> getColourComboBox() { return colourComboBox_; }
  std::shared_ptr<EarComboBox> getSpeakerSetupsComboBox() {
    return speakerSetupsComboBox_;
  }

 private:
  const float labelWidth_ = 110.f;
  const float rowHeight_ = 40.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  std::shared_ptr<EarComboBox> colourComboBox_;
  std::shared_ptr<EarNameTextEditor> name_;
  std::unique_ptr<Label> speakerSetupLabel_;
  std::shared_ptr<EarComboBox> speakerSetupsComboBox_;
  std::unique_ptr<Label> routingLabel_;
  std::shared_ptr<EarComboBox> routingComboBox_;
  std::shared_ptr<ImageComponent> routingInfoIcon_;
  std::shared_ptr<ToggleButton> useTrackNameCheckbox_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxMain)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
