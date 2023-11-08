#pragma once

#include "speaker_layer.hpp"

namespace ear {
namespace plugin {
namespace ui {

SpeakerLayer::SpeakerLayer() {
    setColour(backgroundColourId, EarColours::Transparent);
    setColour(trackColourId, EarColours::SliderTrack);
    setColour(highlightColourId, EarColours::Item04);
    setColour(labelColourId, EarColours::Text.withAlpha(Emphasis::medium));
}

SpeakerLayer::~SpeakerLayer() {}

void SpeakerLayer::setLayer(Layer layer) {
    layer_ = layer;
    repaint();
}

void SpeakerLayer::clearSpeakerSetup() {
    pfData.reset();
    repaint();
}

void SpeakerLayer::setSpeakerSetupPackFormat(int pfId) {
    pfData = AdmPresetDefinitionsHelper::getSingleton()->getPackFormatData(1, pfId);
    repaint();
}

void SpeakerLayer::resized() {}

void SpeakerLayer::paint(Graphics& g) {
    g.fillAll(findColour(backgroundColourId));

    centre_ = juce::Point<float>{ getWidth() / 2.f, getHeight() / 2.f };

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

    speakerPlacement_.reset();
    if (pfData) {
        for (auto const& cfData : pfData->relatedChannelFormats) {
            Layer layer{ Layer::middle };
            if (cfData->elevation <= -30.f) {
                layer = Layer::bottom;
            }
            else if (cfData->elevation >= 30.f) {
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
                    speakerPlacement_.addSpeaker(label, cfData->azimuth);
                }
            }
        }
        // Draw speakers
        speakerPlacement_.drawSpeakers(g, this);
    }
}

void SpeakerLayer::drawSpeaker(Graphics& g, float azimuth, float labelAzimuth, const std::string& label, bool outer) {
    const float angleRad = azimuth / 180.f * MathConstants<float>::pi;
    const float angleRadLabel = labelAzimuth / 180.f * MathConstants<float>::pi;
    g.setColour(findColour(highlightColourId));

    auto radius = diameter_ / 2.f;

    float xPos = centre_.getX() - std::sin(angleRad) * radius;
    float yPos = centre_.getY() - std::cos(angleRad) * radius;
    g.fillEllipse(xPos - knobSize_ / 2.f, yPos - knobSize_ / 2.f, knobSize_, knobSize_);

    float labelWidth = speakerLabelFont_.getStringWidthFloat(String(label));
    const float labelHeight = speakerLabelFont_.getHeight();
    float labelRadius = distanceToEdge(labelWidth, labelHeight, labelAzimuth);
    float radOffset = outer ? (labelRadius + knobSize_) : -(labelRadius + knobSize_);

    xPos = centre_.getX() - std::sin(angleRadLabel) * (radius + radOffset);
    yPos = centre_.getY() - std::cos(angleRadLabel) * (radius + radOffset);
    juce::Rectangle<float> labelRect{ xPos, yPos, labelWidth, labelHeight };
    labelRect.setCentre(xPos, yPos); // we point to intended centre
    g.setFont(speakerLabelFont_);
    g.setColour(findColour(labelColourId));
    g.drawText(String(label), labelRect, Justification::centred);
}

void SpeakerLayer::drawLfe(Graphics& g, float azimuth, const std::string& label) {
    float angleRad = azimuth / 180.f * MathConstants<float>::pi;
    float xPos =
        centre_.getX() - std::sin(angleRad) * (diameter_ / 2.f + lfeDistance_);
    float yPos =
        centre_.getY() - std::cos(angleRad) * (diameter_ / 2.f + lfeDistance_);
    juce::Rectangle<float> lfeRect{ xPos, yPos, 26.f, 26.f };
    lfeRect.translate(-13.f, -13.f);
    g.setFont(lfeLabelFont_);
    g.setColour(findColour(labelColourId));
    g.drawText(String(label), lfeRect, Justification::centred);
    g.setColour(findColour(highlightColourId));
    g.drawRect(lfeRect);
}

void SpeakerLayer::drawVoiceOfGod(Graphics& g, const std::string& label) {
    g.setColour(findColour(highlightColourId));
    float xPos = centre_.getX();
    float yPos = centre_.getY();
    g.fillEllipse(xPos - knobSize_ / 2.f, yPos - knobSize_ / 2.f, knobSize_,
        knobSize_);
    juce::Rectangle<float> labelRect{ xPos, yPos - labelDistance_, 100.f, 26.f };
    labelRect.translate(-50.f, -13.f);
    g.setFont(speakerLabelFont_);
    g.setColour(findColour(labelColourId));
    g.drawText(String(label), labelRect, Justification::centred);
}

float SpeakerLayer::distanceToEdge(float width, float height, float angleDegrees) {
    // Convert angle from degrees to radians
    float angleRadians = angleDegrees * M_PI / 180.0f;
    float a = fabs((height / 2.0f) / cos(angleRadians));
    float b = fabs((width / 2.0f) / sin(angleRadians));
    return std::min(a, b);
}

SpeakerLabelPlacement::SpeakerLabelPlacement() {};
SpeakerLabelPlacement::~SpeakerLabelPlacement() {};

void SpeakerLabelPlacement::reset() {
    drawableSpeakers.clear();
}

void SpeakerLabelPlacement::addSpeaker(const std::string& label, float azimuth) {
    SpUi spUi{ label, azimuth, azimuth, false };
    while (spUi.spAz >= 360.0) spUi.spAz -= 360.0;
    while (spUi.spAz < 0.0) spUi.spAz += 360.0;
    spUi.labAz = spUi.spAz;
    drawableSpeakers.push_back(spUi);
}

void SpeakerLabelPlacement::drawSpeakers(Graphics& g, SpeakerLayer* layer) {
    sortSpeakers();
    int lastIndex = static_cast<int>(drawableSpeakers.size()) - 1;
    // Process speaker positions. Ignore 0 deg and 180 - we always want to leave those as they are.
    /// Check from 0 to 180 for spacing
    for (int i = 0; i <= lastIndex; ++i) {
        const float thisAz = drawableSpeakers[i].spAz;
        if (thisAz == 0.f) continue;
        if (thisAz >= 180.f) break;
        processSpeaker(i);
    }
    /// Check from 360 to 180 for spacing
    for (int i = lastIndex; i >= 0; --i) {
        const float thisAz = drawableSpeakers[i].spAz;
        if (thisAz == 0.f) continue;
        if (thisAz <= 180.f) break;
        processSpeaker(i);
    }
    // Draw them
    for (auto const& spUi : drawableSpeakers) {
        layer->drawSpeaker(g, spUi.spAz, spUi.labAz, spUi.label, !spUi.inner);
    }
}

void SpeakerLabelPlacement::sortSpeakers()
{
    std::sort(drawableSpeakers.begin(), drawableSpeakers.end(),
        [](const SpUi& a, const SpUi& b) {
            return a.spAz < b.spAz;
        }
    );
}

void SpeakerLabelPlacement::processSpeaker(int spkIndex)
{
    if (tooCloseToPrev(spkIndex)) {
        drawableSpeakers[spkIndex].inner = true;
        drawableSpeakers[spkIndex].labAz += bigAzNudge;
        getPrevSpk(spkIndex)->labAz -= littleAzNudge;
    }

    if (tooCloseToNext(spkIndex)) {
        drawableSpeakers[spkIndex].inner = true;
        drawableSpeakers[spkIndex].labAz -= bigAzNudge;
        getNextSpk(spkIndex)->labAz += littleAzNudge;
    }
}

bool SpeakerLabelPlacement::tooCloseToNext(int spkIndex)
{
    auto spk = getNextSpk(spkIndex);
    return tooClose(&drawableSpeakers[spkIndex], spk);
}

bool SpeakerLabelPlacement::tooCloseToPrev(int spkIndex)
{
    auto spk = getPrevSpk(spkIndex);
    return tooClose(&drawableSpeakers[spkIndex], spk);
}

bool SpeakerLabelPlacement::tooClose(const SpUi* spkA, const SpUi* spkB)
{
    // Check there actually is a neighbour
    if (!spkA || !spkB) return false;
    // Check that the neighbour is on a separate ring
    if (spkA->inner != spkB->inner) return false;
    // Check angular distance between speakers
    return angularDistance(*spkA, *spkB) < minAzDist;
}

float SpeakerLabelPlacement::angularDistance(const SpUi& spkA, const SpUi& spkB)
{
    float mid = std::max(spkA.spAz, spkB.spAz);
    float lower = std::min(spkA.spAz, spkB.spAz);
    float upper = lower + 360.0;
    return std::min(mid - lower, upper - mid);
}

SpeakerLabelPlacement::SpUi* SpeakerLabelPlacement::getNextSpk(int fromIndex)
{
    if (drawableSpeakers.size() < 2) return nullptr;
    int index = fromIndex + 1;
    const int lastIndex = static_cast<int>(drawableSpeakers.size()) - 1;
    if (index > lastIndex)
        index = 0;
    return &drawableSpeakers[index];
}

SpeakerLabelPlacement::SpUi* SpeakerLabelPlacement::getPrevSpk(int fromIndex)
{
    if (drawableSpeakers.size() < 2) return nullptr;
    int index = fromIndex - 1;
    const int lastIndex = static_cast<int>(drawableSpeakers.size()) - 1;
    if (index < 0)
        index = lastIndex;
    return &drawableSpeakers[index];
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
