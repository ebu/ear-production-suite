#include "order_box.hpp"

#include "pyramid_box.hpp"
#include "value_box_order_display.hpp"
#include "hoa_plugin_processor.hpp"
#include "components/level_meter.hpp"
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"

namespace {
int numChannelsUpToOrder(int order) { return pow(order + 1, 2); }
int numChannelsOnlyInOrder(int order) { return order < 0 ? 0 : order * 2 + 1; }
}  // namespace

using namespace ear::plugin::ui;

OrderBox::OrderBox(HoaAudioProcessor* p,
                   ValueBoxOrderDisplay* valueBoxOrderDisplay, String name,
                   int rowOrder, int hoaOrder)
    : levelMeter_(std::make_unique<LevelMeter>()),
      orderLabel_(std::make_unique<Label>()),
      rowOrder_(rowOrder),
      hoaOrder_(hoaOrder),
      p_(p),
      valueBoxOrderDisplay_(valueBoxOrderDisplay) {
  levelMeter_->setName("LevelMeter (OrderBox::levelMeter_)");
  orderLabel_->setName("Label (OrderBox::orderLabel_)");

  levelMeter_->setOrientation(LevelMeter::horizontal);
  levelMeter_->enableAverage(true);
  addAndMakeVisible(levelMeter_.get());

  orderLabel_->setText(name, dontSendNotification);
  orderLabel_->setFont(EarFonts::Items);
  orderLabel_->setColour(Label::textColourId, EarColours::Label);
  orderLabel_->setJustificationType(Justification::centred);
  addAndMakeVisible(orderLabel_.get());

  addPyramidBoxesToOrderBox();
}
OrderBox::~OrderBox() {}
void OrderBox::paint(Graphics& g) { g.fillAll(EarColours::Area01dp); }

void OrderBox::resized() {
  auto area = getLocalBounds();

  orderLabel_->setBounds(area.removeFromLeft(40));

  area.reduce(5, 5);

  levelMeter_->setBounds(area.removeFromLeft(250));

  auto areaWidth = area.getWidth();

  int pyramidBoxEdgeSize(area.getHeight());
  int numberOfBoxPartitions = 14;
  // int numberOfBoxPartitions = (hoaOrder_ * 2) + 2;//this row makes the gaps between the pyramid boxes vary
  int partitionNumber = (numberOfBoxPartitions / 2) - rowOrder_;

  for (auto pyramidBox : pyramidBoxes_) {
    auto removeFromLeft = (partitionNumber * areaWidth) / numberOfBoxPartitions;
    auto pyramidBoxArea = area.withTrimmedLeft(removeFromLeft);
    pyramidBox->setBounds(pyramidBoxArea.removeFromLeft(pyramidBoxEdgeSize));
    partitionNumber++;
  }
}

void OrderBox::addPyramidBoxesToOrderBox() {
  auto startChannel = numChannelsUpToOrder(rowOrder_ - 1);
  auto numChannels = numChannelsOnlyInOrder(rowOrder_);

  // Child comp removal probably happens anyway, but to be sure...
  for (auto& pyramidBox : pyramidBoxes_) {
    removeChildComponent(pyramidBox.get());
  }

  pyramidBoxes_.clear();
  pyramidBoxes_.reserve(numChannels);

  std::vector<int> channels;

  for (int i(0); i < numChannels; i++) {
    int channel(startChannel + i);
    channels.push_back(channel);

    std::shared_ptr<ear::plugin::ui::PyramidBox> pyramidBox =
        std::make_shared<PyramidBox>(p_->getLevelMeter(), valueBoxOrderDisplay_,
                                     channel);
    pyramidBoxes_.push_back(pyramidBox);
    addAndMakeVisible(*pyramidBox);
  }

  levelMeter_->setMeter(p_->getLevelMeter(), channels);
}
