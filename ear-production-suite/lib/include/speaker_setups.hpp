#pragma once

#include <string>
#include <vector>
#include <algorithm>

namespace ear {
namespace plugin {

enum Layer { upper, middle, bottom };

struct Speaker {
  std::string spLabel;
  std::string label;
  float azimuth;
  float elevation;
  Layer layer;
  bool isLfe;
  bool isVoG;
};

struct SpeakerSetup {
  SpeakerSetup() = default;
  SpeakerSetup(std::string initName, std::string initCommonName,
    std::string initSpecification, std::string initDisplayName,
    std::vector<Speaker> initSpeakers)
      : name(initName), commonName(initCommonName), specification(initSpecification), 
    displayName(initDisplayName), speakers(initSpeakers) {}
  std::string name;
  std::string commonName;
  std::vector<Speaker> speakers;
  std::string specification;
  std::string displayName;
};

static const std::vector<SpeakerSetup> SPEAKER_SETUPS = {
        SpeakerSetup("0+1+0", "mono", "BS.775", "mono",
                     std::vector<Speaker>{Speaker{"M", "M+000", 0.f, 0.f,
                                                  Layer::middle, false, false}}),
        SpeakerSetup(
            "0+2+0", "stereo", "BS.2051", "stereo",
            std::vector<Speaker>{
                Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false}}),
      SpeakerSetup(
                "0+3+0", "3.0", "BS.775", "3.0",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false}}),
      SpeakerSetup(
                "0+4+0", "4.0", "BS.775", "4.0",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"BC", "M+180", 180.f, 0.f, Layer::middle, false, false}}),
      SpeakerSetup(
                "0+5+0", "5.1", "BS.2051", "5.1",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rs", "M-110", -110.f, 0.f, Layer::middle, false, false}}),
      SpeakerSetup(
               "0+5+0", "5.0", "BS.2051", "5.0",
               std::vector<Speaker>{
                   Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                   Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                   Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                   Speaker{"Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
                   Speaker{"Rs", "M-110", -110.f, 0.f, Layer::middle, false, false}}),
      SpeakerSetup(
                "0+6+0", "6.1", "BS.2051", "6.1",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
                    Speaker{"BC", "M+180", 180.f, 0.f, Layer::middle, false, false}}),
      SpeakerSetup(
                "0+7+0", "7.1front", "BS.2094", "7.1 Front",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
                    Speaker{"FLM", "M+045", 45.f, 0.f, Layer::middle, false, false},
                    Speaker{"FRM", "M-045", -45.f, 0.f, Layer::middle, false, false}}),
      SpeakerSetup(
                "0+7+0", "7.1back", "BS.2051", "7.1 Back",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"Lss", "M+090", 90.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rss", "M-090", -90.f, 0.f, Layer::middle, false, false},
                    Speaker{"Lrs", "M+135", 135.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rrs", "M-135", -135.f, 0.f, Layer::middle, false, false}}),
      SpeakerSetup(
                "2+5+0", "5.1+2H", "BS.2051", "5.1+2H",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
                    Speaker{"Ltf", "U+030", 30.f, 45.f, Layer::upper, false, false},
                    Speaker{"Rtf", "U-030", -30.f, 45.f, Layer::upper, false, false}}),
      SpeakerSetup(
                "0+7+0", "7.1side 5.1+sc", "BS.2094", "7.1 Side",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"Lss", "M+090", 90.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rss", "M-090", -90.f, 0.f, Layer::middle, false, false},
                    Speaker{"Lsc", "M+SC", 15.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rsc", "M-SC", -15.f, 0.f, Layer::middle, false, false}}),
      SpeakerSetup(
                "2+5+0", "7.1topside 5.1.2", "BS.2094", "5.1.2",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"Lss", "M+090", 90.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rss", "M-090", -90.f, 0.f, Layer::middle, false, false},
                    Speaker{ "TpSiL", "U+090", 90.f, 45.f, Layer::upper, false, false },
                    Speaker{"TpSiR", "U-090", -90.f, 45.f, Layer::upper, false, false}}),
      SpeakerSetup(
                "2+7+0", "9.1screen 5.1.2+sc 5.1.2", "BS.2094", "9.1 Screen",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"Lss", "M+090", 90.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rss", "M-090", -90.f, 0.f, Layer::middle, false, false},
                    Speaker{ "TpSiL", "U+090", 90.f, 45.f, Layer::upper, false, false },
                    Speaker{"TpSiR", "U-090", -90.f, 45.f, Layer::upper, false, false},
                    Speaker{ "Lsc", "M+SC", 15.f, 0.f, Layer::middle, false, false },
                    Speaker{ "Rsc", "M-SC", -15.f, 0.f, Layer::middle, false, false }}),
      SpeakerSetup(
                "2+7+0", "9.1 7.1.2", "BS.2094", "9.1",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{ "SiR", "M-090", -90.f, 0.f, Layer::middle, false, false }, 
                    Speaker{ "TpC", "T+000", 0.f, 90.f, Layer::upper, false, true }, 
                    Speaker{ "LB", "M+135", 135.f, 0.f, Layer::middle, false, false }, 
                    Speaker{ "Rrs", "M-135", -135.f, 0.f, Layer::middle, false, false }, 
                    Speaker{ "TpSiL", "U+090", 90.f, 45.f, Layer::upper, false, false },
                    Speaker{ "TpSiR", "U-090", -90.f, 45.f, Layer::upper, false, false }}),
      SpeakerSetup(
                "4+5+0", "9.1 5.1+4H", "BS.2051", "5.1+4H",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
                    Speaker{"Ltf", "U+030", 30.f, 45.f, Layer::upper, false, false},
                    Speaker{"Rtf", "U-030", -30.f, 45.f, Layer::upper, false, false},
                    Speaker{"Ltr", "U+110", 110.f, 45.f, Layer::upper, false, false},
                    Speaker{"Rtr", "U-110", -110.f, 45.f, Layer::upper, false, false}}),
      SpeakerSetup(
                "4+5+1", "10.1", "BS.2051", "10.1",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
                    Speaker{"Ltf", "U+030", 30.f, 45.f, Layer::upper, false, false},
                    Speaker{"Rtf", "U-030", -30.f, 45.f, Layer::upper, false, false},
                    Speaker{"Ltr", "U+110", 110.f, 45.f, Layer::upper, false, false},
                    Speaker{"Rtr", "U-110", -110.f, 45.f, Layer::upper, false, false},
                    Speaker{"Cbf", "B+000", 0.f, -30.f, Layer::bottom, false, false}}),
      SpeakerSetup(
                "3+7+0", "10.2 7.2+3H", "BS.2051", "7.2+3H",
                std::vector<Speaker>{
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"LH", "U+045", 45.f, 45.f, Layer::upper, false, false},
                    Speaker{"RH", "U-045", -45.f, 45.f, Layer::upper, false, false},
                    Speaker{"LS", "M+090", 90.f, 0.f, Layer::middle, false, false},
                    Speaker{"RS", "M-090", -90.f, 0.f, Layer::middle, false, false},
                    Speaker{"LB", "M+135", 135.f, 0.f, Layer::middle, false, false},
                    Speaker{"RB", "M-135", -135.f, 0.f, Layer::middle, false, false},
                    Speaker{"CH", "UH+180", 180.f, 45.f, Layer::upper, false, false},
                    Speaker{"LFE1", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"LFE2", "LFE2", -45.f, -30.f, Layer::bottom, true, false}}),
      SpeakerSetup(
                "4+7+0", "11.1 5.1.4+sc", "BS.2094", "11.1",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
                    Speaker{"Ltf", "U+030", 30.f, 45.f, Layer::upper, false, false},
                    Speaker{"Rtf", "U-030", -30.f, 45.f, Layer::upper, false, false},
                    Speaker{"Ltr", "U+110", 110.f, 45.f, Layer::upper, false, false},
                    Speaker{"Rtr", "U-110", -110.f, 45.f, Layer::upper, false, false},
                    Speaker{ "Lsc", "M+SC", 15.f, 0.f, Layer::middle, false, false },
                    Speaker{ "Rsc", "M-SC", -15.f, 0.f, Layer::middle, false, false }}),
      SpeakerSetup(
                "4+7+0", "11.1 7.1+4H", "BS.2051", "7.1+4H",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"Lss", "M+090", 90.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rss", "M-090", -90.f, 0.f, Layer::middle, false, false},
                    Speaker{"Lrs", "M+135", 135.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rrs", "M-135", -135.f, 0.f, Layer::middle, false, false},
                    Speaker{"Ltf", "U+045", 45.f, 45.f, Layer::upper, false, false},
                    Speaker{"Rtf", "U-045", -45.f, 45.f, Layer::upper, false, false},
                    Speaker{"Ltb", "U+135", 135.f, 45.f, Layer::upper, false, false},
                    Speaker{"Rtb", "U-135", -135.f, 45.f, Layer::upper, false,false}}),
      SpeakerSetup(
                "4+9+0", "13.1 9.1+4H", "BS.2051", "9.1+4H",
                std::vector<Speaker>{
                    Speaker{"L", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"R", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"C", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"Lss", "M+090", 90.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rss", "M-090", -90.f, 0.f, Layer::middle, false, false},
                    Speaker{"Lrs", "M+135", 135.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rrs", "M-135", -135.f, 0.f, Layer::middle, false, false},
                    Speaker{"Ltf", "U+045", 45.f, 45.f, Layer::upper, false, false},
                    Speaker{"Rtf", "U-045", -45.f, 45.f, Layer::upper, false, false},
                    Speaker{"Ltb", "U+135", 135.f, 45.f, Layer::upper, false, false},
                    Speaker{"Rtb", "U-135", -135.f, 45.f, Layer::upper, false, false},
                    Speaker{"Lsc", "M+SC", 15.f, 0.f, Layer::middle, false, false},
                    Speaker{"Rsc", "M-SC", -15.f, 0.f, Layer::middle, false, false}}),
      SpeakerSetup(
                "9+10+3", "22.2", "BS.2051", "22.2",
                std::vector<Speaker>{
                    Speaker{"FL", "M+060", 60.f, 0.f, Layer::middle, false, false},
                    Speaker{"FR", "M-060", -60.f, 0.f, Layer::middle, false, false},
                    Speaker{"FC", "M+000", 0.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE1", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"BL", "M+135", 135.f, 0.f, Layer::middle, false, false},
                    Speaker{"BR", "M-135", -135.f, 0.f, Layer::middle, false, false},
                    Speaker{"FLc", "M+030", 30.f, 0.f, Layer::middle, false, false},
                    Speaker{"FRc", "M-030", -30.f, 0.f, Layer::middle, false, false},
                    Speaker{"BC", "M+180", 180.f, 0.f, Layer::middle, false, false},
                    Speaker{"LFE2", "LFE2", -45.f, -30.f, Layer::bottom, true, false},
                    Speaker{"SiL", "M+090", 90.f, 0.f, Layer::middle, false, false},
                    Speaker{"SiR", "M-090", -90.f, 0.f, Layer::middle, false, false},
                    Speaker{"TpFL", "U+045", 45.f, 45.f, Layer::upper, false, false},
                    Speaker{"TpFR", "U-045", -45.f, 45.f, Layer::upper, false, false},
                    Speaker{"TpFC", "U+000", 0.f, 45.f, Layer::upper, false, false},
                    Speaker{"TpC", "T+000", 0.f, 90.f, Layer::upper, false, true},
                    Speaker{"TpBL", "U+135", 135.f, 45.f, Layer::upper, false, false},
                    Speaker{"TpBR", "U-135", -135.f, 45.f, Layer::upper, false, false},
                    Speaker{"TpSiL", "U+090", 90.f, 45.f, Layer::upper, false, false},
                    Speaker{"TpSiR", "U-090", -90.f, 45.f, Layer::upper, false, false},
                    Speaker{"TpBC", "U+180", 180.f, 45.f, Layer::upper, false, false},
                    Speaker{"BtFC", "B+000", 0.f, -30.f, Layer::bottom, false, false},
                    Speaker{"BtFL", "B+045", 45.f, -30.f, Layer::bottom, false, false},
                    Speaker{"BtFR", "B-045", -45.f, -30.f, Layer::bottom, false,false}}),
      SpeakerSetup(
                "9+9+0", "Auro-3D", "BS.2094", "Auro-3D",
                std::vector<Speaker>{
                    Speaker{ "L", "M+030", 30.f, 0.f, Layer::middle, false, false },
                    Speaker{ "R", "M-030", -30.f, 0.f, Layer::middle, false, false },
                    Speaker{ "C", "M+000", 0.f, 0.f, Layer::middle, false, false },
                    Speaker{ "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false },
                    Speaker{ "Ls", "M+110", 110.f, 0.f, Layer::middle, false, false },
                    Speaker{ "Rs", "M-110", -110.f, 0.f, Layer::middle, false, false },
                    Speaker{ "SiL", "M+090", 90.f, 0.f, Layer::middle, false, false },
                    Speaker{ "SiR", "M-090", -90.f, 0.f, Layer::middle, false, false },
                    Speaker{ "BL", "M+135", 135.f, 0.f, Layer::middle, false, false }, // This speaker is marked as diffuse 
                    Speaker{"BR", "M-135", -135.f, 0.f, Layer::middle, false, false},  // This speaker is marked as diffuse 
                    Speaker{ "Ltf", "U+030", 30.f, 45.f, Layer::upper, false, false }, 
                    Speaker{ "Rtf", "U-030", -30.f, 45.f, Layer::upper, false, false }, 
                    Speaker{ "TpFC", "U+000", 0.f, 45.f, Layer::upper, false, false }, 
                    Speaker{ "Ltr", "U+110", 110.f, 45.f, Layer::upper, false, false },
                    Speaker{"Rtr", "U-110", -110.f, 45.f, Layer::upper, false, false},
                    Speaker{ "TpSiL", "U+090", 90.f, 45.f, Layer::upper, false, false },
                    Speaker{ "TpSiR", "U-090", -90.f, 45.f, Layer::upper, false, false }, 
                    Speaker{ "Ltb", "U+135", 135.f, 45.f, Layer::upper, false, false },
                    Speaker{ "Rtb", "U-135", -135.f, 45.f, Layer::upper, false,false }}),
        };


inline SpeakerSetup speakerSetupByName(const std::string& name) {
  auto it = std::find_if(
      SPEAKER_SETUPS.begin(), SPEAKER_SETUPS.end(),
      [name](auto speakerSetup) { return speakerSetup.name == name; });
  if (it != SPEAKER_SETUPS.end()) {
    return *it;
  } else {
    return SpeakerSetup{};
  }
}

inline SpeakerSetup speakerSetupByCommonName(const std::string& commonName) {
  auto it = std::find_if(SPEAKER_SETUPS.begin(), SPEAKER_SETUPS.end(),
                         [commonName](auto speakerSetup) {
                           return speakerSetup.commonName == commonName;
                         });
  if (it != SPEAKER_SETUPS.end()) {
    return *it;
  } else {
    return SpeakerSetup{};
  }
}

inline SpeakerSetup speakerSetupByIndex(int index) {
  if (0 <= index && index < SPEAKER_SETUPS.size()) {
    return SPEAKER_SETUPS.at(index);
  } else {
    return SpeakerSetup{};
  }
}

inline SpeakerSetup speakerSetupByNameAndSpecification(const std::string& name, const std::string& specification) {
  auto it = std::find_if(
    SPEAKER_SETUPS.begin(), SPEAKER_SETUPS.end(),
    [name, specification](auto speakerSetup) { return (speakerSetup.name == name) && (speakerSetup.specification == specification); });
  if (it != SPEAKER_SETUPS.end()) {
    return *it;
  }
  else {
    return SpeakerSetup{};
  }
}

}  // namespace plugin
}  // namespace ear
