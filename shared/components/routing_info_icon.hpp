#pragma once

#include "JuceHeader.h"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

    inline std::unique_ptr<ImageComponent> createRoutingInfoIcon() {
        auto routingIcon = std::make_unique<ImageComponent>();
        routingIcon->setImage(ImageFileFormat::loadFrom(
            binary_data::infologo_png, binary_data::infologo_pngSize));
        routingIcon->setImagePlacement(RectanglePlacement::centred +
            RectanglePlacement::doNotResize);
        routingIcon->setAlpha(0.8);
        routingIcon->setMouseCursor(MouseCursor::PointingHandCursor);
        routingIcon->setTooltip(
            "128 channel support requires REAPER v7 or later.");
        return std::move(routingIcon);
    }

    inline void placeRoutingInfoIconRight(juce::Rectangle<int>& area, ImageComponent* routingInfoIcon) {
        auto routingInfoArea = area.removeFromRight(13);
        // Need an odd height to avoid blurring through anti-aliasing
        // (since icon is odd height too, so ensures integer padding when centring vertically)
        if (routingInfoArea.getHeight() % 2 == 0) {
            routingInfoArea.removeFromTop(1);
        }
        routingInfoIcon->setBounds(routingInfoArea);
    }

}  // namespace ui
}  // namespace plugin
}  // namespace ear
