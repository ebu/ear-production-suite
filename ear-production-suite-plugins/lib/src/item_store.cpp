//
// Created by Richard Bailey on 25/02/2022.
//

#include "item_store.hpp"

using namespace ear::plugin;

proto::InputItemMetadata ItemStore::get(
    const communication::ConnectionId& id) const {
  std::lock_guard<std::mutex> lock(mutex_);
  return store_.at(id);
}

std::optional<proto::InputItemMetadata> ItemStore::maybeGet(
    const communication::ConnectionId& id) const {
  std::lock_guard<std::mutex> lock(mutex_);
  std::optional<proto::InputItemMetadata> item;
  if(auto it = store_.find(id); it != store_.end()) {
    item = it->second;
  }
  return item;
}

std::map<communication::ConnectionId, proto::InputItemMetadata>
ItemStore::allItems() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return store_;
}

std::multimap<int, communication::ConnectionId> ItemStore::routeMap() const {
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

void ItemStore::addItem(const communication::ConnectionId& id) {
  std::lock_guard<std::mutex> lock(mutex_);
  proto::InputItemMetadata emptyItemMetadata;
  emptyItemMetadata.set_connection_id(id.string());
  store_[id] = emptyItemMetadata;
  notifyItemAdded(emptyItemMetadata);
}

std::optional<proto::InputItemMetadata> ItemStore::setItem(
    const communication::ConnectionId& id,
    const proto::InputItemMetadata& item) {
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
void ItemStore::removeItem(const communication::ConnectionId& id) {
  std::lock_guard<std::mutex> lock(mutex_);
  if(auto it = store_.find(id); it != store_.end()) {
    auto item = it->second;
    store_.erase(it);
    notifyItemRemoved(item);
  }
}

bool ItemStore::clearChanged() {
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

void ItemStore::addListener(const std::shared_ptr<Listener>& listener) {
  listeners_.emplace_back(listener);
}

void ItemStore::Listener::itemAdded(const proto::InputItemMetadata& item) {
  addItem(item);
}

void ItemStore::Listener::itemChanged(const proto::InputItemMetadata& oldItem,
                                      const proto::InputItemMetadata& newItem) {
  changeItem(oldItem, newItem);
}
void ItemStore::Listener::itemRemoved(const proto::InputItemMetadata& oldItem) {
  removeItem(oldItem);
}
void ItemStore::Listener::dispatch(std::function<void()> event) {
  event();
}
