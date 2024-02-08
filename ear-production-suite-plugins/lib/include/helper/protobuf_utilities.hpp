#pragma once

#include "type_metadata.pb.h"

namespace ear {
namespace plugin {
namespace proto {

proto::DirectSpeakersTypeMetadata* convertPackFormatToEpsMetadata(
    int packFormatIdValue);

}  // namespace proto
}  // namespace plugin
}  // namespace ear
