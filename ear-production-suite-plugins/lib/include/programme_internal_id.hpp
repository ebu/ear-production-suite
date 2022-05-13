#pragma once
#include "programme_store.pb.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace ear::plugin {

using ProgrammeInternalId = std::string;

inline ProgrammeInternalId newProgrammeInternalId() {
  auto uuid = boost::uuids::random_generator()();
  auto uuidStr = boost::uuids::to_string(uuid);
  return uuidStr;
}

inline const ear::plugin::proto::Programme* getProgrammeWithId(const ear::plugin::proto::ProgrammeStore &programmeStore, const ProgrammeInternalId& progId) {
  for(const auto& programme : programmeStore.programme()) {
    if(programme.has_programme_internal_id() && programme.programme_internal_id() == progId) {
      return &programme;
    }
  }
  return nullptr;
}

inline int getProgrammeIndexFromId(const ear::plugin::proto::ProgrammeStore &programmeStore, const ProgrammeInternalId& progId) {
  for(int i = 0; i < programmeStore.programme_size(); i++) {
    if(programmeStore.programme(i).has_programme_internal_id() && programmeStore.programme(i).programme_internal_id() == progId) {
      return i;
    }
  }
  return -1;
}

}
