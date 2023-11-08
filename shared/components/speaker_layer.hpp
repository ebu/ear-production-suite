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
    SpeakerLayer();
    ~SpeakerLayer();

    void setLayer(Layer layer);
    void clearSpeakerSetup();
    void setSpeakerSetupPackFormat(int pfId);

    void resized() override;
    void paint(Graphics& g) override;

    void drawSpeaker(Graphics& g, float azimuth, float labelAzimuth, const std::string& label, bool outer);
    void drawLfe(Graphics& g, float azimuth, const std::string& label);
    void drawVoiceOfGod(Graphics& g, const std::string& label);

    enum ColourIds {
        backgroundColourId = 0x00010001,
        trackColourId = 0x00010002,
        highlightColourId = 0x00010003,
        labelColourId = 0x00010004,
    };

private:
    float distanceToEdge(float width, float height, float angleDegrees);
    std::shared_ptr<AdmPresetDefinitionsHelper::PackFormatData> pfData;

    Layer layer_;

    Font speakerLabelFont_ = EarFontsSingleton::instance().Description;
    Font lfeLabelFont_ = EarFontsSingleton::instance().Measures;
    const float labelDistance_ = 13.f;

    juce::Point<float> centre_;
    const float diameter_ = 170.f;
    const float trackWidth_ = 5.f;
    const float knobSize_ = 10.f;
    const float crossSize_ = 17.f;
    const float crossLinewidth_ = 1.f;
    const float lfeDistance_ = 50.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpeakerLayer)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
