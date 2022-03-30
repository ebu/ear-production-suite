//
// Created by Richard Bailey on 25/02/2022.
//

#include "item_store.hpp"
#include <google/protobuf/util/message_differencer.h>

using namespace ear::plugin;

proto::InputItemMetadata ItemStore::getItem(
    const communication::ConnectionId& id) const {
  return store_.at(id);
}

proto::InputItemMetadata const& ItemStore::get(
    const communication::ConnectionId& id) const {
  std::optional<proto::InputItemMetadata> item;
  auto it = store_.find(id);
  assert(it != store_.end());
  return it->second;
}

std::map<communication::ConnectionId, proto::InputItemMetadata>
ItemStore::allItems() const {
  return store_;
}

std::multimap<int, communication::ConnectionId> ItemStore::routeMap() const {
  std::multimap<int, communication::ConnectionId> routes;
  std::transform(store_.cbegin(), store_.cend(),
                 std::inserter(routes, routes.begin()),
                 [](auto const& idItemPair) {
                   return std::make_pair(idItemPair.second.routing(),
                                         idItemPair.first);
                 });
  return routes;
}

void ItemStore::setItem(
    const communication::ConnectionId& id,
    const proto::InputItemMetadata& item) {
  using google::protobuf::util::MessageDifferencer;

  assert(id.string() == item.connection_id());
  if (auto result = store_.emplace(id, item); result.second) {
    fireEvent([&item](auto const& listener) {
      listener->itemAdded(item);
    });
  } else {
    auto previousItem = result.first->second;
    if(!MessageDifferencer::ApproximatelyEquals(previousItem, item)) {
        result.first->second = item;
        fireEvent([&previousItem, &item](auto const& listener) {
          listener->itemChanged(previousItem, item);
        });
    }
  }
}

void ItemStore::removeItem(const communication::ConnectionId& id) {
  if(auto it = store_.find(id); it != store_.end()) {
    auto item = it->second;
    store_.erase(it);
    fireEvent([&item](auto const& listener) {
      listener->itemRemoved(item);
    });
  }
}

void ItemStore::clearChanged() {
  bool clearedAny{false};
  for(auto& item : store_) {
    if(item.second.changed()) {
      item.second.set_changed(false);
      clearedAny = true;
    }
  }

  if(clearedAny) {
    fireEvent([](auto const& listener) {
      listener->changesCleared();
    });
  }
}

ItemMap ItemStore::get() const {
  return store_;
}

void ItemStore::addListener(Listener* listener) {
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
void ItemStore::Listener::changesCleared() {
  clearChanges();
}
