#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "input_item_metadata.pb.h"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class ItemView : public Component {
 public:
  struct Data {
    ear::plugin::proto::InputItemMetadata metadata;
    bool selected;
  };

  ItemView() {
    onStateIcon_ = std::unique_ptr<Drawable>(Drawable::createFromImageData(
        binary_data::check_box_icon_svg, binary_data::check_box_icon_svgSize));
    offStateIcon_ = std::unique_ptr<Drawable>(Drawable::createFromImageData(
        binary_data::check_box_outline_blank_icon_svg,
        binary_data::check_box_outline_blank_icon_svgSize));
    setSize(getParentWidth(), 46);
  }
  void paint(Graphics& g) override {
    auto area = getLocalBounds();
    area.reduce(0, 1);
    if (data_.selected) {
      onStateIcon_->drawWithin(
          g, area.removeFromLeft(33).toFloat(),
          RectanglePlacement::centred | RectanglePlacement::doNotResize,
          Emphasis::medium);
    } else {
      offStateIcon_->drawWithin(
          g, area.removeFromLeft(33).toFloat(),
          RectanglePlacement::centred | RectanglePlacement::doNotResize,
          Emphasis::medium);
    }
    g.setColour(EarColours::Area01dp);
    g.fillRect(area);
    if (isEnabled() && isMouseOver(true)) {
      g.setColour(EarColours::Area02dp);
      g.fillRect(area);
    }
    g.setColour(Colour(data_.metadata.colour()));
    g.fillRect(area.removeFromLeft(5));
    area.removeFromLeft(10);
    g.setColour(EarColours::Text);
    g.setFont(EarFonts::Items);
    g.drawText(data_.metadata.name(), area, Justification::left);
  }

  void setData(Data data) {
    data_ = data;
    repaint();
  }

  void setSelected(bool selected) {
    data_.selected = selected;
    repaint();
  }
  bool isSelected() { return data_.selected; }

  communication::ConnectionId getId() {
    return communication::ConnectionId(data_.metadata.connection_id());
  }

  void mouseDown(const MouseEvent& event) override {}
  void mouseEnter(const MouseEvent& event) override { repaint(); }
  void mouseExit(const MouseEvent& event) override { repaint(); }
  void mouseUp(const MouseEvent& event) override {
    if (isEnabled() && event.getNumberOfClicks() == 1) {
      data_.selected = !data_.selected;
      repaint();
    }
  }

  MouseCursor getMouseCursor() override {
    if (isEnabled()) {
      return MouseCursor::PointingHandCursor;
    }
    return MouseCursor::NormalCursor;
  }

  int getDesiredHeight() { return 46; }

 protected:
  std::unique_ptr<Drawable> onStateIcon_;
  std::unique_ptr<Drawable> offStateIcon_;

  Data data_;

  const float marginBig_ = 20;
  const float marginSmall_ = 10;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ItemView)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
