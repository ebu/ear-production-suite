//
// Created by Richard Bailey on 09/03/2022.
//

#ifndef EAR_PRODUCTION_SUITE_METADATA_HPP
#define EAR_PRODUCTION_SUITE_METADATA_HPP
#include "programme_store.pb.h"
#include "input_item_metadata.pb.h"
#include "communication/common_types.hpp"

namespace ear::plugin {

struct InputItem {
  communication::ConnectionId id;
  proto::InputItemMetadata data;
};

using ItemMap = std::map<communication::ConnectionId, proto::InputItemMetadata>;
using RouteMap = std::multimap<int, communication::ConnectionId>;

struct ProgrammeStatus {
  int index;
  std::string id;
  bool isSelected;
};

struct Movement {
  int from;
  int to;
};

struct ProgrammeObject {
  proto::Object programmeObject;
  proto::InputItemMetadata inputMetadata;
};

class ProgrammeObjects {
 public:
  ProgrammeObjects(ProgrammeStatus status,
                   proto::Programme const& programme,
                   ItemMap const& inputItems)
      : status{status},
        name_{programme.has_name() ? programme.name() : ""},
        hasLanguage_{programme.has_language()},
        language_{programme.has_language() ? programme.language() : ""} {
    data.reserve(programme.element_size());
    for (auto const& element : programme.element()) {
      if (element.has_object()) {
        auto id = element.object().connection_id();
        if (auto inputItemIt = inputItems.find(id);
            inputItemIt != inputItems.end()) {
          data.push_back({element.object(), inputItemIt->second});
        }
      }
    }
  }

  [[nodiscard]]
  int index() const {
    return status.index;
  }

  [[nodiscard]]
  bool isSelected() const {
    return status.isSelected;
  }

  [[nodiscard]]
  bool hasLanguage() const {
    return hasLanguage_;
  };

  std::string const& language() {
    return language_;
  }

  [[nodiscard]]
  std::optional<ProgrammeObject> dataItem(communication::ConnectionId const& id) const {
    auto it = std::find_if(data.begin(), data.end(), [&id](auto const& item) {
      return communication::ConnectionId(item.inputMetadata.connection_id()) == id;
    });
    std::optional<ProgrammeObject> item;
    if(it != data.end()) {
      item = *it;
    }
    return item;
  }
  using const_iterator = std::vector<ProgrammeObject>::const_iterator;
  [[nodiscard]]
  const_iterator begin() const {
    return data.begin();
  }
  [[nodiscard]]
  const_iterator end() const {
    return data.end();
  }
  [[nodiscard]]
  const_iterator cbegin() const {
    return data.cbegin();
  }
  [[nodiscard]]
  const_iterator cend() const {
    return data.cend();
  }
 private:
  std::string name_;
  bool hasLanguage_;
  std::string language_;
  ProgrammeStatus status;
  std::vector<ProgrammeObject> data;
};

}

#endif  // EAR_PRODUCTION_SUITE_METADATA_HPP
