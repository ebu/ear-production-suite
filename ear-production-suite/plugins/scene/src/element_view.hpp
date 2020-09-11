#pragma once

#include "JuceHeader.h"

#include "../../shared/components/look_and_feel/colours.hpp"
#include "../../shared/components/look_and_feel/fonts.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class ElementView : public Component {
 public:
  ElementView()
      : handleButton_(std::make_unique<EarButton>()),
        removeButton_(std::make_unique<EarButton>()) {
    handleButton_->setOffStateIcon(std::unique_ptr<Drawable>(
        Drawable::createFromImageData(binary_data::drag_handle_icon_svg,
                                      binary_data::drag_handle_icon_svgSize)));
    handleButton_->setColour(EarButton::highlightColourId,
                             EarColours::Transparent);
    handleButton_->setColour(EarButton::backgroundColourId,
                             EarColours::Transparent);
    handleButton_->setColour(EarButton::hoverColourId, EarColours::Transparent);
    addAndMakeVisible(handleButton_.get());

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

    addAndMakeVisible(removeButton_.get());

    removeMouseListener(this);
    addMouseListener(this, true);
  }
  void paint(Graphics& g) override {
    g.fillAll(EarColours::ObjectItemBackground);
    if (isMouseOver(true)) {
      g.fillAll(EarColours::Area02dp);
    }
  }

  void mouseDown(const MouseEvent& event) override {
    DragAndDropContainer* dragContainer =
        DragAndDropContainer::findParentDragContainerFor(this);

    if (handleButton_->isMouseOver() && !dragContainer->isDragAndDropActive()) {
      auto dragStartOffset_ =
          -getLocalPoint(event.eventComponent, event.getPosition());
      auto image = createComponentSnapshot(getLocalBounds());
      image.multiplyAllAlphas(Emphasis::medium);
      dragContainer->startDragging("element", this, image, true,
                                   &dragStartOffset_);
      setAlpha(Emphasis::disabled);
    }
  }

  virtual int getDesiredHeight() = 0;

  void mouseEnter(const MouseEvent& event) override { repaint(); }
  void mouseExit(const MouseEvent& event) override { repaint(); }
  void mouseUp(const MouseEvent& event) override { setAlpha(Emphasis::full); }

  void resized() override { setSize(getParentWidth(), 100); }

  EarButton* getHandleButton() { return handleButton_.get(); }
  EarButton* getRemoveButton() { return removeButton_.get(); }

 protected:
  std::unique_ptr<EarButton> handleButton_;
  std::unique_ptr<EarButton> removeButton_;

  const float marginBig_ = 20;
  const float marginSmall_ = 10;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElementView)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
