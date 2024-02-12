#pragma once

#include "JuceHeader.h"

#include <speaker_setups.hpp>
#include <helper/adm_preset_definitions_helper.h>

#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"

namespace ear {
namespace plugin {
namespace ui {

class SpeakerLayer;

class SpeakerLabelPlacement {
public:
    SpeakerLabelPlacement();
    ~SpeakerLabelPlacement();

    void reset();
    void addSpeaker(const std::string& label, float azimuth);
    void drawSpeakers(Graphics& g, SpeakerLayer* layer);

private:
    struct SpUi {
        std::string label;
        float spAz;
        float labAz;
        bool inner;
    };
    std::vector<SpUi> drawableSpeakers;
    void sortSpeakers();
    void processSpeaker(int spkIndex);
    bool tooClose(const SpUi& spkA, const SpUi& spkB);
    float angularDistance(const SpUi& spkA, const SpUi& spkB);
    int getNextSpkIndex(int fromIndex);
    int getPrevSpkIndex(int fromIndex);
    const float minAzDist = 15.f;
    const float bigAzNudge = 10.f;
    const float littleAzNudge = 5.f;
};

class SpeakerLayer : public Component {
public:
    SpeakerLayer();
    ~SpeakerLayer();

    void setLayer(Layer layer);
    void clearSpeakerSetup();
    void setSpeakerSetupPackFormat(int pfId);

    void resized() override;
    void paint(Graphics& g) override;

    void drawSpeaker(Graphics& g, float azimuth, float labelAzimuth, const std::string& label, bool inner);
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
    std::shared_ptr<AdmPresetDefinitionsHelper::PackFormatData const> pfData;

    Layer layer_;
    SpeakerLabelPlacement speakerPlacement_{};

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
