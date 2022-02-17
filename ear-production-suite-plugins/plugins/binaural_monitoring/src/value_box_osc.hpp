#pragma once

#include "orientation_osc.hpp"

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/ear_button.hpp"
#include "components/ear_slider.hpp"

namespace ear {
namespace plugin {
namespace ui {

    class TooltipLookAndFeel : public LookAndFeel_V4
    {
    public:
      TooltipLookAndFeel()
      {
        setColour(TooltipWindow::backgroundColourId, EarColours::Primary.darker());
        setColour(TooltipWindow::textColourId, juce::Colours::white);
      }

      TextLayout layoutTooltipText (const String& text, Colour colour) noexcept
      {
        const float tooltipFontSize = 13.0f;
        const int maxToolTipWidth = 400;

        AttributedString s;
        s.setJustification (Justification::centred);
        s.append (text, Font (tooltipFontSize, Font::plain), colour);

        TextLayout tl;
        tl.createLayoutWithBalancedLineLengths (s, (float) maxToolTipWidth);
        return tl;
      }

      void drawTooltip (Graphics& g, const String& text, int width, int height) override
      {
        Rectangle<int> bounds (width, height);
        auto cornerSize = 5.0f;

        g.setColour (findColour (TooltipWindow::backgroundColourId));
        g.fillRoundedRectangle (bounds.toFloat(), cornerSize);

        g.setColour (findColour (TooltipWindow::outlineColourId));
        g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

        layoutTooltipText (text, findColour (TooltipWindow::textColourId))
          .draw (g, { static_cast<float> (width), static_cast<float> (height) });
      }

    };


class ValueBoxOsc : public Component {
public:

  void setInputTypeHighlight(ListenerOrientationOscReceiver::InputType inputType, bool forceUpdate = false) {
    const auto activeText = EarColours::Label.withAlpha(0.5f);
    const auto inactiveText = EarColours::Label.withAlpha(0.25f);
    const auto activeTick = EarColours::PrimaryVariant;
    const auto inactiveTick = EarColours::Primary;

    if(inputType == lastInputTypeGuiUpdate && !forceUpdate) return;

    if(inputType == ListenerOrientationOscReceiver::InputType::Euler) {
      invY_->setColour(ToggleButton::textColourId, activeText);
      invY_->setColour(ToggleButton::tickColourId, activeTick);
      invP_->setColour(ToggleButton::textColourId, activeText);
      invP_->setColour(ToggleButton::tickColourId, activeTick);
      invR_->setColour(ToggleButton::textColourId, activeText);
      invR_->setColour(ToggleButton::tickColourId, activeTick);
    } else {
      invY_->setColour(ToggleButton::textColourId, inactiveText);
      invY_->setColour(ToggleButton::tickColourId, inactiveTick);
      invP_->setColour(ToggleButton::textColourId, inactiveText);
      invP_->setColour(ToggleButton::tickColourId, inactiveTick);
      invR_->setColour(ToggleButton::textColourId, inactiveText);
      invR_->setColour(ToggleButton::tickColourId, inactiveTick);
    }

    if(inputType == ListenerOrientationOscReceiver::InputType::Quaternion) {
      invQW_->setColour(ToggleButton::textColourId, activeText);
      invQW_->setColour(ToggleButton::tickColourId, activeTick);
      invQX_->setColour(ToggleButton::textColourId, activeText);
      invQX_->setColour(ToggleButton::tickColourId, activeTick);
      invQY_->setColour(ToggleButton::textColourId, activeText);
      invQY_->setColour(ToggleButton::tickColourId, activeTick);
      invQZ_->setColour(ToggleButton::textColourId, activeText);
      invQZ_->setColour(ToggleButton::tickColourId, activeTick);
    } else {
      invQW_->setColour(ToggleButton::textColourId, inactiveText);
      invQW_->setColour(ToggleButton::tickColourId, inactiveTick);
      invQX_->setColour(ToggleButton::textColourId, inactiveText);
      invQX_->setColour(ToggleButton::tickColourId, inactiveTick);
      invQY_->setColour(ToggleButton::textColourId, inactiveText);
      invQY_->setColour(ToggleButton::tickColourId, inactiveTick);
      invQZ_->setColour(ToggleButton::textColourId, inactiveText);
      invQZ_->setColour(ToggleButton::tickColourId, inactiveTick);
    }

    lastInputTypeGuiUpdate = inputType;
  }

