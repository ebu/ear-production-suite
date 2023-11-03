#pragma once

#include "JuceHeader.h"

#include <speaker_setups.hpp>
#include <helper/common_definition_helper.h>

#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class SpeakerLayer : public Component {
 public:
  SpeakerLayer() {
    setColour(backgroundColourId, EarColours::Transparent);
    setColour(trackColourId, EarColours::SliderTrack);
    setColour(highlightColourId, EarColours::Item04);
    setColour(labelColourId, EarColours::Text.withAlpha(Emphasis::medium));
  }

  ~SpeakerLayer() {}

  void setLayer(Layer layer) {
    layer_ = layer;
    repaint();
  }

  void clearSpeakerSetup() {
    pfData.reset();
    repaint();
  }
  void setSpeakerSetupPackFormat(int pfId) {
    pfData = AdmCommonDefinitionHelper::getSingleton()->getPackFormatData(1, pfId);
    repaint();
  }

  void paint(Graphics &g) override {
    g.fillAll(findColour(backgroundColourId));

    centre_ = juce::Point<float>{getWidth() / 2.f, getHeight() / 2.f + 10.f};

    // circle
    g.setColour(findColour(trackColourId));
    g.drawEllipse(centre_.getX() - diameter_ / 2.f,
                  centre_.getY() - diameter_ / 2.f, diameter_, diameter_,
                  trackWidth_);

    // center cross
    Line<float> horizontalLine(
        centre_.getX() - crossSize_ / 2.f, centre_.getY(),
        centre_.getX() + crossSize_ / 2.f, centre_.getY());
    Line<float> verticalLine(centre_.getX(), centre_.getY() - crossSize_ / 2.f,
                             centre_.getX(), centre_.getY() + crossSize_ / 2.f);
    g.setColour(findColour(trackColourId));
    g.drawLine(horizontalLine, crossLinewidth_);
    g.drawLine(verticalLine, crossLinewidth_);

    if (pfData) {
        for (auto const& cfData : pfData->relatedChannelFormats) {
            Layer layer{ Layer::middle };
            if (cfData->elevation <= -30.f) {
                layer = Layer::bottom;
            } else if(cfData->elevation >= 30.f) {
                layer = Layer::upper;
            }
            if (layer == layer_) {
                std::string label{ cfData->name };
                if (cfData->speakerLabels.size() > 0) {
                    label = cfData->speakerLabels[0];
                }
                if (cfData->elevation >= 90.f) {
                    drawVoiceOfGod(g, label);
                }
                else if (cfData->isLfe) {
                    drawLfe(g, cfData->azimuth, label);
                }
                else {
                    drawSpeaker(g, cfData->azimuth, label);
                }
            }
        }
    }
  }

  void drawSpeaker(Graphics &g, float azimuth, const std::string& label) {
    const float angleRad = azimuth / 180.f * MathConstants<float>::pi;
    g.setColour(findColour(highlightColourId));
    float xPos = centre_.getX() - std::sin(angleRad) * diameter_ / 2.f;
    float yPos = centre_.getY() - std::cos(angleRad) * diameter_ / 2.f;
    g.fillEllipse(xPos - knobSize_ / 2.f, yPos - knobSize_ / 2.f, knobSize_,
                  knobSize_);
    float xPosLabel = centre_.getX() -
                      std::sin(angleRad) * (diameter_ / 2.f + labelDistance_);
    float yPosLabel = centre_.getY() -
                      std::cos(angleRad) * (diameter_ / 2.f + labelDistance_);
    juce::Rectangle<float> labelRect{xPos, yPos - labelDistance_, 100.f, 26.f};
    labelRect.translate(-50.f, -13.f);
    g.setFont(speakerLabelFont_);
    g.setColour(findColour(labelColourId));
    g.drawText(String(label), labelRect, Justification::centred);
  }

  void drawLfe(Graphics &g, float azimuth, const std::string& label) {
    float angleRad = azimuth / 180.f * MathConstants<float>::pi;
    float xPos =
        centre_.getX() - std::sin(angleRad) * (diameter_ / 2.f + lfeDistance_);
    float yPos =
        centre_.getY() - std::cos(angleRad) * (diameter_ / 2.f + lfeDistance_);
    juce::Rectangle<float> lfeRect{xPos, yPos, 26.f, 26.f};
    lfeRect.translate(-13.f, -13.f);
    g.setFont(lfeLabelFont_);
    g.setColour(findColour(labelColourId));
    g.drawText(String(label), lfeRect, Justification::centred);
    g.setColour(findColour(highlightColourId));
    g.drawRect(lfeRect);
  }

  void drawVoiceOfGod(Graphics &g, const std::string& label) {
    g.setColour(findColour(highlightColourId));
    float xPos = centre_.getX();
    float yPos = centre_.getY();
    g.fillEllipse(xPos - knobSize_ / 2.f, yPos - knobSize_ / 2.f, knobSize_,
                  knobSize_);
    juce::Rectangle<float> labelRect{xPos, yPos - labelDistance_, 100.f, 26.f};
    labelRect.translate(-50.f, -13.f);
    g.setFont(speakerLabelFont_);
    g.setColour(findColour(labelColourId));
    g.drawText(String(label), labelRect, Justification::centred);
  }

  enum ColourIds {
    backgroundColourId = 0x00010001,
    trackColourId = 0x00010002,
    highlightColourId = 0x00010003,
    labelColourId = 0x00010004,
  };

  void resized() override {}

 private:
  std::shared_ptr<AdmCommonDefinitionHelper::PackFormatData> pfData;

  Layer layer_;

  Font speakerLabelFont_ = EarFontsSingleton::instance().Description;
  Font lfeLabelFont_ = EarFontsSingleton::instance().Measures;
  const float labelDistance_ = 13.f;
  // ITU Labels
  // Font speakerLabelFont_ = EarFontsSingleton::instance().Measures;
  // const float labelDistance_ = 11.f;

  juce::Point<float> centre_;
  // TODO: Dia was 120 when boxes were smaller (before metadata removal)
  const float diameter_ = 170.f;
  const float trackWidth_ = 5.f;
  const float knobSize_ = 10.37f;
  const float crossSize_ = 17.f;
  const float crossLinewidth_ = 1.f;
  const float lfeDistance_ = 45.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpeakerLayer)
};  // namespace ui

}  // namespace ui
}  // namespace plugin
}  // namespace ear
