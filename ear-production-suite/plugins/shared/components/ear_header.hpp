#pragma once

#include "JuceHeader.h"

#include "../helper/graphics.hpp"
#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"
#include "look_and_feel/shadows.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class EarHeader : public Component {
 public:
  EarHeader() :
      logo_{Drawable::createFromImageData(binary_data::EARlogowhite_transparentBG_PS_small_png,
                                          binary_data::EARlogowhite_transparentBG_PS_small_pngSize)}
  {
  }

  void setText(std::string const& text) {
    headerText_ = AttributedString(text);
    headerText_.setFont(EarFonts::HeroHeading);
    headerText_.setColour(EarColours::Text);
    repaint();
  }

  void resized() override {
    auto area = getLocalBounds();
    area.reduce(10, 5);
    auto logoBounds = logo_->getDrawableBounds();
    logoTransform_ = AffineTransform{}.translated(area.getX(), area.getY());
    area.removeFromLeft(logoBounds.getWidth());
    area.removeFromTop(9);
    textRect_ = area;
  }

  void paint(Graphics& g) override {
    g.fillAll(EarColours::Background);
    logo_->draw(g, 1.f, logoTransform_);
    headerText_.draw(g, textRect_.toFloat());
  }
 private:
  std::unique_ptr<Drawable> logo_;
  AttributedString headerText_;
  AffineTransform logoTransform_;
  juce::Rectangle<int> textRect_;
};

}
}
}
