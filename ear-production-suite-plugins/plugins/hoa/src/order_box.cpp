#include "order_box.hpp"

#include "pyramid_box.hpp"
#include "value_box_order_display.hpp"
#include "hoa_plugin_processor.hpp"
#include "components/level_meter.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include <numeric>
#include "value_box_order_display.hpp"

using namespace ear::plugin::ui;

float average(std::vector<float> const& v) {// ME temporary add
  if (v.empty()) {
    return 0;
  }

  auto const count = static_cast<float>(v.size());
  return std::reduce(v.begin(), v.end()) / count;
}

OrderBox::OrderBox(HoaAudioProcessor* p,
                   ValueBoxOrderDisplay* valueBoxOrderDisplay,
                   String name,
                   int rowOrder, int hoaOrder)
  : levelMeter_(std::make_unique<LevelMeter>()),
  orderLabel_(std::make_unique<Label>()),
  rowOrder_(rowOrder),
  hoaOrder_(hoaOrder),
  p_(p) ,
  valueBoxOrderDisplay_(valueBoxOrderDisplay) {
  levelMeter_->setOrientation(LevelMeter::horizontal);
  addAndMakeVisible(levelMeter_.get());
  
  orderLabel_->setText(name, dontSendNotification);
  orderLabel_->setFont(EarFonts::Items);
  orderLabel_->setColour(Label::textColourId, EarColours::Label);
  orderLabel_->setJustificationType(Justification::centred);
  addAndMakeVisible(orderLabel_.get());

  addPyramidBoxesToOrderBox();
}
OrderBox::~OrderBox() {}
void OrderBox::paint(Graphics& g) { 

  int averageLevel = static_cast<int>(average(levelMeter_->getValues()));

  g.fillAll(EarColours::Area01dp);
  g.setColour(EarColours::Primary);
  // ME temporary add
  g.fillRect(averageLevel + levelMeter_->getX(), getLocalBounds().getY(), 5,
                     getLocalBounds().getHeight());
}

void OrderBox::resized()  {  // Here we actually set the look of the level meter
            auto area = getLocalBounds();

            orderLabel_->setBounds(area.removeFromLeft(40));
            //levelDisplayBox_->setBounds(area.removeFromLeft(200));
            levelMeter_->setBounds(area.removeFromLeft(250));

            area.removeFromBottom(5);
            area.removeFromTop(5);
            auto areaWidth = area.getWidth();

            int pyramidBoxWidth(40);
            int numberOfBoxPartitions = (hoaOrder_ * 2) + 2;
            int partitionNumber = (numberOfBoxPartitions / 2) - rowOrder_;

            for (auto pyramidBox : pyramidBoxes_) {
                auto removeFromLeft =
                    (partitionNumber * areaWidth) / numberOfBoxPartitions;
                auto pyramidBoxArea = area.withTrimmedLeft(removeFromLeft);
                pyramidBox->setBounds(pyramidBoxArea.removeFromLeft(pyramidBoxWidth));
                partitionNumber++;
            }
}

        //LevelMeter* getLevelMeter() { return levelMeter_.get(); }

void OrderBox::addPyramidBoxesToOrderBox() {

            auto numChannelsOnRow = [](int value) { return value < 0 ? 0 : value * 2 + 1; };
            auto numChannelsInOrder = [](int value) { return pow(value + 1, 2); };

            pyramidBoxes_.reserve(numChannelsOnRow(rowOrder_));

            std::vector<int> routing;
            int routingFirstChannel(p_->getRouting()->get());

            for (int i(0); i < numChannelsOnRow(rowOrder_); i++) {
                int channelRouting(routingFirstChannel +
                    numChannelsInOrder(rowOrder_ - 1) + i + 1);
                routing.push_back(channelRouting);

                std::shared_ptr<ear::plugin::ui::PyramidBox> pyramidBox =
                    std::make_shared<PyramidBox>(
                        p_->getLevelMeter(),
                        valueBoxOrderDisplay_,
                        i + 1 + static_cast<int>(numChannelsInOrder(rowOrder_ - 1)),
                        channelRouting);
                pyramidBoxes_.push_back(pyramidBox);
                addAndMakeVisible(*pyramidBox);
            }

            levelMeter_->setMeter(p_->getLevelMeter(), routing);

            //pyramidBoxes_.push_back(pyramidBox);

            //updatePyramidBoxBounds();
            //repaint();
}

void OrderBox::removeAllOrderBoxes() {
            removeAllChildren();
            pyramidBoxes_.clear();
            repaint();
}
/*bool OrderBox::clippingIsOccuringOnRow() {   
  for (std::shared_ptr<PyramidBox> pyramidBox : pyramidBoxes_) {
    if (pyramidBox->getHasClipped()) {
      return true;
    }
  }
  return false;
};*/

/*void OrderBox::timerCallback() {
  if (auto meter = levelMeterCalculator_.lock()) {
    meter->decayIfNeeded(60);
    for (int i = 0; i < channels_.size(); ++i) {
      values_.at(i) = meter->getLevel(channels_.at(i));
    }
    repaint();
  }
}*/
