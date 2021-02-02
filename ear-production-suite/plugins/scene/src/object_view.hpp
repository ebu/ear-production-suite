#pragma once

#include "JuceHeader.h"

#include "element_view.hpp"
#include "gain_interaction_settings.hpp"
#include "position_interaction_settings.hpp"
#include "speaker_setups.hpp"
#include "../../shared/components/ear_expansion_panel.hpp"
#include "../../shared/components/ear_colour_indicator.hpp"
#include "../../shared/components/level_meter.hpp"
#include "../../shared/components/ear_slider_range.hpp"
#include "../../shared/components/look_and_feel/colours.hpp"
#include "../../shared/components/look_and_feel/fonts.hpp"
#include "programme_store.pb.h"

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
  enum class ObjectType {
    DirectSpeakers,
    Other
  };

  explicit ObjectView(ObjectType type)
      : ElementView(),
        colourIndicator_(std::make_unique<EarColourIndicator>()),
        nameLabel_(std::make_unique<Label>()),
        levelMeter_(std::make_unique<LevelMeter>()),
        metadataLabel_(std::make_unique<Label>()),
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

    nameLabel_->setFont(EarFonts::Items);
    nameLabel_->setBorderSize(BorderSize<int>{0});
    nameLabel_->setColour(Label::textColourId, EarColours::Text);
    nameLabel_->setMinimumHorizontalScale(1.f);
    addAndMakeVisible(nameLabel_.get());

    addAndMakeVisible(levelMeter_.get());

    metadataLabel_->setFont(EarFonts::Values);
    metadataLabel_->setColour(Label::textColourId,
                              EarColours::Text.withAlpha(Emphasis::medium));
    addAndMakeVisible(metadataLabel_.get());

    settingsButton_->setButtonText("Settings");
    settingsButton_->setClickingTogglesState(true);
    settingsButton_->onStateChange = [this]() {
      data_.object.set_show_settings(this->settingsButton_->getToggleState());
      onOffInteractionPanel_->setVisible(
          this->settingsButton_->getToggleState());
      gainInteractionPanel_->setVisible(
          this->settingsButton_->getToggleState());
      if(objectType_ != ObjectType::DirectSpeakers) {
        positionInteractionPanel_->setVisible(
            this->settingsButton_->getToggleState());
      }
      updateDesiredHeight();
      this->notifyDataChange();
    };
    addAndMakeVisible(settingsButton_.get());

    onOffInteractionPanel_->setHeading("On/Off Interaction");
    onOffInteractionPanel_->onStateChange = [this]() {
      data_.object.mutable_interactive_on_off()->set_enabled(
          this->onOffInteractionPanel_->isExpanded());
      this->notifyDataChange();
    };
    addAndMakeVisible(onOffInteractionPanel_.get());

    gainInteractionPanel_->setContentHeight(50);
    gainInteractionPanel_->setComponent(gainInteractionSettings_.get());
    gainInteractionPanel_->setHeading("Gain Interaction");
    gainInteractionPanel_->onStateChange = [this]() {
      data_.object.mutable_interactive_gain()->set_enabled(
          this->gainInteractionPanel_->isExpanded());
      this->notifyDataChange();
    };
    gainInteractionSettings_->gainSliderRange->addListener(this);
    gainInteractionSettings_->gainMinSlider->addListener(this);
    gainInteractionSettings_->gainMaxSlider->addListener(this);
    addAndMakeVisible(gainInteractionPanel_.get());

    positionInteractionPanel_->setContentHeight(90);
    positionInteractionPanel_->setComponent(positionInteractionSettings_.get());
    positionInteractionPanel_->setHeading("Position Interaction");
    positionInteractionPanel_->onStateChange = [this]() {
      data_.object.mutable_interactive_position()->set_enabled(
          this->positionInteractionPanel_->isExpanded());
      this->notifyDataChange();
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
            
    settingsButtonBackgroundColour = settingsButton_->findColour(EarButton::backgroundColourId);

    updateDesiredHeight();
  }

  void setData(const Data& data) {
    data_ = data;
    colourIndicator_->setColour(Colour(data_.item.colour()));
    nameLabel_->setText(String(data_.item.name()), dontSendNotification);
    if (data_.item.has_ds_metadata()) {
      auto layoutIndex = data_.item.ds_metadata().layout();
      if (layoutIndex >= 0 && layoutIndex < SPEAKER_SETUPS.size()) {
        metadataLabel_->setText(
            String(SPEAKER_SETUPS.at(layoutIndex).commonName),
            dontSendNotification);
      }
    } else {
      metadataLabel_->setText("", dontSendNotification);
    }

    settingsButton_->setToggleState(data_.object.show_settings(),
                                    dontSendNotification);
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
    repaint();
  }

  Data getData() { return data_; }

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
    metadataLabel_->setBounds(mainArea.removeFromRight(150).reduced(10, 5));

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
    const Colour col = (data_.object.mutable_interactive_gain()->enabled() || data_.object.mutable_interactive_on_off()->enabled() || data_.object.mutable_interactive_position()->enabled()) ? Colours::orange : settingsButtonBackgroundColour;
                                           
    settingsButton_->setColour(EarButton::backgroundColourId, col);
    
    Component::BailOutChecker checker(this);
    listeners_.callChecked(
        checker, [&](Listener& l) { l.objectDataChanged(this->data_); });
    if (checker.shouldBailOut()) {
      return;
    }
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
      if(objectType_ != ObjectType::DirectSpeakers) {
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
                       
  Colour settingsButtonBackgroundColour{Colours::black};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ObjectView)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
