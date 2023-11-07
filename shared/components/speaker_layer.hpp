#pragma once

#include "JuceHeader.h"

#include <speaker_setups.hpp>
#include <helper/adm_preset_definitions_helper.h>

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

 private:
  float distanceToEdge(float width, float height, float angleDegrees) {
    // Convert angle from degrees to radians
    float angleRadians = angleDegrees * M_PI / 180.0f;
    float a = fabs((height / 2.0f) / cos(angleRadians));
    float b = fabs((width / 2.0f) / sin(angleRadians));
    return std::min(a, b);
  }

 public:

  void setLayer(Layer layer) {
    layer_ = layer;
    repaint();
  }

  void clearSpeakerSetup() {
    pfData.reset();
    repaint();
  }
  void setSpeakerSetupPackFormat(int pfId) {
    pfData = AdmPresetDefinitionsHelper::getSingleton()->getPackFormatData(1, pfId);
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
        struct SpUi { std::string label; float azimuth; bool outer; };
        std::vector<SpUi> drawableSpeakers;
        for (auto const& cfData : pfData->relatedChannelFormats) {
            Layer layer{ Layer::middle };
            if (cfData->elevation <= -30.f) {
                layer = Layer::bottom;
            } else if(cfData->elevation >= 30.f) {
                layer = Layer::upper;
            }
            if (layer == layer_) {
                std::string label;
                if (cfData->legacySpeakerLabel.has_value()) {
                    label = cfData->legacySpeakerLabel.value();
                }
                else if (cfData->ituLabel.has_value()) {
                    label = cfData->ituLabel.value();
                }
                if (cfData->elevation >= 90.f) {
                    drawVoiceOfGod(g, label);
                }
                else if (cfData->isLfe) {
                    drawLfe(g, cfData->azimuth, label);
                }
                else {
                    // Can't draw immediately - need to check label spacing
                    SpUi spUi{ label, cfData->azimuth, true };
                    while (spUi.azimuth >= 360.0) spUi.azimuth -= 360.0;
                    while (spUi.azimuth < 0.0) spUi.azimuth += 360.0;
                    drawableSpeakers.push_back(spUi);
                }
            }
        }

        // Check spacing between labels
        std::sort(drawableSpeakers.begin(), drawableSpeakers.end(), 
            [](const SpUi& a, const SpUi& b) {
                return a.azimuth < b.azimuth;
            }
        );
        const float minDist = 15.f;
        int drawableSpeakersSize = static_cast<int>(drawableSpeakers.size());
        /// Check from 0 to 180 for spacing
        for (int i = 1; i < drawableSpeakersSize - 1; ++i) {
            int prev = i - 1;
            int next = i + 1;
            float thisAz = drawableSpeakers[i].azimuth;
            if (thisAz == 0.f) continue;
            if (thisAz >= 180.f) break;
            float prevAz = drawableSpeakers[prev].azimuth;
            float nextAz = drawableSpeakers[next].azimuth;
            if (drawableSpeakers[prev].outer && drawableSpeakers[next].outer) {
                // Surrounded by outer labels. Should we pull this one in?
                if (thisAz - prevAz < minDist || nextAz - thisAz < minDist) {
                    drawableSpeakers[i].outer = false;
                }
            }
        }
        /// Check from 360 to 180 for spacing
        for (int i = drawableSpeakersSize - 1; i > 0; --i) {
            int prev = i + 1;
            int next = i - 1;
            float thisAz = drawableSpeakers[i].azimuth;
            if (thisAz <= 180.f) break;
            float prevAz;
            float nextAz = drawableSpeakers[next].azimuth;
            if (prev >= drawableSpeakersSize) {
                prev = 0;
                prevAz = drawableSpeakers[prev].azimuth + 360.f;
            }
            else {
                prevAz = drawableSpeakers[prev].azimuth;
            }
            if (drawableSpeakers[prev].outer && drawableSpeakers[next].outer) {
                // Surrounded by outer labels. Should we pull this one in?
                if (thisAz - nextAz < minDist || prevAz - thisAz < minDist) {
                    drawableSpeakers[i].outer = false;
                }
            }
        }
        /// Draw them
        for (auto const& spUi : drawableSpeakers) {
            drawSpeaker(g, spUi.azimuth, spUi.label, spUi.outer);
        }
    }
  }

  void drawSpeaker(Graphics &g, float azimuth, const std::string& label, bool outer) {
    const float angleRad = azimuth / 180.f * MathConstants<float>::pi;
    g.setColour(findColour(highlightColourId));

    auto radius = diameter_ / 2.f;

    float xPos = centre_.getX() - std::sin(angleRad) * radius;
    float yPos = centre_.getY() - std::cos(angleRad) * radius;
    g.fillEllipse(xPos - knobSize_ / 2.f, yPos - knobSize_ / 2.f, knobSize_,  knobSize_);

    float labelWidth = speakerLabelFont_.getStringWidthFloat(String(label));
    const float labelHeight = speakerLabelFont_.getHeight();
    float labelRadius = distanceToEdge(labelWidth, labelHeight, azimuth);
    float radOffset = outer ? (labelRadius + knobSize_) : -(labelRadius + knobSize_);

    xPos = centre_.getX() - std::sin(angleRad) * (radius + radOffset);
    yPos = centre_.getY() - std::cos(angleRad) * (radius + radOffset);
    juce::Rectangle<float> labelRect{xPos, yPos, labelWidth, labelHeight };
    labelRect.setCentre(xPos, yPos); // we point to intended centre
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
  std::shared_ptr<AdmPresetDefinitionsHelper::PackFormatData> pfData;

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
  const float knobSize_ = 10.f;
  const float crossSize_ = 17.f;
  const float crossLinewidth_ = 1.f;
  const float lfeDistance_ = 50.f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpeakerLayer)
};  // namespace ui

}  // namespace ui
}  // namespace plugin
}  // namespace ear
