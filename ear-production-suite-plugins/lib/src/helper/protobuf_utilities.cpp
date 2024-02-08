#include "helper/protobuf_utilities.hpp"

#include <helper/adm_preset_definitions_helper.h>

namespace ear {
namespace plugin {
namespace proto {

proto::DirectSpeakersTypeMetadata* convertPackFormatToEpsMetadata(
    int packFormatIdValue) {
  auto commonDefinitionHelper = AdmPresetDefinitionsHelper::getSingleton();

  auto ds_metadata = new proto::DirectSpeakersTypeMetadata();
  ds_metadata->set_packformatidvalue(packFormatIdValue);

  auto pfData = commonDefinitionHelper->getPackFormatData(1, packFormatIdValue);
  if (pfData) {
    for (auto cfData : pfData->relatedChannelFormats) {
      auto newSpeaker = ds_metadata->add_speakers();
      newSpeaker->set_id(cfData->idValue);
      newSpeaker->set_is_lfe(cfData->isLfe);
      for (auto const& speakerLabel : cfData->speakerLabels) {
        newSpeaker->add_labels(speakerLabel);
      }
      newSpeaker->mutable_position()->set_azimuth(cfData->azimuth);
      newSpeaker->mutable_position()->set_elevation(cfData->elevation);
    }
  }
  return ds_metadata;
}

}  // namespace proto
}  // namespace plugin
}  // namespace ear
