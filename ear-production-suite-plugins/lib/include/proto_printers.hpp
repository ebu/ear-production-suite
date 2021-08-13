#pragma once

#include "programme_store.pb.h"
#include <iostream>
#include <google/protobuf/util/json_util.h>

namespace google {
namespace protobuf {

std::ostream& operator<< (std::ostream& os, Message const& message);

}
}

