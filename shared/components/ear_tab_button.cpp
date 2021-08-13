#include "ear_tabbed_component.hpp"

#include "ear_button.hpp"
#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"

#include <cmath>

using namespace ear::plugin::ui;

EarTabButton::EarTabButton() : EarTabButton(String()) {}

EarTabButton::EarTabButton(const String& text)
    : text_(text), removeButton_(std::make_unique<EarButton>()) {
  setColour(backgroundColourId, EarColours::Area06dp);
  setColour(highlightBackgroundColourId, EarColours::Area06dp);
  setColour(highlightColourId, EarColours::Primary);
  setColour(textColourId, EarColours::Text);
  setColour(hoverColourId, EarColours::Area01dp);

  removeButton_->setOffStateIcon(
      std::unique_ptr<Drawable>(Drawable::createFromImageData(
          binary_data::delete_icon_svg, binary_data::delete_icon_svgSize)),
      Emphasis::medium);
  removeButton_->setHoverStateIcon(std::unique_ptr<Drawable>(
      Drawable::createFromImageData(binary_data::delete_icon_red_svg,
                                    binary_data::delete_icon_red_svgSize)));
  removeButton_->setColour(EarButton::highlightColourId,
                           EarColours::Transparent);
  removeButton_->setColour(EarButton::backgroundColourId,
                           EarColours::Transparent);
  removeButton_->setColour(EarButton::hoverColourId, EarColours::Transparent);
  removeButton_->onClick = [this]() {
    if (onCloseClick) {
      onCloseClick(this);
    }
  };
  addAndMakeVisible(removeButton_.get());

  addMouseListener(this, true);
};

void EarTabButton::setText(const String& text) {
  if (text_ != text) {
    text_ = text;
    repaint();
  }
}

String EarTabButton::getText() const { return text_; }

void EarTabButton::setSelected(bool selected) {
  if (selected_ != selected) {
    selected_ = selected;
    repaint();
  }
}
bool EarTabButton::isSelected() const { return selected_; }

void EarTabButton::paint(Graphics& g) {
  float alpha = Emphasis::medium;
  if (isSelected()) {
    alpha = 1.f;
  }
  g.beginTransparencyLayer(alpha);
  auto area = getLocalBounds();
  g.fillAll(findColour(highlightBackgroundColourId));
  if (isMouseOver(true)) {
    g.setColour(findColour(hoverColourId));
    g.fillAll(findColour(hoverColourId));
  }
  g.setColour(findColour(textColourId));
  g.setFont(EarFonts::Items);
  g.drawText(text_,
             area.withTrimmedLeft(paddingLeft_)
                 .withTrimmedRight(paddingRight_)
                 .withTrimmedTop(paddingTop_)
                 .withTrimmedBottom(paddingBottom_),
             Justification::centred, true);
  g.setColour(findColour(highlightColourId));
  if (isSelected()) {
    g.fillRect(area.removeFromBottom(4));
  }
  g.endTransparencyLayer();
}

void EarTabButton::resized() {
  auto area = getLocalBounds();
  area.removeFromBottom(4);
  removeButton_->setBounds(area.removeFromRight(area.getHeight()));
}

void EarTabButton::mouseEnter(const MouseEvent& event) { repaint(); }
void EarTabButton::mouseExit(const MouseEvent& event) { repaint(); }

void EarTabButton::mouseDown(const MouseEvent& event) {
  if (isEnabled() && event.getNumberOfClicks() == 1) {
    if (onClick) {
      onClick(this);
    }
  }
}

MouseCursor EarTabButton::getMouseCursor() {
  if (isEnabled()) {
    return MouseCursor::PointingHandCursor;
  }
  return MouseCursor::NormalCursor;
}
