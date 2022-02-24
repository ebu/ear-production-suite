#pragma once

#include "communication/common_types.hpp"
#include "input_item_metadata.pb.h"
#include <map>
#include <vector>
#include "helper/weak_ptr.hpp"

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
  class Listener {
   public:
    void itemAdded(proto::InputItemMetadata item) {
      addItem(item);
    }

    void itemChanged(proto::InputItemMetadata oldItem,
                     proto::InputItemMetadata newItem) {
      changeItem(oldItem, newItem);
    }

    void itemRemoved(proto::InputItemMetadata oldItem) {
      removeItem(oldItem);
    }
   protected:
    virtual void addItem(proto::InputItemMetadata const& item) = 0;
    virtual void changeItem(proto::InputItemMetadata const& oldItem,
                            proto::InputItemMetadata const& newItem) = 0;

    virtual void removeItem(proto::InputItemMetadata const& oldItem) = 0;
  };

  // TODO remove this as setItem will do.
  void addItem(communication::ConnectionId const& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    proto::InputItemMetadata emptyItemMetadata;
    emptyItemMetadata.set_connection_id(id.string());
    store_[id] = emptyItemMetadata;
    notifyItemAdded(emptyItemMetadata);
  }

  std::optional<proto::InputItemMetadata> setItem(communication::ConnectionId const& id, proto::InputItemMetadata const& item) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::optional<proto::InputItemMetadata> previousItem;
    assert(id.string() == item.connection_id());
    if (auto result = store_.emplace(id, item); result.second) {
      // Currently this should never be called, but this path should be used
      // when we remove addItem
      assert(false);
      notifyItemAdded(item);
    } else {
      previousItem = result.first->second;
      result.first->second = item;
      notifyItemChanged(result.first->second, item);
    }
    return previousItem;
  }

  void removeItem(communication::ConnectionId const& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(auto it = store_.find(id); it != store_.end()) {
      auto item = it->second;
      store_.erase(it);
      notifyItemRemoved(item);
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

  void addListener(std::shared_ptr<Listener> const& listener) {
    listeners_.emplace_back(listener);
  }

 private:
  mutable std::mutex mutex_;
  std::vector<std::weak_ptr<Listener>> listeners_;
  std::map<communication::ConnectionId, proto::InputItemMetadata> store_;

  void notifyItemAdded(proto::InputItemMetadata const& item) {
    removeDeadPointersAfter(listeners_,
                            [&item](auto const& listener) {
                              listener->itemAdded(item);
                            });
  }

  void notifyItemChanged(proto::InputItemMetadata const& newItem,
                   proto::InputItemMetadata const& oldItem) {
    removeDeadPointersAfter(listeners_,
                            [&newItem, &oldItem](auto const& listener) {
                              listener->itemChanged(newItem, oldItem);
                            });
  }

  void notifyItemRemoved(proto::InputItemMetadata const& oldItem) {
    removeDeadPointersAfter(listeners_,
                            [&oldItem](auto const& listener) {
                              listener->itemRemoved(oldItem);
                            });
  }

};

//using ItemStore =
//    std::map<communication::ConnectionId, proto::InputItemMetadata>;

}
}  // namespace ear
