#include "element_view.hpp"

#include "../../shared/components/look_and_feel/colours.hpp"
#include "../../shared/components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

ElementView::ElementView()
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

void ElementView::paint(Graphics& g) {
  g.fillAll(EarColours::ObjectItemBackground);
  if (isMouseOver(true)) {
    g.fillAll(EarColours::Area02dp);
  }
}

void ElementView::mouseDown(const MouseEvent& event) {
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

void ElementView::mouseEnter(const MouseEvent& event) { repaint(); }

void ElementView::mouseExit(const MouseEvent& event) { repaint(); }

void ElementView::mouseUp(const MouseEvent& event) { setAlpha(Emphasis::full); }

void ElementView::resized() { setSize(getParentWidth(), 100); }

EarButton* ElementView::getHandleButton() { return handleButton_.get(); }

EarButton* ElementView::getRemoveButton() { return removeButton_.get(); }

}  // namespace ui
}  // namespace plugin
}  // namespace ear
