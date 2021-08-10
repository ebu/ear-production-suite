#pragma once

#include "communication/common_types.hpp"
#include "input_item_metadata.pb.h"
#include <map>

namespace ear {
namespace plugin {

using ItemStore =
    std::map<communication::ConnectionId, proto::InputItemMetadata>;

}
}  // namespace ear
