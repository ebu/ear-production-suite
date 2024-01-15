#include "value_box_data_file.hpp"

namespace ear {
namespace plugin {
namespace ui {

ValueBoxDataFile::ValueBoxDataFile() { 
  label_.setFont(EarFontsSingleton::instance().Label);
  label_.setColour(Label::textColourId, EarColours::Label);
  label_.setText("Filter Set:", juce::NotificationType::dontSendNotification);
  label_.setJustificationType(Justification::left);
  addAndMakeVisible(label_);

  comboBox_ = std::make_shared<EarComboBox>();
  comboBox_->setDefaultText("Select filter set");
  comboBox_->setCanClearSelection(false);
  addAndMakeVisible(comboBox_.get());
}

void ValueBoxDataFile::paint(Graphics& g) {
  g.fillAll(EarColours::Area01dp);
}

void ValueBoxDataFile::resized() { 
  auto area = getLocalBounds();
  auto vPad = (area.getHeight() - rowHeight_) / 2;
  if (vPad < 0) vPad = 0;
  area.reduce(margin_, vPad);
  label_.setBounds(area.removeFromLeft(labelWidth_));
  comboBox_->setBounds(area);
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