   ValueBoxOsc() :
        enableButton_(std::make_shared<EarButton>()),
        invertLabel_(std::make_unique<Label>()),
        invY_(std::make_shared<ToggleButton>()),
        invP_(std::make_shared<ToggleButton>()),
        invR_(std::make_shared<ToggleButton>()),
        invQW_(std::make_shared<ToggleButton>()),
        invQX_(std::make_shared<ToggleButton>()),
        invQY_(std::make_shared<ToggleButton>()),
        invQZ_(std::make_shared<ToggleButton>()),
        portLabel_(std::make_unique<Label>()),
        portControl_(std::make_shared<EarSlider>()),
        statusLabel_(std::make_shared<Label>()) {
    setColour(backgroundColourId, EarColours::Area01dp);

    enableButton_->setButtonText("Enable OSC");
    enableButton_->setShape(EarButton::Shape::Toggle);
    // Note: Do NOT enableButton_->setClickingTogglesState(true);
    /// It is false by default and State is handled completely by
    ///  BinauralMonitoringJuceFrontendConnector
    enableButton_->setEnabled(true);
    enableButton_->setTooltip ("Note: To minimise head-tracking latency, turn off any anticipative processing in the DAW.");
    addAndMakeVisible(enableButton_.get());

    tooltipWindow.setLookAndFeel(&tooltipLookAndFeel);
    tooltipWindow.setOpaque(false);

    portLabel_->setFont(EarFonts::Values); //(EarFonts::Label);
    portLabel_->setColour(Label::textColourId, EarColours::Label);
    portLabel_->setText("Port", juce::NotificationType::dontSendNotification);
    portLabel_->setJustificationType(Justification::left);
    addAndMakeVisible(portLabel_.get());

    portControl_->setSliderStyle(Slider::IncDecButtons);
    portControl_->setTextBoxStyle(Slider::TextBoxRight, false, 44, 40);
    portControl_->setRange(1, 65535, 1);
    portControl_->setNumDecimalPlacesToDisplay(0);
    portControl_->setDoubleClickReturnValue(true, 8000);
    portControl_->setMouseDragSensitivity(65535);
    portControl_->setIncDecButtonsMode(juce::Slider::IncDecButtonMode::incDecButtonsNotDraggable);
    addAndMakeVisible(portControl_.get());

    statusLabel_->setFont(EarFonts::Units);
    statusLabel_->setColour(Label::textColourId, EarColours::Label);
    statusLabel_->setText("Initialising...", juce::NotificationType::dontSendNotification);
    statusLabel_->setJustificationType(Justification::left);
    addAndMakeVisible(statusLabel_.get());

    invertLabel_->setFont(EarFonts::Label);
    invertLabel_->setColour(Label::textColourId, EarColours::Label);
    invertLabel_->setText("Inversion", juce::NotificationType::dontSendNotification);
    invertLabel_->setJustificationType(Justification::left);
    addAndMakeVisible(invertLabel_.get());

    setInputTypeHighlight(ListenerOrientationOscReceiver::InputType::None, true);

    // Note: MUST setClickingTogglesState(false) on ToggleButtons;
    /// It is true by default and State is handled completely by
    ///  BinauralMonitoringJuceFrontendConnector

    invY_->setButtonText("Yaw");
    invY_->setClickingTogglesState(false);
    addAndMakeVisible(invY_.get());

    invP_->setButtonText("Pitch");
    invP_->setClickingTogglesState(false);
    addAndMakeVisible(invP_.get());

    invR_->setButtonText("Roll");
    invR_->setClickingTogglesState(false);
    addAndMakeVisible(invR_.get());

    invQW_->setButtonText("Quat W");
    invQW_->setClickingTogglesState(false);
    addAndMakeVisible(invQW_.get());

    invQX_->setButtonText("Quat X");
    invQX_->setClickingTogglesState(false);
    addAndMakeVisible(invQX_.get());

    invQY_->setButtonText("Quat Y");
    invQY_->setClickingTogglesState(false);
    addAndMakeVisible(invQY_.get());

    invQZ_->setButtonText("Quat Z");
    invQZ_->setClickingTogglesState(false);
    addAndMakeVisible(invQZ_.get());
  }

