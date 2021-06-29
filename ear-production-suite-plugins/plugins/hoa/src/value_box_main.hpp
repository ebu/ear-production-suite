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
    //ME add in common definition version
        commonDefinitionLabel_(std::make_unique<Label>()),//unique pointer
        commonDefinitionComboBox_(std::make_shared<EarComboBox>()),//shared pointer
        commonDefinitionComboBoxPopup_(
            std::make_shared<EarComboBoxPopup>(commonDefinitionComboBox_.get())),
        /* Old DS Code
        // Need a similar thing for HOA type
        speakerSetupLabel_(std::make_unique<Label>()),
        speakerSetupsComboBox_(std::make_shared<EarComboBox>()),
        */
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

    routingComboBox_->setDefaultText("enter receiving Channel");
    addAndMakeVisible(routingComboBox_.get());
    
    //ME added this. Up here in public decide what to actually do with the variables defined in private
    commonDefinitionLabel_->setFont(EarFonts::Label);
    commonDefinitionLabel_->setText( "HOA type",
                           juce::NotificationType::dontSendNotification);
    commonDefinitionLabel_->setColour(Label::textColourId, EarColours::Label);
    commonDefinitionLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(commonDefinitionLabel_.get());

    /* commonDefinitionComboBox_->setDefaultText("enter HOA type");
    commonDefinitionComboBox_->addTextEntry("3D order1 SN3D ACN");
    commonDefinitionComboBox_->addTextEntry("3D order2 SN3D ACN");
    commonDefinitionComboBox_->addTextEntry("3D order3 SN3D ACN");
    commonDefinitionComboBox_->addTextEntry("3D order4 SN3D ACN");
    commonDefinitionComboBox_->addTextEntry("3D order5 SN3D ACN");
    commonDefinitionComboBox_->addTextEntry("3D order6 SN3D ACN");
    commonDefinitionComboBox_->addTextEntry("3D order1 N3D ACN");
    commonDefinitionComboBox_->addTextEntry("3D order2 N3D ACN");
    commonDefinitionComboBox_->addTextEntry("3D order3 N3D ACN");
    commonDefinitionComboBox_->addTextEntry("3D order4 N3D ACN");
    commonDefinitionComboBox_->addTextEntry("3D order5 N3D ACN");
    commonDefinitionComboBox_->addTextEntry("3D order6 N3D ACN");
    commonDefinitionComboBox_->addTextEntry("3D order1 FuMa");
    commonDefinitionComboBox_->addTextEntry("3D order2 FuMa");
    commonDefinitionComboBox_->addTextEntry("3D order3 FuMa");
    commonDefinitionComboBox_->addTextEntry("2D Order1 N3D ACN");
    commonDefinitionComboBox_->addTextEntry("2D Order2 N3D ACN");
    commonDefinitionComboBox_->addTextEntry("2H1P N3D ACN");
    commonDefinitionComboBox_->addTextEntry("3H1P N3D ACN");
    commonDefinitionComboBox_->addTextEntry("2H1V N3D ACN");*/
    addAndMakeVisible(commonDefinitionComboBox_.get());

    commonDefinitionComboBoxPopup_->setName("HOA type box popup");
    commonDefinitionComboBoxPopup_->show();
    commonDefinitionComboBoxPopup_->setVisible(true);
    addAndMakeVisible(commonDefinitionComboBoxPopup_.get());
    /*
    //ME exoeriementing
    /auto elementRelationships =p->admCommonDefinitions.getElementRelationships();
    for (auto const& [id, tdData] : elementRelationships) {
      commonDefinitionComboBox_->addTextEntry(tdData->name, id);
    }
    //ME end
    */

    /* Old DS Code
    // Need a similar thing for HOA type
    speakerSetupLabel_->setFont(EarFonts::Label);
    speakerSetupLabel_->setColour(Label::textColourId, EarColours::Label);
    speakerSetupLabel_->setText("Layout",
                                juce::NotificationType::dontSendNotification);
    speakerSetupLabel_->setJustificationType(Justification::right);
    addAndMakeVisible(speakerSetupLabel_.get());
    speakerSetupsComboBox_->setDefaultText("select speaker layout");

    speakerSetupsComboBox_->addTextEntry("mono (0+1+0)");
    speakerSetupsComboBox_->addTextEntry("stereo (0+2+0)");
    speakerSetupsComboBox_->addTextEntry("5.1 (0+5+0)");
    speakerSetupsComboBox_->addTextEntry("5.1+2H (2+5+0)");
    speakerSetupsComboBox_->addTextEntry("5.1+4H (4+5+0)");
    speakerSetupsComboBox_->addTextEntry(" (4+5+1)");
    speakerSetupsComboBox_->addTextEntry("7.2+3H (3+7+0)");
    speakerSetupsComboBox_->addTextEntry("9.1+4H (4+9+0)");
    speakerSetupsComboBox_->addTextEntry("22.2 (9+10+3)");
    speakerSetupsComboBox_->addTextEntry("7.1 (0+7+0)");
    speakerSetupsComboBox_->addTextEntry("7.1+4H (4+7+0)");
    speakerSetupsComboBox_->addTextEntry("7.1+2H (2+7+0)");
    speakerSetupsComboBox_->addTextEntry("3.0 (0+3+0)");
    speakerSetupsComboBox_->addTextEntry("5.0 (0+5+0)");
    addAndMakeVisible(speakerSetupsComboBox_.get());
    */

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

    auto descArea = area.removeFromTop(63);
    auto colourArea = descArea.withWidth(labelWidth_);
    colourComboBox_->setBounds(colourArea);
    auto nameArea = descArea.withTrimmedLeft(labelWidth_ + marginBig_)
                        .reduced(0, marginSmall_);
    name_->setBounds(nameArea);

    area.removeFromTop(15.f);

    //ME add for HOA
    auto commonDefinitionArea = area.removeFromTop(rowHeight_);
    auto commonDefinitionLabelArea = commonDefinitionArea.withWidth(labelWidth_);
    auto commonDefinitionComboBoxArea = commonDefinitionArea.withTrimmedLeft(labelWidth_ + marginBig_).reduced(0, marginSmall_);
    commonDefinitionLabel_->setBounds(commonDefinitionLabelArea);
    commonDefinitionComboBox_->setBounds(commonDefinitionComboBoxArea);
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
  std::shared_ptr<EarComboBox> getCommonDefinitionComboBox() { return commonDefinitionComboBox_;  }

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
  std::unique_ptr<Label> commonDefinitionLabel_;
  std::shared_ptr<EarComboBox> commonDefinitionComboBox_;
  std::shared_ptr<EarComboBoxPopup> commonDefinitionComboBoxPopup_;
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
