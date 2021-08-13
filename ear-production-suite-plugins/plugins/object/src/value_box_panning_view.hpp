#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/panner_side_view.hpp"
#include "components/panner_top_view.hpp"

namespace ear {
namespace plugin {
namespace ui {

class ValueBoxPanningView : public Component {
 public:
  ValueBoxPanningView()
      : pannerTopView_(std::make_shared<PannerTopView>()),
        pannerSideView_(std::make_shared<PannerSideView>()) {
    setColour(backgroundColourId, EarColours::Area01dp);

    addAndMakeVisible(pannerTopView_.get());
    addAndMakeVisible(pannerSideView_.get());
  }

  ~ValueBoxPanningView() {}

  void paint(Graphics& g) override {
    // background
    g.fillAll(findColour(backgroundColourId));
  }

  enum ColourIds {
    backgroundColourId = 0x00010001,
  };

  void resized() override {
    auto area = getLocalBounds();
    area.reduce(10, 5);
    pannerTopView_->setBounds(area.removeFromLeft(275));
    pannerSideView_->setBounds(area);
  }

  Value& getAzimuthValueObject() {
    return pannerTopView_->getAzimuthValueObject();
  }
  Value& getElevationValueObject() {
    return pannerSideView_->getElevationValueObject();
  }
  Value& getDistanceValueObject() {
    return pannerTopView_->getDistanceValueObject();
  }

  std::shared_ptr<PannerTopView> getPannerTopView() { return pannerTopView_; }
  std::shared_ptr<PannerSideView> getPannerSideView() {
    return pannerSideView_;
  }

 private:
  std::shared_ptr<PannerTopView> pannerTopView_;
  std::shared_ptr<PannerSideView> pannerSideView_;

  const float marginSmall_ = 5.f;
  const float marginBig_ = 10.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBoxPanningView)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
