//
// Created by Richard Bailey on 01/03/2022.
//

#include "store_metadata.hpp"
#include <google/protobuf/util/message_differencer.h>
#include <algorithm>

using namespace ear::plugin;

void EventDispatcher::dispatchEvent(std::function<void()> event) {
  doDispatch(std::move(event));
}

void Metadata::doAddItems(ProgrammeStatus status,
    const std::vector<proto::Object>& items) {
  std::vector<ProgrammeObject> pairs;
  pairs.reserve(items.size());
  for(auto const& item : items) {
    auto id = communication::ConnectionId(item.connection_id());
    pairs.push_back({item, itemStore_.at(id)});
  }

  fireEvent(&MetadataListener::notifyItemsAddedToProgramme,
            status, pairs);
}

void Metadata::doChangeStore(proto::ProgrammeStore const& store) {
  auto items = itemStore_;
  fireEvent(&MetadataListener::notifyDataReset,
            store, items);
}

void Metadata::doAddProgramme(
    ProgrammeStatus status, const proto::Programme& programme) {
  fireEvent(&MetadataListener::notifyProgrammeAdded,
            status.index, programme);
}

void Metadata::doMoveProgramme(
    Movement movement,
    const proto::Programme& programme) {
  fireEvent(&MetadataListener::notifyProgrammeMoved,
            movement, programme);
}

void Metadata::doRemoveProgramme(int index) {
  fireEvent(&MetadataListener::notifyProgrammeRemoved,
            index);
}

void Metadata::doSelectProgramme(int index, proto::Programme const& programme) {
  ProgrammeObjects objects({index, true}, programme, itemStore_);
  fireEvent(&MetadataListener::notifyProgrammeSelected,
            objects);
}

void Metadata::doRemoveItem(ProgrammeStatus status, const proto::Object& element) {
    EAR_LOGGER_TRACE(logger_, "remove programme item id {}", element.connection_id());
  auto id = communication::ConnectionId(element.connection_id());
  fireEvent(&MetadataListener::notifyItemRemovedFromProgramme,
            status, id);
}

void Metadata::doUpdateItem(ProgrammeStatus status, const proto::Object& element) {
  auto id = communication::ConnectionId(element.connection_id());
  auto const& item = itemStore_.at(id);

  fireEvent(&MetadataListener::notifyProgrammeItemUpdated,
            status, ProgrammeObject{element, item});
}

void Metadata::doSetAutoMode(bool enabled) {
  fireEvent(&MetadataListener::notifyAutoModeChanged,
            enabled);
}

void Metadata::doUpdateProgramme(
    int index, const proto::Programme& programme) {
  fireEvent(&MetadataListener::notifyProgrammeUpdated,
            index, programme);
}

void Metadata::doAddInputItem(
    const proto::InputItemMetadata& item) {
    EAR_LOGGER_TRACE(logger_, "addItem id {}", item.connection_id());
  fireEvent(&MetadataListener::notifyInputAdded,
            InputItem{item.connection_id(), item});
}

void Metadata::doChangeInputItem(
    const proto::InputItemMetadata& oldItem,
    const proto::InputItemMetadata& newItem) {
  fireEvent(&MetadataListener::notifyInputUpdated,
          InputItem{newItem.connection_id(), newItem});
  programmeStore.autoUpdateFrom(routeMap());
  auto selectedIndex = programmeStore.get().selected_programme_index();
  for(auto i = 0; i != programmeStore.programmeCount(); ++i) {
    auto const& programme = programmeStore.programmeAtIndex(i);
    auto const& elements = programme->element();
    auto const& id = newItem.connection_id();
    if(auto it = std::find_if(elements.begin(), elements.end(), [id](auto const& element){
          if(element.has_object()) {
            return element.object().connection_id() == id;
          }
          return false;
        });
        it != elements.end()) {
      fireEvent(&MetadataListener::notifyProgrammeItemUpdated,
                ProgrammeStatus{i, i == selectedIndex},
                ProgrammeObject{it->object(), newItem});
    }
  }
}

void Metadata::doRemoveInputItem(
    const proto::InputItemMetadata& oldItem) {
    EAR_LOGGER_TRACE(logger_, "removeInput id {}", oldItem.connection_id());
  programmeStore.removeElementFromAllProgrammes(oldItem.connection_id());
  programmeStore.autoUpdateFrom(routeMap());
  fireEvent(&MetadataListener::notifyInputRemoved,
            oldItem.connection_id());
}

void Metadata::addUIListener(std::weak_ptr<MetadataListener> listener) {
    std::lock_guard<std::mutex> lock(mutex_);
    uiListeners_.push_back(std::move(listener));
}

void Metadata::addListener(std::weak_ptr<MetadataListener> listener) {
    listeners_.push_back(std::move(listener));
}

void Metadata::setInputItemMetadata(
        const communication::ConnectionId& id,
        const proto::InputItemMetadata& item) {
    std::lock_guard<std::mutex> lock(mutex_);
    using google::protobuf::util::MessageDifferencer;

    assert(id.string() == item.connection_id());
    if (auto result = itemStore_.emplace(id, item); result.second) {
        doAddInputItem(item);
    } else {
        auto previousItem = result.first->second;
        if(!MessageDifferencer::ApproximatelyEquals(previousItem, item)) {
            result.first->second = item;
            doChangeInputItem(previousItem, item);
        }
    }
    itemStore_[id].set_changed(false);
}

void Metadata::removeInput(const communication::ConnectionId& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(auto it = itemStore_.find(id); it != itemStore_.end()) {
        auto item = it->second;
        itemStore_.erase(it);
        doRemoveInputItem(item);
    }
}

RouteMap Metadata::routeMap() const {
    RouteMap routes;
    std::transform(itemStore_.cbegin(), itemStore_.cend(),
                   std::inserter(routes, routes.begin()),
                   [](auto const& idItemPair) {
                       return std::make_pair(idItemPair.second.routing(),
                                             idItemPair.first);
                   });
    return routes;
}
