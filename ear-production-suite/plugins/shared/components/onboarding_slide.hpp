#pragma once

#include "JuceHeader.h"

#include "../binary_data.hpp"
#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class OnboardingSlide : public Component {
 public:
  OnboardingSlide() {}
  ~OnboardingSlide() {}

  void setHeading(const String& heading) {
    heading_ = AttributedString{heading};
    heading_.setColour(EarColours::Text);
    heading_.setFont(font::RobotoSingleton::instance().getRegular(30.f));
    heading_.setLineSpacing(8.f);
    repaint();
  }

  void setText(const AttributedString& text) {
    mainText_ = text;
    repaint();
  }

  void setBackgroundImage(std::unique_ptr<Drawable> image) {
    backgroundImage_ = std::move(image);
    repaint();
  }
  void setSlideImage(std::unique_ptr<Drawable> image) {
    slideImage_ = std::move(image);
    auto imageBounds = slideImage_->getDrawableBounds();
    slideImageTransform_ = slideImageTransform_.scaled(0.3f, 0.3f)
                               .translated(200.f, 290.f)
                               .translated(-imageBounds.getWidth() * 0.15f,
                                           -imageBounds.getHeight() * 0.15f);
    repaint();
  }

  void paint(Graphics& g) override {
    if (backgroundImage_) {
      backgroundImage_->drawWithin(g, getLocalBounds().toFloat(),
                                   RectanglePlacement::fillDestination, 1.f);
    }
    g.fillAll(EarColours::Overlay);
    heading_.draw(g, headingRect_);
    mainText_.draw(g, mainTextRect_);
    if (slideImage_) {
      slideImage_->draw(g, 1.f, slideImageTransform_);
      // slideImage_->drawWithin(g, imageRect_,
      //                         RectanglePlacement::fillDestination, 1.f);
    }
  }

  void resized() override {
    auto area = getLocalBounds();
    headingRect_ = juce::Rectangle<float>{365, 63, 320, 79};
    mainTextRect_ = juce::Rectangle<float>{365, 176, 272, 276};
    imageRect_ = juce::Rectangle<float>{18, 100, 328, 400};
  }

 private:
  AttributedString heading_;
  juce::Rectangle<float> headingRect_;
  AttributedString mainText_;
  juce::Rectangle<float> mainTextRect_;
  std::unique_ptr<Drawable> backgroundImage_;
  std::unique_ptr<Drawable> slideImage_;
  AffineTransform slideImageTransform_;
  juce::Rectangle<float> imageRect_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OnboardingSlide)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
