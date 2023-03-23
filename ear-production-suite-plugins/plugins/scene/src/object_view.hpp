#pragma once

#include "JuceHeader.h"

#include "element_view.hpp"
#include "gain_interaction_settings.hpp"
#include "position_interaction_settings.hpp"
#include "speaker_setups.hpp"
#include "components/ear_expansion_panel.hpp"
#include "components/ear_colour_indicator.hpp"
#include "components/level_meter.hpp"
#include "components/ear_slider_range.hpp"
#include "components/ear_inc_dec_slider.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "metadata.hpp"
#include "helper/common_definition_helper.h"
#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class ObjectView : public ElementView,
                   private Slider::Listener,
                   private EarSliderRange::Listener {
 public:
  struct Data {
    ear::plugin::proto::InputItemMetadata item;
    ear::plugin::proto::Object object;
  };

  // Note, if ObjectView diverges much based on type
  // revisit this and consider splitting it into type
  // specific subclasses. Just disables one panel in
  // one case right now so would be massive overkill
  enum class ObjectType { Object, DirectSpeakers, HOA, Other };

  explicit ObjectView(ObjectType type)
      : ElementView(),
        colourIndicator_(std::make_unique<EarColourIndicator>()),
        nameLabel_(std::make_unique<Label>()),
        levelMeter_(std::make_unique<LevelMeter>()),
        metadataLabel_(std::make_unique<Label>()),
        importanceLabel_(std::make_unique<Label>()),
        importanceSlider_(std::make_unique<EarIncDecSlider>()),
        settingsButton_(std::make_unique<EarButton>()),
        onOffInteractionPanel_(std::make_unique<EarExpansionPanel>()),
        gainInteractionPanel_(std::make_unique<EarExpansionPanel>()),
        gainInteractionSettings_(std::make_unique<GainInteractionSettings>()),
        positionInteractionPanel_(std::make_unique<EarExpansionPanel>()),
        positionInteractionSettings_(
            std::make_unique<PositionInteractionSettings>()),
        objectType_{type} {
    colourIndicator_->setColour(EarColours::Item01);
    addAndMakeVisible(colourIndicator_.get());

    nameLabel_->setFont(EarFontsSingleton::instance().Items);
    nameLabel_->setBorderSize(BorderSize<int>{0});
    nameLabel_->setColour(Label::textColourId, EarColours::Text);
    nameLabel_->setMinimumHorizontalScale(1.f);
    addAndMakeVisible(nameLabel_.get());

    addAndMakeVisible(levelMeter_.get());

    metadataLabel_->setFont(EarFontsSingleton::instance().Values);
    metadataLabel_->setJustificationType(Justification::centredLeft);
    metadataLabel_->setColour(Label::textColourId,
                              EarColours::Text.withAlpha(Emphasis::medium));
    addAndMakeVisible(metadataLabel_.get());

    importanceLabel_->setFont(EarFontsSingleton::instance().Values);
    importanceLabel_->setColour(Label::textColourId, EarColours::Label);
    importanceLabel_->setText("Importance", dontSendNotification);
    importanceLabel_->setJustificationType(Justification::centredLeft);
    addAndMakeVisible(importanceLabel_.get());

    importanceSlider_->slider.valueFromTextFunction = [](const juce::String& text) {
      double val = text.getDoubleValue(); // Returns 0.0 for non-numeric
      if(val < 1.0 || val > 10.0) {
        return 11.0;
      } else {
        return val;
      }
    };
    importanceSlider_->slider.textFromValueFunction = [](double val) {
      if(val < 1.0 || val > 10.0) {
        return juce::String("---");
      } else {
        return juce::String(std::to_string(int(val)));
      }
    };
#ifdef BAREBONESPROFILE
    importanceSlider_->slider.setRange(1, 10);
    importanceSlider_->slider.setDoubleClickReturnValue(true, 10);
#else
    importanceSlider_->wrapAround = true;
    importanceSlider_->slider.setRange(1, 11);
    importanceSlider_->slider.setDoubleClickReturnValue(true, 11);
#endif
    importanceSlider_->slider.setNumDecimalPlacesToDisplay(0);
    importanceSlider_->slider.setMouseDragSensitivity(10);
    importanceSlider_->slider.setIncDecButtonsMode (Slider::incDecButtonsDraggable_AutoDirection);
    importanceSlider_->slider.addListener(this);
    addAndMakeVisible(importanceSlider_.get());

    settingsButton_->setButtonText("Settings");
    settingsButton_->setClickingTogglesState(true);
    settingsButton_->onClick = [this]() {
      data_.object.set_show_settings(this->settingsButton_->getToggleState());
      onOffInteractionPanel_->setVisible(
          this->settingsButton_->getToggleState());
      gainInteractionPanel_->setVisible(
          this->settingsButton_->getToggleState());
      if (objectType_ == ObjectType::Object) {
        positionInteractionPanel_->setVisible(
            this->settingsButton_->getToggleState());
      }
      updateDesiredHeight();
      this->notifyDataChange();
    };
    addAndMakeVisible(settingsButton_.get());

    onOffInteractionPanel_->setHeading("On/Off Interaction");
    onOffInteractionPanel_->onClick = [this]() {
      data_.object.mutable_interactive_on_off()->set_enabled(
          this->onOffInteractionPanel_->isExpanded());
      this->notifyDataChange();
      updateSettingsButtonColour();
    };
    addAndMakeVisible(onOffInteractionPanel_.get());

    gainInteractionPanel_->setContentHeight(50);
    gainInteractionPanel_->setComponent(gainInteractionSettings_.get());
    gainInteractionPanel_->setHeading("Gain Interaction");
    gainInteractionPanel_->onClick = [this]() {
      data_.object.mutable_interactive_gain()->set_enabled(
          this->gainInteractionPanel_->isExpanded());
      this->notifyDataChange();
      updateSettingsButtonColour();
    };
    gainInteractionSettings_->gainSliderRange->addListener(this);
    gainInteractionSettings_->gainMinSlider->addListener(this);
    gainInteractionSettings_->gainMaxSlider->addListener(this);
    addAndMakeVisible(gainInteractionPanel_.get());

    positionInteractionPanel_->setContentHeight(90);
    positionInteractionPanel_->setComponent(positionInteractionSettings_.get());
    positionInteractionPanel_->setHeading("Position Interaction");
    positionInteractionPanel_->onClick = [this]() {
      data_.object.mutable_interactive_position()->set_enabled(
          this->positionInteractionPanel_->isExpanded());
      this->notifyDataChange();
      updateSettingsButtonColour();
    };
    positionInteractionSettings_->azimuthSliderRange->addListener(this);
    positionInteractionSettings_->azimuthMinSlider->addListener(this);
    positionInteractionSettings_->azimuthMaxSlider->addListener(this);
    positionInteractionSettings_->elevationSliderRange->addListener(this);
    positionInteractionSettings_->elevationMinSlider->addListener(this);
    positionInteractionSettings_->elevationMaxSlider->addListener(this);
    positionInteractionSettings_->distanceSliderRange->addListener(this);
    positionInteractionSettings_->distanceMinSlider->addListener(this);
    positionInteractionSettings_->distanceMaxSlider->addListener(this);
    addAndMakeVisible(positionInteractionPanel_.get());

    updateDesiredHeight();
  }

  void updateFromInternalDataObject() {
    // NOTE - will require repaint()!!
    settingsButton_->setToggleState(data_.object.show_settings(),
                                    dontSendNotification);
    updateSettingsButtonColour();

    onOffInteractionPanel_->setVisible(this->settingsButton_->getToggleState());
    gainInteractionPanel_->setVisible(this->settingsButton_->getToggleState());
    positionInteractionPanel_->setVisible(
      this->settingsButton_->getToggleState());
    updateDesiredHeight();

    onOffInteractionPanel_->setExpanded(
      data_.object.interactive_on_off().enabled());
    gainInteractionPanel_->setExpanded(
      data_.object.interactive_gain().enabled());
    gainInteractionSettings_->gainMinSlider->setValue(
      Decibels::gainToDecibels(data_.object.interactive_gain().min()),
      dontSendNotification);
    gainInteractionSettings_->gainMaxSlider->setValue(
      Decibels::gainToDecibels(data_.object.interactive_gain().max()),
      dontSendNotification);
    positionInteractionPanel_->setExpanded(
      data_.object.interactive_position().enabled());
    positionInteractionSettings_->azimuthMinSlider->setValue(
      data_.object.interactive_position().min_az(), dontSendNotification);
    positionInteractionSettings_->azimuthMaxSlider->setValue(
      data_.object.interactive_position().max_az(), dontSendNotification);
    positionInteractionSettings_->elevationMinSlider->setValue(
      data_.object.interactive_position().min_el(), dontSendNotification);
    positionInteractionSettings_->elevationMaxSlider->setValue(
      data_.object.interactive_position().max_el(), dontSendNotification);
    positionInteractionSettings_->distanceMinSlider->setValue(
      data_.object.interactive_position().min_r(), dontSendNotification);
    positionInteractionSettings_->distanceMaxSlider->setValue(
      data_.object.interactive_position().max_r(), dontSendNotification);
  }

  bool updateFromInternalDataInputItemMetadata() {
    // NOTE - will require repaint()!!
    bool changes = false;

    auto colour = Colour(data_.item.colour());
    if(colourIndicator_->getColour() != colour) {
      colourIndicator_->setColour(colour);
      changes = true;
    }

    juce::String nameLabelText{data_.item.name()};
    if(nameLabel_->getText() != nameLabelText) {
      nameLabel_->setText(nameLabelText, dontSendNotification);
      changes = true;
    }

    juce::String metadataLabelText;
    if (data_.item.has_ds_metadata()) {
      metadataLabelText = "DirectSpeakers";
      auto layoutIndex = data_.item.ds_metadata().layout();
      if (layoutIndex >= 0 && layoutIndex < SPEAKER_SETUPS.size()) {
        metadataLabelText = String(SPEAKER_SETUPS.at(layoutIndex).commonName);
      }
    } else if (data_.item.has_hoa_metadata()) {
      metadataLabelText = "HOA";
      auto commonDefinitionHelper = AdmCommonDefinitionHelper::getSingleton();
      auto pfData = commonDefinitionHelper->getPackFormatData(
        4, data_.item.hoa_metadata().packformatidvalue());
      if (pfData) {
        int cfCount = pfData->relatedChannelFormats.size();
        int order = std::sqrt(cfCount) - 1;
        metadataLabelText = "HOA order " + String(order);
      }
    }

    if(!data_.object.has_importance()) {
      // >10 = not set
      if(importanceSlider_->slider.getValue() <= 10) {
        importanceSlider_->slider.setValue(11);
        changes = true;
      }

    } else {
      if(data_.object.importance() != importanceSlider_->slider.getValue()) {
        importanceSlider_->slider.setValue(data_.object.importance());
        changes = true;
      }
    }

    if(metadataLabel_->getText() != metadataLabelText) {
      metadataLabel_->setText(metadataLabelText, dontSendNotification);
      changes = true;
    }

    return changes;
  }

  void setData(const Data& data) {
    data_ = data;
    updateFromInternalDataObject();
    updateFromInternalDataInputItemMetadata();
    repaint();
  }

  Data getData() { return data_; }

  void setInputItemMetadata(const ear::plugin::proto::InputItemMetadata &item) {
    data_.item = item;
    if(updateFromInternalDataInputItemMetadata()) {
      repaint(); // Only repaint if something actually changed!
    }
  }

  std::string getConnectionId() {
    return data_.item.connection_id();
  }

  int getDesiredHeight() override { return desiredHeight_; }

  void paint(Graphics& g) override {
    g.fillAll(EarColours::ObjectItemBackground);
    if ((!Desktop::getInstance().getMainMouseSource().isDragging() &&
         getScreenBounds().contains(Desktop::getMousePosition())) ||
        (Desktop::getInstance().getMainMouseSource().isDragging() &&
         getScreenBounds().contains(Desktop::getLastMouseDownPosition()))) {
      g.fillAll(EarColours::Area02dp);
    }
  }

  void childBoundsChanged(Component* child) override { updateDesiredHeight(); }

  void resized() override {
    updateDesiredHeight();
    auto area = getLocalBounds();
    auto mainArea = area.removeFromTop(mainAreaHeight_);
    colourIndicator_->setBounds(mainArea.removeFromLeft(10));
    handleButton_->setBounds(
        mainArea.removeFromLeft(30).withSizeKeepingCentre(24, 24));
    mainArea.removeFromLeft(marginBig_);

    removeButton_->setBounds(
        mainArea.removeFromRight(55).withSizeKeepingCentre(24, 24));
    settingsButton_->setBounds(
        mainArea.removeFromRight(120).reduced(marginSmall_, 17));

    auto metadataArea = mainArea.removeFromRight(190);
    metadataArea = metadataArea.withTrimmedRight(20).withTrimmedLeft(10); // padding
    auto importanceArea = metadataArea.removeFromTop(38).reduced(0, 10);
    importanceLabel_->setBounds(importanceArea.removeFromLeft(75));
    importanceSlider_->setBounds(importanceArea);
    metadataLabel_->setBounds(metadataArea.removeFromTop(13));

    auto nameMeterArea = mainArea;
    nameLabel_->setBounds(nameMeterArea.removeFromTop(38));
    levelMeter_->setBounds(nameMeterArea.removeFromTop(13));
    area.removeFromLeft(5);
    area.reduce(5, 5);
    if (settingsButton_->getToggleState()) {
      onOffInteractionPanel_->setBounds(
          area.removeFromTop(onOffInteractionPanel_->getDesiredHeight()));
      area.removeFromTop(marginMini_);
      gainInteractionPanel_->setBounds(
          area.removeFromTop(gainInteractionPanel_->getDesiredHeight()));
      if (gainInteractionPanel_->isExpanded()) {
        area.removeFromTop(marginBig_);
      } else {
        area.removeFromTop(marginMini_);
      }
      positionInteractionPanel_->setBounds(
          area.removeFromTop(positionInteractionPanel_->getDesiredHeight()));
    }
  }

  LevelMeter* getLevelMeter() { return levelMeter_.get(); }

  class Listener {
   public:
    virtual ~Listener() = default;
    virtual void objectDataChanged(ObjectView::Data data) = 0;
  };

  void addListener(Listener* l) { listeners_.add(l); }
  void removeListener(Listener* l) { listeners_.remove(l); }

 private:
  // Slider::Listener
  void sliderValueChanged(Slider* slider) override {
    // gain
    if (slider == gainInteractionSettings_->gainMinSlider.get()) {
      auto value = gainInteractionSettings_->gainMinSlider->getValue();
      data_.object.mutable_interactive_gain()->set_min(
          Decibels::decibelsToGain(value));
      notifyDataChange();
    }
    if (slider == gainInteractionSettings_->gainMaxSlider.get()) {
      auto value = gainInteractionSettings_->gainMaxSlider->getValue();
      data_.object.mutable_interactive_gain()->set_max(
          Decibels::decibelsToGain(value));
      notifyDataChange();
    }
    // position
    if (slider == positionInteractionSettings_->azimuthMinSlider.get()) {
      auto value = positionInteractionSettings_->azimuthMinSlider->getValue();
      data_.object.mutable_interactive_position()->set_min_az(value);
      notifyDataChange();
    }
    if (slider == positionInteractionSettings_->azimuthMaxSlider.get()) {
      auto value = positionInteractionSettings_->azimuthMaxSlider->getValue();
      data_.object.mutable_interactive_position()->set_max_az(value);
      notifyDataChange();
    }

    if (slider == positionInteractionSettings_->elevationMinSlider.get()) {
      auto value = positionInteractionSettings_->elevationMinSlider->getValue();
      data_.object.mutable_interactive_position()->set_min_el(value);
      notifyDataChange();
    }
    if (slider == positionInteractionSettings_->elevationMaxSlider.get()) {
      auto value = positionInteractionSettings_->elevationMaxSlider->getValue();
      data_.object.mutable_interactive_position()->set_max_el(value);
      notifyDataChange();
    }

    if (slider == positionInteractionSettings_->distanceMinSlider.get()) {
      auto value = positionInteractionSettings_->distanceMinSlider->getValue();
      data_.object.mutable_interactive_position()->set_min_r(value);
      notifyDataChange();
    }
    if (slider == positionInteractionSettings_->distanceMaxSlider.get()) {
      auto value = positionInteractionSettings_->distanceMaxSlider->getValue();
      data_.object.mutable_interactive_position()->set_max_r(value);
      notifyDataChange();
    }
    // importance
    if (slider == &importanceSlider_->slider) {
      int value = importanceSlider_->slider.getValue();
      if(value < 1 || value > 10) {
        // Out of range == Not set
        data_.object.clear_importance();
      } else {
        data_.object.set_importance(value);
      }
      notifyDataChange();
    }
  }

  // EarSliderRange::Listener
  void sliderRangeValueChanged(EarSliderRange* range) override {
    // gain
    if (range == gainInteractionSettings_->gainSliderRange.get()) {
      auto lowerValue =
          gainInteractionSettings_->gainSliderRange->getLowerValue();
      auto upperValue =
          gainInteractionSettings_->gainSliderRange->getUpperValue();
      data_.object.mutable_interactive_gain()->set_min(
          Decibels::decibelsToGain(lowerValue));
      data_.object.mutable_interactive_gain()->set_max(
          Decibels::decibelsToGain(upperValue));
      notifyDataChange();
    }
    // position
    if (range == positionInteractionSettings_->azimuthSliderRange.get()) {
      auto lowerValue =
          positionInteractionSettings_->azimuthSliderRange->getLowerValue();
      auto upperValue =
          positionInteractionSettings_->azimuthSliderRange->getUpperValue();
      data_.object.mutable_interactive_position()->set_min_az(lowerValue);
      data_.object.mutable_interactive_position()->set_max_az(upperValue);
      notifyDataChange();
    }
    if (range == positionInteractionSettings_->elevationSliderRange.get()) {
      auto lowerValue =
          positionInteractionSettings_->elevationSliderRange->getLowerValue();
      auto upperValue =
          positionInteractionSettings_->elevationSliderRange->getUpperValue();
      data_.object.mutable_interactive_position()->set_min_el(lowerValue);
      data_.object.mutable_interactive_position()->set_max_el(upperValue);
      notifyDataChange();
    }
    if (range == positionInteractionSettings_->distanceSliderRange.get()) {
      auto lowerValue =
          positionInteractionSettings_->distanceSliderRange->getLowerValue();
      auto upperValue =
          positionInteractionSettings_->distanceSliderRange->getUpperValue();
      data_.object.mutable_interactive_position()->set_min_r(lowerValue);
      data_.object.mutable_interactive_position()->set_max_r(upperValue);
      notifyDataChange();
    }
  }

  void notifyDataChange() {
    Component::BailOutChecker checker(this);
    listeners_.callChecked(
        checker, [&](Listener& l) { l.objectDataChanged(this->data_); });
    if (checker.shouldBailOut()) {
      return;
    }
  }

  void updateSettingsButtonColour() {
    const Colour col = (data_.object.mutable_interactive_gain()->enabled() ||
                        data_.object.mutable_interactive_on_off()->enabled() ||
                        data_.object.mutable_interactive_position()->enabled())
                           ? EarColours::SettingsButtonInteraction
                           : EarColours::Area01dp;

    settingsButton_->setColour(EarButton::backgroundColourId, col);
  }

  void updateDesiredHeight() {
    desiredHeight_ = mainAreaHeight_;
    if (settingsButton_->getToggleState()) {
      desiredHeight_ += marginSmall_;
      desiredHeight_ += onOffInteractionPanel_->getDesiredHeight();
      desiredHeight_ += marginMini_;
      desiredHeight_ += gainInteractionPanel_->getDesiredHeight();
      if (gainInteractionPanel_->isExpanded()) {
        desiredHeight_ += marginBig_;
      } else {
        desiredHeight_ += marginMini_;
      }
      if (objectType_ == ObjectType::Object) {
        desiredHeight_ += positionInteractionPanel_->getDesiredHeight();
        if (positionInteractionPanel_->isExpanded()) {
          desiredHeight_ += marginBig_;
        } else {
          desiredHeight_ += marginSmall_;
        }
      }
    }
    if (auto parent = getParentComponent()) {
      parent->resized();
    }
  }

  std::unique_ptr<EarColourIndicator> colourIndicator_;
  std::unique_ptr<Label> nameLabel_;
  std::unique_ptr<LevelMeter> levelMeter_;
  std::unique_ptr<Label> metadataLabel_;
  std::unique_ptr<Label> importanceLabel_;
  std::unique_ptr<EarIncDecSlider> importanceSlider_;
  std::unique_ptr<EarButton> settingsButton_;
  std::unique_ptr<EarExpansionPanel> onOffInteractionPanel_;
  std::unique_ptr<EarExpansionPanel> gainInteractionPanel_;
  std::unique_ptr<EarExpansionPanel> positionInteractionPanel_;
  std::unique_ptr<GainInteractionSettings> gainInteractionSettings_;
  std::unique_ptr<PositionInteractionSettings> positionInteractionSettings_;

  Data data_;

  const int mainAreaHeight_ = 60;
  const int marginMini_ = 2;
  const int marginSmall_ = 5;
  const int marginBig_ = 10;

  int desiredHeight_;
  ObjectType objectType_;

  ListenerList<Listener> listeners_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ObjectView)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
