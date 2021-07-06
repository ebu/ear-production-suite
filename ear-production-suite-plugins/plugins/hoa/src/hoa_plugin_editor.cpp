#include "hoa_plugin_editor.hpp"

// TODO - remove unrequired components once UI dev complete
#include "components/look_and_feel/colours.hpp"
#include "components/look_and_feel/fonts.hpp"
#include "components/look_and_feel/shadows.hpp"
#include "hoa_plugin_processor.hpp"
#include "hoa_frontend_connector.hpp"

using namespace ear::plugin::ui;

HoaAudioProcessorEditor::HoaAudioProcessorEditor(//this is the constructor
    HoaAudioProcessor* p)
    : AudioProcessorEditor(p),
      p_(p),
      content_(std::make_unique<HoaComponent>(p)),
      viewport_(std::make_unique<Viewport>()) {
  p->getFrontendConnector()->setStatusBarLabel(content_->statusBarLabel);


  //ME here we set what is in the main box, these are all grouped together in a value_box_main object
  p->getFrontendConnector()->setNameTextEditor(
      content_->mainValueBox->getNameTextEditor());
  p->getFrontendConnector()->setColourComboBox(
      content_->mainValueBox->getColourComboBox());
  p->getFrontendConnector()->setRoutingComboBox(
      content_->mainValueBox->getRoutingComboBox());
  //ME add: add likewise option for chooosing common definition
  p->getFrontendConnector()->setCommonDefinitionComboBox(//here we are saying what we want to display. alt_1st, when editor called
      content_->mainValueBox->getCommonDefinitionComboBox());//arguments is pointer to a combo box
  /* Old DS Code
  // TODO - we only need to connect to UI components present in HOA plugin.
  Rewrite this.
  p->getFrontendConnector()->setSpeakerSetupsComboBox(
      content_->mainValueBox->getSpeakerSetupsComboBox());
  p->getFrontendConnector()->setUpperLayerValueBox(
      content_->upperLayerValueBox);
  p->getFrontendConnector()->setMiddleLayerValueBox(
      content_->middleLayerValueBox);
  p->getFrontendConnector()->setBottomLayerValueBox(
      content_->bottomLayerValueBox);
  p->getFrontendConnector()->setChannelGainsValueBox(
      content_->channelGainValueBox);
  */
  viewport_->setViewedComponent(content_.get(), false);
  viewport_->setScrollBarsShown(true, true);
  viewport_->getHorizontalScrollBar().setColour(ScrollBar::thumbColourId,
                                                EarColours::Primary);
  viewport_->getVerticalScrollBar().setColour(ScrollBar::thumbColourId,
                                              EarColours::Primary);
  addAndMakeVisible(viewport_.get());

  setResizable(true, false);
  setResizeLimits(0, 0, 726, 930);
  setSize(726, 930);
  
  //ME add
  //auto packFormats = p->admCommonDefinitions->getElements<adm::AudioPackFormat>();
  auto elementRelationships =p->admCommonDefinitions.getElementRelationships();
  //int layoutSize{0};
  //auto elementRelationships = p->admCommonDefinitions.populateElementRelationshipsFor(adm::TypeDefinition::HOA);
  for (auto const& [id, tdData] : elementRelationships) {
    if (id == 4) {
      auto packData = tdData->relatedPackFormats;
      //int size{static_cast<int>(packData.size())};
      //p->setNumHoaTypes(size);
      for (auto const& pfData : packData) {
        content_->mainValueBox->getCommonDefinitionComboBox()->addTextEntry(
            pfData->niceName, pfData->id);
        //layoutSize = pfData->relatedChannelFormats.size();
        //content_->mainValueBox->getCommonDefinitionComboBox()->setSelectedId
      }
    }
  }
  /* if (auto routingComboBoxLocked = routingComboBox_.lock()) {

    if (0 <= cachedCommonDefinition_ && cachedCommonDefinition_ < 0xFFFF) {
      layoutSize = SPEAKER_SETUPS.at(cachedCommonDefinition_).speakers.size();
    } else {
      layoutSize = SpeakerSetup{}.speakers.size();
    }

    routingComboBoxLocked->clearEntries();
    auto layoutSizeFixed = layoutSize != 0 ? layoutSize - 1 : layoutSize;
    for (int i = 1; i + layoutSizeFixed <= 64; ++i) {
      routingComboBoxLocked->addTextEntry(String(i) + String::fromUTF8("–") +
                                          String(i + layoutSizeFixed));
    }
    routingComboBoxLocked->selectEntry(cachedRouting_, sendNotification);
  }*/
  
 // addAndMakeVisible(packFormatSelector);


  //ME emd
}


HoaAudioProcessorEditor::~HoaAudioProcessorEditor() {}

void HoaAudioProcessorEditor::paint(Graphics& g) {}

void HoaAudioProcessorEditor::resized() {
  viewport_->setBounds(getLocalBounds());
  content_->setBounds(0, 0, 726, 930);
}
