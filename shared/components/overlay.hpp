#pragma once

#include "JuceHeader.h"

#include "ear_button.hpp"
#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class Overlay : public Component {
 public:
  Overlay() : closeButton_(std::make_unique<EarButton>()) {
    setColour(overlayBackgroundColourId, EarColours::Overlay);
    setColour(contentBackgroundColourId, EarColours::Background);
    setColour(windowBorderColourId, EarColours::WindowBorder);
    setColour(headerTextColourId, EarColours::Text);
    setAlwaysOnTop(true);

    closeButton_->setShape(EarButton::Shape::Circular);
    closeButton_->setOffStateIcon(
        std::unique_ptr<Drawable>(Drawable::createFromImageData(
            binary_data::close_icon_svg, binary_data::close_icon_svgSize)),
        Emphasis::medium);
    closeButton_->setOnStateIcon(
        std::unique_ptr<Drawable>(Drawable::createFromImageData(
            binary_data::close_icon_svg, binary_data::close_icon_svgSize)),
        Emphasis::medium);
    closeButton_->onClick = [this] {
      setVisible(false);
      if (onClose) {
        onClose();
      }
    };
    addAndMakeVisible(closeButton_.get());
  }

  enum ColourIds {
    overlayBackgroundColourId = 0x00120001,
    contentBackgroundColourId = 0x00120002,
    windowBorderColourId = 0x00120003,
    headerTextColourId = 0x00120004
  };

  void setContent(Component *content) {
    if (content_) {
      removeChildComponent(content_);
    }
    content_ = content;
    if (content_) {
      addAndMakeVisible(content_);
    }
  }

  void setWindowSize(int width, int height) {
    if (windowWidth_ != width || windowHeight_ != height) {
      windowWidth_ = width;
      windowHeight_ = height;
      resized();
      repaint();
    }
  }

  void setHeaderHeight(int height) {
    if (headerHeight_ != height) {
      headerHeight_ = height;
      resized();
      repaint();
    }
  }

  void setHeaderText(const String &headerText) {
    if (headerText_ != headerText) {
      headerText_ = headerText;
      repaint();
    }
  }

  // void mouseDown(const MouseEvent &event) override { setVisible(false); }

  void paint(Graphics &g) override {
    g.fillAll(findColour(overlayBackgroundColourId));
    g.setColour(findColour(windowBorderColourId));
    g.fillRect(windowRect_);
    g.setFont(EarFonts::Items);
    g.setColour(findColour(headerTextColourId));
    g.drawText(headerText_, headerRect_, Justification::centredLeft);
    g.setColour(findColour(contentBackgroundColourId));
    g.fillRect(contentRect_);
  };

  void resized() override {
    windowRect_ = juce::Rectangle<int>{0, 0, windowWidth_, windowHeight_};
    windowRect_.setCentre(
        juce::Point<float>{getWidth() / 2.f, getHeight() / 2.f}.roundToInt());
    contentRect_ = windowRect_;
    headerRect_ = contentRect_.removeFromTop(headerHeight_ + borderWidth_);
    headerRect_ = headerRect_.withTrimmedLeft(headerPadding_)
                      .withTrimmedTop(borderWidth_)
                      .withTrimmedRight(borderWidth_);

    contentRect_.reduce(borderWidth_, borderWidth_);
    if (content_) {
      content_->setBounds(contentRect_);
    }
    closeButton_->setBounds(
        headerRect_.removeFromRight(headerHeight_).reduced(4));
  }

  std::function<void()> onClose;

 private:
  int windowWidth_;
  int windowHeight_;
  int borderWidth_ = 2;
  int headerHeight_ = 31;
  int headerPadding_ = 11;
  String headerText_;
  juce::Rectangle<int> windowRect_;
  juce::Rectangle<int> headerRect_;
  juce::Rectangle<int> contentRect_;
  std::unique_ptr<EarButton> closeButton_;
  Component *content_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Overlay)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
