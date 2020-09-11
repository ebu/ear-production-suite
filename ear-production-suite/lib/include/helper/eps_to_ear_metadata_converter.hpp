#pragma once

#include "helper/protobuf_utilities.hpp"

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
    auto packFormatId =
        proto::SpeakerLayoutTranslator::proto2ear(epsMetadata.layout());
    std::vector<ear::DirectSpeakersTypeMetadata> earMetadata;
    for (auto epsSpeaker : epsMetadata.speakers()) {
      ear::DirectSpeakersTypeMetadata earSpeaker;
      earSpeaker.audioPackFormatID = packFormatId;
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
};

}  // namespace plugin
}  // namespace ear
