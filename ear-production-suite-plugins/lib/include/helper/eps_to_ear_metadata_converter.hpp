#pragma once

#include "helper/protobuf_utilities.hpp"
#include "helper/adm_preset_definitions_helper.h"
#include <string>
#include <memory>
#include <adm/detail/id_parser.hpp>

namespace ear {
namespace plugin {

struct EpsToEarMetadataConverter {
  static ear::ObjectsTypeMetadata convert(
      const proto::ObjectsTypeMetadata &epsMetadata) {
    ear::ObjectsTypeMetadata earMetadata;
    earMetadata.gain = epsMetadata.gain();
    earMetadata.position = ear::PolarPosition(
        epsMetadata.position().azimuth(), epsMetadata.position().elevation(),
        epsMetadata.position().distance());
    earMetadata.width = epsMetadata.width();
    earMetadata.height = epsMetadata.height();
    earMetadata.depth = epsMetadata.depth();
    earMetadata.diffuse = epsMetadata.diffuse();
    // XXX: uncomment to enable divergence
    // earMetadata.objectDivergence =
    //     PolarObjectDivergence{epsMetadata.factor(), epsMetadata.range()};
    return earMetadata;
  }

  static std::vector<ear::DirectSpeakersTypeMetadata> convert(
      const proto::DirectSpeakersTypeMetadata &epsMetadata) {

    adm::AudioPackFormatId audioPackFormatID(
        adm::TypeDefinition::DIRECT_SPEAKERS,
        adm::AudioPackFormatIdValue(epsMetadata.packformatidvalue()));
    auto audioPackFormatIDstr = adm::formatId(audioPackFormatID);

    std::vector<ear::DirectSpeakersTypeMetadata> earMetadata;
    for (auto epsSpeaker : epsMetadata.speakers()) {
      ear::DirectSpeakersTypeMetadata earSpeaker;
      earSpeaker.audioPackFormatID = audioPackFormatIDstr;
      earSpeaker.position = ear::PolarSpeakerPosition(
          epsSpeaker.position().azimuth(), epsSpeaker.position().elevation(),
          epsSpeaker.position().distance());
      std::vector<std::string> labels;
      for (auto label : epsSpeaker.labels()) {
        labels.push_back(label);
      }
      earSpeaker.speakerLabels = labels;
      if (epsSpeaker.is_lfe()) {
        earSpeaker.channelFrequency = ChannelFrequency{120.0};
      }
      earMetadata.push_back(earSpeaker);
    }
    return earMetadata;
  }
  static ear::HOATypeMetadata convert(
      const proto::HoaTypeMetadata &epsMetadata) {
    auto pfData = AdmPresetDefinitionsHelper::getSingleton().getPackFormatData(
        4, epsMetadata.packformatidvalue());
    ear::HOATypeMetadata earMetadata;

    if (pfData) {
      auto cfData = pfData->relatedChannelFormats;
      earMetadata.degrees.resize(cfData.size());
      earMetadata.orders.resize(cfData.size());

      for (int channelNum = 0; channelNum < cfData.size(); channelNum++) {
        auto bfData =
            cfData[channelNum]
                ->channelFormat->getElements<adm::AudioBlockFormatHoa>();
        int order = bfData[0].get<adm::Order>().get();
        int degree = bfData[0].get<adm::Degree>().get();
        earMetadata.degrees[channelNum] = degree;
        earMetadata.orders[channelNum] = order;

        if (channelNum == 0) {
          std::string normalisation = bfData[0].get<adm::Normalization>().get();
          earMetadata.normalization = normalisation;
        }
      }
    }

    return earMetadata;
  }
};

}  // namespace plugin
}  // namespace ear
