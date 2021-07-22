#pragma once

#include "helper/protobuf_utilities.hpp"
#include "../../../common_helpers/common_definition_helper.h"

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
  static ear::HOATypeMetadata convert(
      const proto::HoaTypeMetadata &epsMetadata, AdmCommonDefinitionHelper &commonDefinitionHelper) {
   // auto commonDefinitionHelper = AdmCommonDefinitionHelper::getSingleton();
    //auto elementRelationships =
    //    commonDefinitionHelper.getElementRelationships();
    auto pfData = commonDefinitionHelper.getPackFormatData(
        4, epsMetadata.hoatypeindex());
    auto cfData = pfData->relatedChannelFormats;

    ear::HOATypeMetadata earMetadata;
    earMetadata.degrees.resize(cfData.size());
    earMetadata.orders.resize(cfData.size());
    
    for (int channelNum = 0; channelNum < cfData.size(); channelNum++) {
      auto bfData = cfData[channelNum]->channelFormat->getElements<adm::AudioBlockFormatHoa>();
      int order = bfData[0].get<adm::Order>().get();
      int degree = bfData[0].get<adm::Degree>().get();
      earMetadata.degrees[channelNum] = degree;
      earMetadata.orders[channelNum] = order;

      if (channelNum == 0) {
        std::string normalisation = bfData[0].get<adm::Normalization>().get();
        earMetadata.normalization = normalisation;
      }
    }
    return earMetadata;
  }
  /*
  //ME ADDED THIS IN THE IMAGE OF OBJECT RATHER THAN DS. NOT SURE IF THIS IS RIGHT, MAY NEED REWORKING
  static ear::HoaTypeMetadata convert(//FIRST NEED TO ADD SUPPORT FOR HOATYPE METADATA, POTENTIALLY IN METADATA.HPP? 
      const proto::ObjectsTypeMetadata &epsMetadata) {
    ear::HoaTypeMetadata earMetadata;
    //earMetadata.gain = epsMetadata.gain();
   // earMetadata.position = ear::PolarPosition(
    //    epsMetadata.position().azimuth(), epsMetadata.position().elevation(),
    //    epsMetadata.position().distance());
   // earMetadata.width = epsMetadata.width();
    //earMetadata.height = epsMetadata.height();
    //earMetadata.depth = epsMetadata.depth();
   // earMetadata.diffuse = epsMetadata.diffuse();
    //// XXX: uncomment to enable divergence
    //// earMetadata.objectDivergence =
    ////     PolarObjectDivergence{epsMetadata.factor(), epsMetadata.range()};
    return earMetadata;
    //ME END
    */
    
};

}  // namespace plugin
}  // namespace ear