  ~ValueBoxOsc() {}

  void paint(Graphics& g) override {
    // background
    g.fillAll(findColour(backgroundColourId));
  }

  enum ColourIds {
    backgroundColourId = 0x00010001,
  };

  void resized() override {
    auto area = getLocalBounds();
    float yReduction = ((float)area.getHeight() - (narrowRowHeight_ * 2.f)) / 4.f;
    area.reduce(marginSmall_, yReduction);
    area.removeFromLeft(marginBig_);

    auto bottomRow = area.removeFromBottom(area.getHeight() - narrowRowHeight_ - marginSmall_);
    area = area.removeFromTop(narrowRowHeight_);

    enableButton_->setBounds(area.removeFromLeft(140));
    portLabel_->setBounds(area.removeFromLeft(35));
    portControl_->setBounds(area.removeFromLeft(50));
    area.removeFromLeft(marginBig_ + marginBig_);
    statusLabel_->setBounds(area);

    bottomRow.removeFromLeft(marginBig_ * 2);
    invertLabel_->setBounds(bottomRow.removeFromLeft(80));

    int squash = (bottomRow.getHeight() - 16) / 2;
    bottomRow.removeFromTop(squash);
    bottomRow.removeFromBottom(squash);
    invY_->setBounds(bottomRow.removeFromLeft(60));
    invP_->setBounds(bottomRow.removeFromLeft(60));
    invR_->setBounds(bottomRow.removeFromLeft(60));
    invQW_->setBounds(bottomRow.removeFromLeft(75));
    invQX_->setBounds(bottomRow.removeFromLeft(75));
    invQY_->setBounds(bottomRow.removeFromLeft(75));
    invQZ_->setBounds(bottomRow.removeFromLeft(75));

  }

  std::shared_ptr<ear::plugin::ui::EarButton> getEnableButton() { return enableButton_; }
  std::shared_ptr<EarSlider> getPortControl() { return portControl_; }
  std::shared_ptr<Label> getStatusLabel() { return statusLabel_; }
  std::shared_ptr<ToggleButton> getInvertYawButton() { return invY_; }
  std::shared_ptr<ToggleButton> getInvertPitchButton() { return invP_; }
  std::shared_ptr<ToggleButton> getInvertRollButton() { return invR_; }
  std::shared_ptr<ToggleButton> getInvertQuatWButton() { return invQW_; }
  std::shared_ptr<ToggleButton> getInvertQuatXButton() { return invQX_; }
  std::shared_ptr<ToggleButton> getInvertQuatYButton() { return invQY_; }
  std::shared_ptr<ToggleButton> getInvertQuatZButton() { return invQZ_; }

private:
  std::unique_ptr<Label> portLabel_;
  std::shared_ptr<ear::plugin::ui::EarButton> enableButton_;
  std::shared_ptr<EarSlider> portControl_;
  std::shared_ptr<Label> statusLabel_;
  std::unique_ptr<Label> invertLabel_;
  std::shared_ptr<ToggleButton> invY_, invP_, invR_, invQW_, invQX_, invQY_, invQZ_;

  ListenerOrientationOscReceiver::InputType lastInputTypeGuiUpdate;

  TooltipWindow tooltipWindow{ this };
  TooltipLookAndFeel tooltipLookAndFeel;

  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;
  const float narrowRowHeight_ = 24.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxOsc)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
