//
// Created by Richard Bailey on 01/03/2022.
//

#include "store_metadata.hpp"

using namespace ear::plugin;

void Metadata::addItems(ProgrammeStatus status,
    const std::vector<proto::Object>& items) {
  std::vector<ProgrammeObject> pairs;
  pairs.reserve(items.size());
  auto selectedIndex = programmeStore.get().selected_programme_index();
  for(auto const& item : items) {
    auto id = communication::ConnectionId(item.connection_id());
    pairs.push_back({item, itemStore.getItem(id)});
  }

  fireEvent([status, &pairs](auto const& listener) {
    listener->itemsAddedToProgramme(status, pairs);
  });
}

void Metadata::changeStore(proto::ProgrammeStore const& store) {
  auto items = itemStore.get();
  fireEvent([&store, &items](auto const& listener) {
    listener->dataReset(store, items);
  });
}
void Metadata::addProgramme(
    ProgrammeStatus status, const proto::Programme& programme) {
  fireEvent([status, &programme](auto const& listener) {
    listener->programmeAdded(status.index, programme);
  });
}
void Metadata::moveProgramme(
    Movement movement,
    const proto::Programme& programme) {
  fireEvent([movement, &programme](auto const& listener) {
    listener->programmeMoved(movement, programme);
  });
}
void Metadata::removeProgramme(int index) {
  fireEvent([index](auto const& listener) {
    listener->programmeRemoved(index);
  });
}
void Metadata::selectProgramme(int index, proto::Programme const& programme) {
  ProgrammeObjects objects({index, true}, programme, itemStore.allItems());
  fireEvent([&objects](auto const& listener) {
    listener->programmeSelected(objects);
  });
}

void Metadata::removeItem(ProgrammeStatus status, const proto::Object& element) {
  auto id = communication::ConnectionId(element.connection_id());
  auto item = itemStore.maybeGet(id);
  assert(item);

  fireEvent([status, &element, &item](auto const& listener) {
    listener->itemRemovedFromProgramme(status, {element, *item});
  });
}
void Metadata::updateItem(ProgrammeStatus status, const proto::Object& element) {
  auto id = communication::ConnectionId(element.connection_id());
  auto item = itemStore.maybeGet(id);
  assert(item);

  fireEvent([status, &element, &item](auto const& listener) {
    listener->programmeItemUpdated(status, {element, *item});
  });

}

void Metadata::setAutoMode(bool enabled) {
  fireEvent([enabled](auto const& listener) {
    listener->autoModeChanged(enabled);
  });
}

void Metadata::updateProgramme(
    int index, const proto::Programme& programme) {
  fireEvent([index, &programme](auto const& listener) {
    listener->programmeUpdated(index, programme);
  });
}

void Metadata::addItem(
    const proto::InputItemMetadata& item) {
  fireEvent([&item](auto const& listener) {
    listener->inputAdded({item.connection_id(), item});
  });
}
void Metadata::changeItem(
    const proto::InputItemMetadata& oldItem,
    const proto::InputItemMetadata& newItem) {
  programmeStore.autoUpdateFrom(itemStore);
  fireEvent([&newItem](auto const& listener) {
    listener->inputUpdated({newItem.connection_id(), newItem});
  });
}
void Metadata::removeItem(
    const proto::InputItemMetadata& oldItem) {
  programmeStore.removeElementFromAllProgrammes(oldItem.connection_id());
  programmeStore.autoUpdateFrom(itemStore);
  fireEvent([&oldItem](auto const& listener) {
    listener->inputRemoved(oldItem.connection_id());
  });

}
void Metadata::clearChanges() {}
