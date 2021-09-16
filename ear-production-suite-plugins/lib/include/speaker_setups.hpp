#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <optional>

namespace ear {
namespace plugin {

enum Layer { upper, middle, bottom };

struct Speaker {
  std::string channelFormatId;
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
    std::string initPackFormatId,
    std::vector<Speaker> initSpeakers, int initLegacySpeakerSetupIndex = -1)
      : name(initName), commonName(initCommonName), specification(initSpecification),
    displayName(initDisplayName), packFormatId(initPackFormatId), speakers(initSpeakers) {
    if(initLegacySpeakerSetupIndex >= 0) legacySpeakerSetupIndex = initLegacySpeakerSetupIndex;
    packFormatIdValue = std::stoi(initPackFormatId.substr(7), nullptr, 16);
  }
  std::string name;
  std::string commonName;
  std::vector<Speaker> speakers;
  std::string specification;
  std::string displayName;
  std::string packFormatId;
  uint16_t packFormatIdValue;
  std::optional<int> legacySpeakerSetupIndex;
};

static const std::vector<SpeakerSetup> SPEAKER_SETUPS = {
  /* clang-format off */

  SpeakerSetup(
    "0+1+0", "mono", "BS.775", "mono", "AP_00010001",
    std::vector<Speaker>{
      Speaker{"AC_00010003", "M", "M+000", 0.f, 0.f, Layer::middle, false, false}
    }, 0),

  SpeakerSetup(
    "0+2+0", "stereo", "BS.2051", "stereo", "AP_00010002",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false}
    }, 1),

  SpeakerSetup(
    "0+3+0", "3.0", "BS.775", "3.0", "AP_0001000a",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false}
    }, 12),

  SpeakerSetup(
    "0+4+0", "4.0", "BS.775", "4.0", "AP_0001000b",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010009", "BC", "M+180", 180.f, 0.f, Layer::middle, false, false}
    }),

  SpeakerSetup(
    "0+5+0", "5.0", "BS.2051", "5.0", "AP_0001000c",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010005", "Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010006", "Rs", "M-110", -110.f, 0.f, Layer::middle, false, false}
    }, 13),

  SpeakerSetup(
    "0+5+0", "5.1", "BS.2051", "5.1", "AP_00010003",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010004", "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_00010005", "Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010006", "Rs", "M-110", -110.f, 0.f, Layer::middle, false, false}
    }, 2),

  SpeakerSetup(
    "0+6+0", "6.1", "BS.2051", "6.1", "AP_0001000d",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010004", "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_00010005", "Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010006", "Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010009", "BC", "M+180", 180.f, 0.f, Layer::middle, false, false}
    }),

  SpeakerSetup(
    "0+7+0", "7.1front", "BS.2094", "7.1 Front", "AP_0001000e",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010004", "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_00010005", "Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010006", "Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010026", "FLM", "M+045", 45.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010027", "FRM", "M-045", -45.f, 0.f, Layer::middle, false, false}
    }),

  SpeakerSetup(
    "0+7+0", "7.1back", "BS.2051", "7.1 Back", "AP_0001000f",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010004", "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_0001000a", "Lss", "M+090", 90.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001000b", "Rss", "M-090", -90.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001001c", "Lrs", "M+135", 135.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001001d", "Rrs", "M-135", -135.f, 0.f, Layer::middle, false, false}
    }, 9),

  SpeakerSetup(
    "2+5+0", "5.1+2H", "BS.2051", "5.1+2H", "AP_00010004",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010004", "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_00010005", "Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010006", "Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001000d", "Ltf", "U+030", 30.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_0001000f", "Rtf", "U-030", -30.f, 45.f, Layer::upper, false, false}
    }, 3),

  SpeakerSetup(
    "0+7+0", "7.1side 5.1+sc", "BS.2094", "7.1 Side", "AP_00010012",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010004", "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_00010005", "Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010006", "Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010024", "Lsc", "M+SC", 15.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010025", "Rsc", "M-SC", -15.f, 0.f, Layer::middle, false, false}
    }),

  SpeakerSetup(
    "2+5+0", "7.1topside 5.1.2", "BS.2094", "5.1.2", "AP_00010013",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010004", "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_00010005", "Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010006", "Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010013", "TpSiL", "U+090", 90.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010014", "TpSiR", "U-090", -90.f, 45.f, Layer::upper, false, false}
    }),

  SpeakerSetup(
    "2+7+0", "9.1screen 5.1.2+sc 5.1.2", "BS.2094", "9.1 Screen", "AP_00010014",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010004", "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_00010005", "Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010006", "Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010013", "TpSiL", "U+090", 90.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010014", "TpSiR", "U-090", -90.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010024", "Lsc", "M+SC", 15.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010025", "Rsc", "M-SC", -15.f, 0.f, Layer::middle, false, false}
    }),

  SpeakerSetup(
    "2+7+0", "9.1 7.1.2", "BS.2094", "9.1", "AP_00010016",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010004", "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_0001000a", "SiL", "M+090", 90.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001000b", "SiR", "M-090", -90.f,  0.f, Layer::middle, false, false},
      Speaker{"AC_0001001c", "LB", "M+135", 135.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001001d", "RB", "M-135", -135.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010013", "TpSiL", "U+090", 90.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010014", "TpSiR", "U-090", -90.f, 45.f, Layer::upper, false, false}
    }, 11),

  SpeakerSetup(
    "4+5+0", "9.1 5.1+4H", "BS.2051", "5.1+4H", "AP_00010005",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010004", "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_00010005", "Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010006", "Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001000d", "Ltf", "U+030", 30.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_0001000f", "Rtf", "U-030", -30.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010010", "Ltr", "U+110", 110.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010012", "Rtr", "U-110", -110.f, 45.f, Layer::upper, false, false}
    }, 4),

  SpeakerSetup(
    "4+5+1", "10.1", "BS.2051", "10.1", "AP_00010010",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010004", "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_00010005", "Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010006", "Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001000d", "Ltf", "U+030", 30.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_0001000f", "Rtf", "U-030", -30.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010010", "Ltr", "U+110", 110.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010012", "Rtr", "U-110", -110.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010015", "Cbf", "B+000", 0.f, -30.f, Layer::bottom, false, false}
    }, 5),

  SpeakerSetup(
    "3+7+0", "10.2 7.2+3H", "BS.2051", "7.2+3H", "AP_00010007",
    std::vector<Speaker>{
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010022", "LH", "U+045", 45.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010023", "RH", "U-045", -45.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_0001000a", "Lss", "M+090", 90.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001000b", "Rss", "M-090", -90.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001001c", "LB", "M+135", 135.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001001d", "RB", "M-135", -135.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010028", "CH", "UH+180", 180.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010020", "LFE1", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_00010021", "LFE2", "LFE2", -45.f, -30.f, Layer::bottom, true, false}
    }, 6),

  SpeakerSetup(
    "4+7+0", "11.1 5.1.4+sc", "BS.2094", "11.1", "AP_00010015",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010004", "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_00010005", "Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010006", "Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001000d", "Ltf", "U+030", 30.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_0001000f", "Rtf", "U-030", -30.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010010", "Ltr", "U+110", 110.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010012", "Rtr", "U-110", -110.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010024", "Lsc", "M+SC", 15.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010025", "Rsc", "M-SC", -15.f, 0.f, Layer::middle, false, false}
    }),

  SpeakerSetup(
    "4+7+0", "11.1 7.1+4H", "BS.2051", "7.1+4H", "AP_00010017",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010004", "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_0001000a", "Lss", "M+090", 90.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001000b", "Rss", "M-090", -90.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001001c", "Lrs", "M+135", 135.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001001d", "Rrs", "M-135", -135.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010022", "LH", "U+045", 45.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010023", "RH", "U-045", -45.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_0001001e", "Ltb", "U+135", 135.f, 30.f, Layer::upper, false, false},
      Speaker{"AC_0001001f", "Rtb", "U-135", -135.f, 30.f, Layer::upper, false,false}
    }, 10),

  SpeakerSetup(
    "4+9+0", "13.1 9.1+4H", "BS.2051", "9.1+4H", "AP_00010008",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010004", "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_0001000a", "Lss", "M+090", 90.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001000b", "Rss", "M-090", -90.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001001c", "Lrs", "M+135", 135.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001001d", "Rrs", "M-135", -135.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010022", "LH", "U+045", 45.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010023", "RH", "U-045", -45.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_0001001e", "Ltb", "U+135", 135.f, 30.f, Layer::upper, false, false},
      Speaker{"AC_0001001f", "Rtb", "U-135", -135.f, 30.f, Layer::upper, false,false},
      Speaker{"AC_00010024", "Lsc", "M+SC", 15.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010025", "Rsc", "M-SC", -15.f, 0.f, Layer::middle, false, false}
    }, 7),

  SpeakerSetup(
    "9+10+3", "22.2", "BS.2051", "22.2", "AP_00010009",
    std::vector<Speaker>{
      Speaker{"AC_00010018", "FL", "M+060", 60.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010019", "FR", "M-060", -60.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "FC", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010020", "LFE1", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_0001001c", "BL", "M+135", 135.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001001d", "BR", "M-135", -135.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010001", "FLc", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "FRc", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010009", "BC", "M+180", 180.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010021", "LFE2", "LFE2", -45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_0001000a", "SiL", "M+090", 90.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001000b", "SiR", "M-090", -90.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010022", "TpFL", "U+045", 45.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010023", "TpFR", "U-045", -45.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_0001000e", "TpFC", "U+000", 0.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_0001000c", "TpC", "T+000", 0.f, 90.f, Layer::upper, false, true},
      Speaker{"AC_0001001e", "TpBL", "U+135", 135.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_0001001f", "TpBR", "U-135", -135.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010013", "TpSiL", "U+090", 90.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010014", "TpSiR", "U-090", -90.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010011", "TpBC", "U+180", 180.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010015", "BtFC", "B+000", 0.f, -30.f, Layer::bottom, false, false},
      Speaker{"AC_00010016", "BtFL", "B+045", 45.f, -30.f, Layer::bottom, false, false},
      Speaker{"AC_00010017", "BtFR", "B-045", -45.f, -30.f, Layer::bottom, false,false}
    }, 8),

  SpeakerSetup(
    "9+9+0", "Auro-3D", "BS.2094", "Auro-3D", "AP_00010011",
    std::vector<Speaker>{
      Speaker{"AC_00010001", "L", "M+030", 30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010002", "R", "M-030", -30.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010003", "C", "M+000", 0.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010004", "LFE", "LFE1", 45.f, -30.f, Layer::bottom, true, false},
      Speaker{"AC_00010005", "Ls", "M+110", 110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_00010006", "Rs", "M-110", -110.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001000a", "Lss", "M+090", 90.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001000b", "Rss", "M-090", -90.f, 0.f, Layer::middle, false, false},
      Speaker{"AC_0001001a", "BL", "M+135", 135.f, 0.f, Layer::middle, false, false}, // This speaker is marked as diffuse
      Speaker{"AC_0001001b", "BR", "M-135", -135.f, 0.f, Layer::middle, false, false},  // This speaker is marked as diffuse
      Speaker{"AC_0001000d", "Ltf", "U+030", 30.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_0001000f", "Rtf", "U-030", -30.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_0001000e", "TpFC", "U+000", 0.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010010", "Ltr", "U+110", 110.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010012", "Rtr", "U-110", -110.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010013", "TpSiL", "U+090", 90.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_00010014", "TpSiR", "U-090", -90.f, 45.f, Layer::upper, false, false},
      Speaker{"AC_0001001e", "Ltb", "U+135", 135.f, 30.f, Layer::upper, false, false},
      Speaker{"AC_0001001f", "Rtb", "U-135", -135.f, 30.f, Layer::upper, false,false}
    }),

  /* clang-format on */
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

inline SpeakerSetup speakerSetupByPackFormatId(const std::string& packFormatId) {
  auto it = std::find_if(
    SPEAKER_SETUPS.begin(), SPEAKER_SETUPS.end(),
    [packFormatId](auto speakerSetup) { return speakerSetup.packFormatId == packFormatId; });
  if (it != SPEAKER_SETUPS.end()) {
    return *it;
  }
  else {
    return SpeakerSetup{};
  }
}

}  // namespace plugin
}  // namespace ear
