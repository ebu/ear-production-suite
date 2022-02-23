#pragma once

#include "communication/common_types.hpp"
#include "input_item_metadata.pb.h"
#include <map>

namespace ear {
namespace plugin {

/*
 * Item store fully describes items provided by all connected input plugins
 * along with their configuration in those plugins.
 * It is not saved with the scene but populated by metadata from the input
 * plugins.
 */
class ItemStore {
 public:
  void addItem(communication::ConnectionId const& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    proto::InputItemMetadata emptyItemMetadata;
    emptyItemMetadata.set_connection_id(id.string());
    store_[id] = emptyItemMetadata;
  }

  std::optional<proto::InputItemMetadata> setItem(communication::ConnectionId const& id, proto::InputItemMetadata const& item) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::optional<proto::InputItemMetadata> previousItem;
    assert(id.string() == item.connection_id());
    if(auto result = store_.emplace(id, item);
        !result.second) {
      previousItem = result.first->second;
      result.first->second = item;
    }
    return previousItem;
  }

  void removeItem(communication::ConnectionId const& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(auto it = store_.find(id); it != store_.end()) {
      store_.erase(it);
    }
  }

  [[ nodiscard ]]
  proto::InputItemMetadata get(communication::ConnectionId const& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return store_.at(id);
  }

  std::optional<proto::InputItemMetadata> maybeGet(communication::ConnectionId const& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::optional<proto::InputItemMetadata> item;
    if(auto it = store_.find(id); it != store_.end()) {
      item = it->second;
    }
    return item;
  }

  [[ nodiscard ]]
  std::map<communication::ConnectionId, proto::InputItemMetadata> allItems() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return store_;
  }

  [[ nodiscard ]]
  bool clearChanged() {
    std::lock_guard<std::mutex> lock(mutex_);
    bool clearedAny{false};
    for(auto& item : store_) {
      if(item.second.changed()) {
        item.second.set_changed(false);
        clearedAny = true;
      }
    }
    return clearedAny;
  }

  std::multimap<int, communication::ConnectionId> routeMap() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::multimap<int, communication::ConnectionId> routes;
    std::transform(store_.cbegin(), store_.cend(),
                   std::inserter(routes, routes.begin()),
                   [](auto const& idItemPair) {
                     return std::make_pair(idItemPair.second.routing(),
                                           idItemPair.first);
                   });
    return routes;
  }

 private:
  mutable std::mutex mutex_;
  std::map<communication::ConnectionId, proto::InputItemMetadata> store_;
};

//using ItemStore =
//    std::map<communication::ConnectionId, proto::InputItemMetadata>;

}
}  // namespace ear
