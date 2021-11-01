#pragma once

#include "JuceHeader.h"

#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class LevelDisplayBox : public Component {
 public:

   struct Data {
     ear::plugin::proto::InputItemMetadata item;
     ear::plugin::proto::HoaTypeMetadata hoa;
  };

  LevelDisplayBox() : levelMeter_(std::make_unique<LevelMeter>()) {}
  ~LevelDisplayBox() {}

  void updateLevelMeterBounds() {  // This seems to be where we place the
                                    // channel gain boxes
    auto area = getLocalBounds();
    area.removeFromBottom(350); /*
    for (auto levelMeter : levelMeters_) {
      levelMeter->setBounds(area.removeFromLeft(50));
      area.removeFromLeft(6);
    }*/
  }

    void paint(Graphics& g) override {
    auto area = getLocalBounds();
    g.fillAll(EarColours::Label);//added temp
    /* if (levelMeters_.empty()) {
      //g.fillAll(EarColours::Area01dp);
      g.fillAll(EarColours::Label);//added temp
      float lineThickness = 1.f;
      g.setColour(EarColours::Error);
      g.drawRect(
          area.toFloat().reduced(lineThickness / 2.f, lineThickness / 2.f),
          lineThickness);
      area.reduce(20.f, 30.f);

      g.setColour(EarColours::Text.withAlpha(Emphasis::medium));
      g.setFont(EarFonts::Label);
      g.drawText("Level Meters not available", area.removeFromTop(25.f),
                 Justification::left);
      g.setColour(EarColours::Text.withAlpha(Emphasis::high));
      g.setFont(EarFonts::Heading);
      g.drawText("Please select a speaker layout first",
                 area.removeFromTop(25.f), Justification::left);
    }*/
  }

  void resized() override { updateLevelMeterBounds(); }
  /*
  void addOrderBox(LevelMeter* levelMeter) {
    levelMeters_.push_back(levelMeter);
    addAndMakeVisible(levelMeter);
    updateLevelMeterBounds();
    repaint();
  }

  void removeAllOrderBoxes() {
    removeAllChildren();
    levelMeters_.clear();
    repaint();
  };*/
  
  LevelMeter* getLevelMeter() { return levelMeter_.get(); } 
    
  void setData() {
    //data_ = data;
    //colourIndicator_->setColour(Colour(data_.item.colour()));
    //nameLabel_->setText(String(data_.item.name()), dontSendNotification);
    repaint();
  }

  Data getData() { return data_; }
  
  class Listener {
  public:
    virtual ~Listener() = default;
    virtual void objectDataChanged(LevelDisplayBox::Data data) = 0;
  };
  
  void addListener(Listener* l) { listeners_.add(l); }
  void removeListener(Listener* l) { listeners_.remove(l); }

private:
  //std::vector<LevelMeter*> levelMeters_;
  std::unique_ptr<LevelMeter> levelMeter_;

  Data data_;

  ListenerList<Listener> listeners_;

  void notifyDataChange() {
    Component::BailOutChecker checker(this);
    listeners_.callChecked(
        checker, [&](Listener& l) { l.objectDataChanged(this->data_); });
    if (checker.shouldBailOut()) {
      return;
    }
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelDisplayBox)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
