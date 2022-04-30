#pragma once
#include "programme_store.pb.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace ear::plugin {
inline std::string newProgrammeInternalId() {
  auto uuid = boost::uuids::random_generator()();
  auto uuidStr = boost::uuids::to_string(uuid);
  return uuidStr;
}
}
