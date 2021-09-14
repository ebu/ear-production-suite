#pragma once

#include "JuceHeader.h"

#include "components/ear_combo_box.hpp"
#include "components/ear_name_text_editor.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {
  //here we are setting up the main box
class ValueBoxMain : public Component {
 public:
  ValueBoxMain()
      : colourComboBox_(std::make_shared<EarComboBox>()),
        name_(std::make_shared<EarNameTextEditor>()),
    //ME add in common definition version, similar to DS
        hoaTypeLabel_(std::make_unique<Label>()),//unique pointer
        hoaTypeComboBox_(std::make_shared<EarComboBox>()),//shared pointer
        //below is just an experiment, ME
        //commonDefinitionComboBoxPopup_(
          //  std::make_shared<EarComboBoxPopup>(commonDefinitionComboBox_.get())),
        routingLabel_(std::make_unique<Label>()),
        routingComboBox_(std::make_shared<EarComboBox>()) {
    name_->setLabelText("Name");
    name_->setText("HOA_1");//not sure what this does
    name_->setEnabled(false);
    name_->setAlpha(0.38f);
    addAndMakeVisible(name_.get());

    routingLabel_->setFont(EarFonts::Label);
    routingLabel_->setText("Routing",
                           juce::NotificationType::dontSendNotification);
    routingLabel_->setColour(Label::textColourId, EarColours::Label);
    routingLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(routingLabel_.get());

    routingComboBox_->setDefaultText("Select Scene channel range");
    addAndMakeVisible(routingComboBox_.get());

    //ME added this. Up here in public decide what combo box defined in private actually looks like
    hoaTypeLabel_->setFont(EarFonts::Label);
    hoaTypeLabel_->setText("HOA type",
                           juce::NotificationType::dontSendNotification);
    hoaTypeLabel_->setColour(Label::textColourId, EarColours::Label);
    hoaTypeLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(hoaTypeLabel_.get());

    auto commonDefinitionHelper = AdmCommonDefinitionHelper::getSingleton();
    auto elementRelationships = commonDefinitionHelper->getElementRelationships();
    for (auto const& [id, tdData] : elementRelationships) {
      if (id == 4) {
        auto packData = tdData->relatedPackFormats;
        for (auto const& pfData : packData) {
          hoaTypeComboBox_->addTextEntry(
              pfData->niceName, pfData->id);
        }
      }
    }

    hoaTypeComboBox_->setDefaultText("Select HOA Type");
    addAndMakeVisible(hoaTypeComboBox_.get());

    /* commonDefinitionComboBoxPopup_->setName("HOA type box popup");
    commonDefinitionComboBoxPopup_->show();
    commonDefinitionComboBoxPopup_->setVisible(true);
    addAndMakeVisible(commonDefinitionComboBoxPopup_.get());*///this was just an experiment
    /*
    //ME exoeriementing
    /auto elementRelationships =p->admCommonDefinitions.getElementRelationships();
    for (auto const& [id, tdData] : elementRelationships) {
      commonDefinitionComboBox_->addTextEntry(tdData->name, id);
    }
    //ME end
    */



    for (auto colour : EarColours::Items) {
      colourComboBox_->addColourEntry(colour);
    }
    colourComboBox_->setEnabled(false);
    colourComboBox_->setAlpha(0.38f);
    addAndMakeVisible(colourComboBox_.get());

    //commonDefinitionComboBox_->onValueChange = [this](int) {
      //comboBoxStateChanged(&packFormatSelector);
    //};

  }

  ~ValueBoxMain() {}

  void paint(Graphics& g) override { g.fillAll(EarColours::Area04dp); }

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

    //ME add for HOA
    auto hoaTypeArea = area.removeFromTop(rowHeight_);
    auto hoaTypeLabelArea = hoaTypeArea.withWidth(labelWidth_);
    auto hoaTypeComboBoxArea =
        hoaTypeArea.withTrimmedLeft(labelWidth_ + marginBig_)
            .reduced(0, marginSmall_);
    hoaTypeLabel_->setBounds(hoaTypeLabelArea);
    hoaTypeComboBox_->setBounds(hoaTypeComboBoxArea);
    //end ME

    /* Old DS Code
    // Need a similar thing for HOA type
    auto speakerSetupArea = area.removeFromTop(rowHeight_);
    auto speakerSetupLabelArea = speakerSetupArea.withWidth(labelWidth_);
    auto speakerSetupComboBoxArea =
        speakerSetupArea.withTrimmedLeft(labelWidth_ + marginBig_)
            .reduced(0, marginSmall_);
    speakerSetupLabel_->setBounds(speakerSetupLabelArea);
    speakerSetupsComboBox_->setBounds(speakerSetupComboBoxArea);
    */

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
  //ME add likewise for common definition of HOA
  std::shared_ptr<EarComboBox> getHoaTypeComboBox() {
    return hoaTypeComboBox_;
  }
  /* Old DS Code
  // Need similar for HOA type
  std::shared_ptr<EarComboBox> getSpeakerSetupsComboBox() {
    return speakerSetupsComboBox_;
  }
  */

 private:
  const float labelWidth_ = 110.f;
  const float rowHeight_ = 40.f;
  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  std::shared_ptr<EarComboBox> colourComboBox_;
  std::shared_ptr<EarNameTextEditor> name_;
  //ME add similar for commonDefinition. Define variables down here in the private section. Define what we want and group it together in this box. in FEconnector we actually set it
  std::unique_ptr<Label> hoaTypeLabel_;
  std::shared_ptr<EarComboBox> hoaTypeComboBox_;
  //std::shared_ptr<EarComboBoxPopup> commonDefinitionComboBoxPopup_;//this was just an experiment
  //ME end

  /* Old DS Code
  // Need a similar selector for HOA type
  std::unique_ptr<Label> speakerSetupLabel_;
  std::shared_ptr<EarComboBox> speakerSetupsComboBox_;
  */
  std::unique_ptr<Label> routingLabel_;
  std::shared_ptr<EarComboBox> routingComboBox_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxMain)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
