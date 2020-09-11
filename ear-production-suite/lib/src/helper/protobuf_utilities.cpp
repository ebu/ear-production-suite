#include "helper/protobuf_utilities.hpp"
#include "speaker_setups.hpp"

#include <algorithm>

namespace ear {
namespace plugin {
namespace proto {

std::map<SpeakerLayout, std::string>
    SpeakerLayoutTranslator::SpeakerLayoutProtoToEarLUT_ = {
        {SpeakerLayout::ITU_BS_2051_0_1_0, std::string("AP_00010001")},
        {SpeakerLayout::ITU_BS_2051_0_2_0, std::string("AP_00010002")},
        {SpeakerLayout::ITU_BS_2051_0_5_0, std::string("AP_00010003")},
        {SpeakerLayout::ITU_BS_2051_2_5_0, std::string("AP_00010004")},
        {SpeakerLayout::ITU_BS_2051_4_5_0, std::string("AP_00010005")},
        {SpeakerLayout::ITU_BS_2051_4_5_1, std::string("AP_00010010")},
        {SpeakerLayout::ITU_BS_2051_3_7_0, std::string("AP_00010007")},
        {SpeakerLayout::ITU_BS_2051_4_9_0, std::string("AP_00010008")},
        {SpeakerLayout::ITU_BS_2051_9_10_3, std::string("AP_00010009")},
        {SpeakerLayout::ITU_BS_2051_0_7_0, std::string("AP_0001000f")},
        {SpeakerLayout::ITU_BS_2051_4_7_0, std::string("AP_00010017")},
        {SpeakerLayout::ITU_BS_2051_0_5_0_NO_LFE, std::string{"AP_0001000c"}},
        {SpeakerLayout::ITU_BS_755_0_3_0, std::string{"AP_0001000a"}},
        {SpeakerLayout::ITU_BS_2094_2_7_0_AP_00010016, std::string{"AP_00020026"}}
};

std::map<std::string, SpeakerLayout>
    SpeakerLayoutTranslator::SpeakerLayoutEarToProtoLUT_ = {
        {std::string("AP_00010001"), SpeakerLayout::ITU_BS_2051_0_1_0},
        {std::string("AP_00010002"), SpeakerLayout::ITU_BS_2051_0_2_0},
        {std::string("AP_00010003"), SpeakerLayout::ITU_BS_2051_0_5_0},
        {std::string("AP_00010004"), SpeakerLayout::ITU_BS_2051_2_5_0},
        {std::string("AP_00010005"), SpeakerLayout::ITU_BS_2051_4_5_0},
        {std::string("AP_00010010"), SpeakerLayout::ITU_BS_2051_4_5_1},
        {std::string("AP_00010007"), SpeakerLayout::ITU_BS_2051_3_7_0},
        {std::string("AP_00010008"), SpeakerLayout::ITU_BS_2051_4_9_0},
        {std::string("AP_00010009"), SpeakerLayout::ITU_BS_2051_9_10_3},
        {std::string("AP_0001000f"), SpeakerLayout::ITU_BS_2051_0_7_0},
        {std::string("AP_00010017"), SpeakerLayout::ITU_BS_2051_4_7_0},
        {std::string{"AP_0001000c"}, SpeakerLayout::ITU_BS_2051_0_5_0_NO_LFE},
        {std::string{"AP_0001000a"}, SpeakerLayout::ITU_BS_755_0_3_0},
        {std::string{"AP_00020026"}, SpeakerLayout::ITU_BS_2094_2_7_0_AP_00010016}
};

std::string SpeakerLayoutTranslator::proto2ear(const SpeakerLayout& layout) {
  auto it = SpeakerLayoutProtoToEarLUT_.find(layout);

  if (it != SpeakerLayoutProtoToEarLUT_.end()) {
    return it->second;
  } else {
    return std::string();
  }
};

SpeakerLayout SpeakerLayoutTranslator::ear2proto(const std::string& layout) {
  auto it = SpeakerLayoutEarToProtoLUT_.find(layout);

  if (it != SpeakerLayoutEarToProtoLUT_.end()) {
    return it->second;
  } else {
    return SpeakerLayout::ITU_BS_2051_UNKNOWN;
  }
}
}  // namespace proto
}  // namespace plugin
}  // namespace ear
