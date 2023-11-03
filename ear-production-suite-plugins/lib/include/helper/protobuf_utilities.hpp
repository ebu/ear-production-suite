#pragma once

#include <map>
#include "type_metadata.pb.h"
#include <speaker_setups.hpp>

namespace ear {
namespace plugin {
namespace proto {

inline proto::DirectSpeakersTypeMetadata* convertSpeakerSetupToEpsMetadata(
    int setupIndex) {
  auto ds_metadata = new proto::DirectSpeakersTypeMetadata();

  ds_metadata->set_layout(proto::SpeakerLayout(setupIndex));
  for (auto speaker : speakerSetupByIndex(setupIndex).speakers) {
    auto newSpeaker = ds_metadata->add_speakers();
    newSpeaker->set_is_lfe(speaker.isLfe);
    newSpeaker->add_labels(speaker.label);
    newSpeaker->mutable_position()->set_azimuth(speaker.azimuth);
    newSpeaker->mutable_position()->set_elevation(speaker.elevation);
  }
  return ds_metadata;
}

}  // namespace proto
}  // namespace plugin
}  // namespace ear
