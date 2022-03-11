//
// Created by Richard Bailey on 01/03/2022.
//

#include "store_metadata.hpp"
#include <algorithm>

using namespace ear::plugin;

void EventDispatcher::dispatchEvent(std::function<void()> event) {
  doDispatch(std::move(event));
}

void Metadata::addItems(ProgrammeStatus status,
    const std::vector<proto::Object>& items) {
  std::vector<ProgrammeObject> pairs;
  pairs.reserve(items.size());
  auto selectedIndex = programmeStore.get().selected_programme_index();
  for(auto const& item : items) {
    auto id = communication::ConnectionId(item.connection_id());
    pairs.push_back({item, itemStore.getItem(id)});
  }

  fireEvent(&MetadataListener::notifyItemsAddedToProgramme,
            status, pairs);
}

void Metadata::changeStore(proto::ProgrammeStore const& store) {
  auto items = itemStore.get();
  fireEvent(&MetadataListener::notifyDataReset,
            store, items);
}

void Metadata::addProgramme(
    ProgrammeStatus status, const proto::Programme& programme) {
  fireEvent(&MetadataListener::notifyProgrammeAdded,
            status.index, programme);
}

void Metadata::moveProgramme(
    Movement movement,
    const proto::Programme& programme) {
  fireEvent(&MetadataListener::notifyProgrammeMoved,
            movement, programme);
}

void Metadata::removeProgramme(int index) {
  fireEvent(&MetadataListener::notifyProgrammeRemoved,
            index);
}

void Metadata::selectProgramme(int index, proto::Programme const& programme) {
  ProgrammeObjects objects({index, true}, programme, itemStore.allItems());
  fireEvent(&MetadataListener::notifyProgrammeSelected,
            objects);
}

void Metadata::removeItem(ProgrammeStatus status, const proto::Object& element) {
  auto id = communication::ConnectionId(element.connection_id());
  auto const& item = itemStore.get(id);

  fireEvent(&MetadataListener::notifyItemRemovedFromProgramme,
            status, ProgrammeObject{element, item});
}

void Metadata::updateItem(ProgrammeStatus status, const proto::Object& element) {
  auto id = communication::ConnectionId(element.connection_id());
  auto const& item = itemStore.get(id);

  fireEvent(&MetadataListener::notifyProgrammeItemUpdated,
            status, ProgrammeObject{element, item});
}

void Metadata::setAutoMode(bool enabled) {
  fireEvent(&MetadataListener::notifyAutoModeChanged,
            enabled);
}

void Metadata::updateProgramme(
    int index, const proto::Programme& programme) {
  fireEvent(&MetadataListener::notifyProgrammeUpdated,
            index, programme);
}

void Metadata::addItem(
    const proto::InputItemMetadata& item) {
  fireEvent(&MetadataListener::notifyInputAdded,
            InputItem{item.connection_id(), item});
}

void Metadata::changeItem(
    const proto::InputItemMetadata& oldItem,
    const proto::InputItemMetadata& newItem) {
  programmeStore.autoUpdateFrom(itemStore);
  auto selectedIndex = programmeStore.get().selected_programme_index();

  for(auto i = 0; i != programmeStore.programmeCount(); ++i) {
    auto const& programme = programmeStore.programmeAtIndex(i);
    auto const& elements = programme->element();
    auto const& id = newItem.connection_id();
    if(auto it = std::find_if(elements.begin(), elements.end(), [id, selectedIndex](auto const& element){
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

  fireEvent(&MetadataListener::notifyInputUpdated,
            InputItem{newItem.connection_id(), newItem});
}

void Metadata::removeItem(
    const proto::InputItemMetadata& oldItem) {
  programmeStore.removeElementFromAllProgrammes(oldItem.connection_id());
  programmeStore.autoUpdateFrom(itemStore);
  fireEvent(&MetadataListener::notifyInputRemoved,
            oldItem.connection_id());
}
void Metadata::clearChanges() {}
