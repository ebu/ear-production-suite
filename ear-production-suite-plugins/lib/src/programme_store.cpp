//
// Created by Richard Bailey on 23/02/2022.
//

#include "../include/programme_store.hpp"
#include <algorithm>
#include "item_store.hpp"
#include "helper/move.hpp"
#include "helper/weak_ptr.hpp"

using namespace ear::plugin;

namespace {
[[nodiscard]] bool isItemInElement(communication::ConnectionId const& id,
                                   proto::ProgrammeElement const& element) {
  if (element.has_object()) {
    return communication::ConnectionId{element.object().connection_id()} == id;
  } else if (element.has_toggle()) {
    for (auto const& toggleElement : element.toggle().element()) {
      if (isItemInElement(id, toggleElement)) {
        return true;
      }
    }
  } else if (element.has_group()) {
    for (auto const& groupElement : element.group().element()) {
      if (isItemInElement(id, groupElement)) {
        return true;
      }
    }
  }
  return false;
}
}

bool ear::plugin::isItemInProgramme(communication::ConnectionId const& id,
                       proto::Programme const& programme) {
  auto elements = programme.element();
  return std::any_of(elements.cbegin(),
                     elements.cend(),
                     [&id](proto::ProgrammeElement const& element) {
    return isItemInElement(id, element);
  });
}

proto::ProgrammeStore ProgrammeStore::get() const {
  return store_;
}

std::optional<proto::Programme> ProgrammeStore::selectedProgramme() const {
  if (store_.has_selected_programme_index()) {
    auto index = store_.selected_programme_index();
    if (index >= 0) {  // could be -1 if plugin just initialised
      assert(index < store_.programme_size());
      auto& programme = store_.programme(index);
      return programme;
    }
  }
  return {};
}

std::optional<proto::Programme> ProgrammeStore::programmeAtIndex(
    int index) const {
  return programmeAtIndexImpl(index);
}

int ProgrammeStore::programmeCount() const {
  return store_.programme_size();
}

std::string ProgrammeStore::programmeName(int index) const {
  assert(index < store_.programme_size());
  return store_.programme(index).name();
}

bool ProgrammeStore::autoModeEnabled() const {
  return store_.auto_mode();
}

void ProgrammeStore::set(proto::ProgrammeStore const& store) {
  store_ = store;
  fireEvent([&store](auto const& listener) {
    listener->storeChanged(store);
  });
}

void ProgrammeStore::setAutoMode(bool enable) {
  auto old = store_.auto_mode();
  if(old != enable) {
    store_.set_auto_mode(enable);
    fireEvent([enable](auto const& listener) {
      listener->autoModeSet(enable);
    });
  }
}

void ProgrammeStore::selectProgramme(int index) {
  auto oldIndex = store_.selected_programme_index();
  if(oldIndex != index) {
    store_.set_selected_programme_index(index);
    auto prog = store_.programme(index);
    fireEvent([index, &prog](auto const& listener){
      listener->programmeSelected(index, prog);
    });
  }
}

void ProgrammeStore::addProgramme() {
  std::string name{"Programme_"};
  auto index = store_.programme_size();
  name.append(std::to_string(index));
  auto programme = addProgrammeImpl(name, "");
  bool selected = (index == store_.selected_programme_index());

  fireEvent([index, selected, &programme](auto const& listener) {
    listener->programmeAdded({index, selected}, *programme);
  });
}

void ProgrammeStore::moveProgramme(int oldIndex, int newIndex) {
  auto programmes = store_.mutable_programme();
  auto size = programmes->size();
  bool moved = false;
  if (oldIndex >= 0 && newIndex >= 0 && oldIndex < size && newIndex < size &&
      oldIndex != newIndex) {
    move(programmes->begin(), oldIndex, newIndex);
    auto programme = programmes->at(newIndex);
    fireEvent([oldIndex, newIndex, &programme](auto const& listener) {
      listener->programmeMoved(Movement{oldIndex, newIndex}, programme);
    });
  }
}

void ProgrammeStore::removeProgramme(int index) {
  auto programme = store_.mutable_programme();
  assert(index < store_.programme_size());
  programme->erase(programme->begin() + index);
  auto selected_index = store_.selected_programme_index();
  fireEvent([index](auto const& listener) {
    listener->programmeRemoved(index);
  });
  if(selected_index >= programme->size()) {
    auto newIndex = std::max<int>(programme->size() - 1, 0);
    store_.set_selected_programme_index(newIndex);
    auto prog = store_.programme(newIndex);
    fireEvent([newIndex, &prog](auto const& listener) {
      listener->programmeSelected(newIndex, prog);
    });
  }
}

void ProgrammeStore::setProgrammeName(int index, const std::string& name) {
  assert(index < store_.programme_size());
  if(name != store_.programme(index).name()) {
    store_.mutable_programme(index)->set_name(name);
    auto prog = store_.programme(index);
    auto selectedIndex = store_.selected_programme_index();
    fireEvent([index, &prog](auto const& listener) {
      listener->programmeUpdated(index, prog);
    });
  }
}

void ProgrammeStore::setProgrammeLanguage(int programmeIndex,
                                          const std::string& language) {
  if(!(store_.programme(programmeIndex).has_language() &&
        store_.programme(programmeIndex).language() == language)) {
    store_.mutable_programme(programmeIndex)->set_language(language);
    auto prog = store_.programme(programmeIndex);
    fireEvent([programmeIndex, &prog](auto const& listener) {
      listener->programmeUpdated(programmeIndex, prog);
    });
  }
}

void ProgrammeStore::clearProgrammeLanguage(int programmeIndex) {
  if(store_.programme(programmeIndex).has_language()) {
    store_.mutable_programme(programmeIndex)->clear_language();
    auto prog = store_.programme(programmeIndex);
    fireEvent([programmeIndex, &prog](auto const& listener) {
      listener->programmeUpdated(programmeIndex, prog);
    });
  }
}

void ProgrammeStore::addItemsToSelectedProgramme(std::vector<
    communication::ConnectionId> const& ids) {
  assert(!ids.empty());
  auto programmeIndex = 0;
  programmeIndex = store_.selected_programme_index();
  auto programme = store_.mutable_programme(programmeIndex);
  std::vector<proto::Object> elements;
  elements.reserve(ids.size());
  for(auto const& id : ids) {
    auto object = addObject(programme, id);
    elements.push_back(*object);
  }

  fireEvent([&elements, programmeIndex](auto const& listener) {
    listener->itemsAdded({programmeIndex, true}, elements);
  });
}

void ProgrammeStore::removeElementFromProgramme(int programmeIndex, int elementIndex) {
  assert(programmeIndex >=0 && programmeIndex < store_.programme_size());
  auto elements =
      store_.mutable_programme(programmeIndex)
          ->mutable_element();
  assert(elementIndex >= 0 && elementIndex < std::distance(elements->begin(), elements->end()));
  auto element = store_.programme(programmeIndex).element(elementIndex);
  ProgrammeStatus status{};
  bool hasObject{false};
  if(element.has_object()) {
    status.index = programmeIndex;
    status.isSelected = false;
    hasObject = true;
  }
  elements->erase(elements->begin() + elementIndex);
  if(hasObject) {
    fireEvent([status, &element](auto const& listener) {
      listener->itemRemoved(status, element.object());
    });
  }
}

void ProgrammeStore::updateElement(const communication::ConnectionId& id,
                                   const proto::Object& element) {
  auto programmeIndex = store_.selected_programme_index();
  auto elements = store_.mutable_programme(programmeIndex)->mutable_element();

  auto it =
      std::find_if(elements->begin(), elements->end(), [&id](auto const& element) {
        if(element.has_object()) {
          communication::ConnectionId elementId(element.object().connection_id());
          return id == elementId;
        }
        return false;
      });

  if (it != elements->end()) {
    *(it->mutable_object()) = element;
    ProgrammeStatus status{
        programmeIndex,
        true
    };
    fireEvent([status, &element](auto const& listener) {
      listener->itemUpdated(status, element);
    });
  }
}

void ProgrammeStore::removeElementFromProgramme(int programmeIndex, const communication::ConnectionId& id) {
  auto elements =
      store_.mutable_programme(programmeIndex)->mutable_element();
  auto element =
      std::find_if(elements->begin(), elements->end(), [id](auto const& entry) {
        return communication::ConnectionId(entry.object().connection_id()) ==
               id;
      });
  if (element != elements->end()) {
    auto elementIndex = static_cast<int>(std::distance(elements->begin(), element));
    removeElementFromProgramme(programmeIndex, elementIndex);
  }
}

//void ProgrammeStore::removeElementFromSelectedProgramme(const communication::ConnectionId& id) {
//  removeElementFromProgramme(store_.selected_programme_index(), id);
//}

void ProgrammeStore::removeElementFromAllProgrammes(const communication::ConnectionId& id) {
  for (int programmeIndex = 0;
       programmeIndex != store_.programme_size();
       ++programmeIndex) {
    removeElementFromProgramme(programmeIndex, id);
  }
}

void ProgrammeStore::moveElement(int programmeIndex, int oldIndex,
                                 int newIndex) {
  auto elements =
      store_.mutable_programme(programmeIndex)
          ->mutable_element();
  if (oldIndex >= 0 &&  //
      newIndex >= 0 &&  //
      oldIndex < elements->size() &&  //
      newIndex < elements->size() &&  //
      oldIndex != newIndex) {
    move(elements->begin(), oldIndex, newIndex);
    auto programme = store_.programme(programmeIndex);
    fireEvent([programmeIndex, &programme](auto const& listener) {
      listener->programmeUpdated(programmeIndex, programme);
    });
  }
}

void ProgrammeStore::autoUpdateFrom(const ItemStore& itemStore) {
  if (store_.auto_mode()) {
    store_.clear_programme();
    auto defaultProgramme = addProgrammeImpl("Default", "");
    store_.set_selected_programme_index(0);
    auto itemsSortedByRoute = itemStore.routeMap();
    for (auto const& routeItem : itemsSortedByRoute) {
      addObject(defaultProgramme, routeItem.second);
    }
    auto index = store_.selected_programme_index();
    auto const prog = *defaultProgramme;
    fireEvent([index, &prog](auto const& listener) {
      listener->programmeUpdated(index, prog);
    });
  }
}

void ProgrammeStore::addListener(
    const std::shared_ptr<ProgrammeStoreListener>& listener) {
  listeners_.push_back(listener);
}
std::optional<proto::Programme> ProgrammeStore::programmeAtIndexImpl(
    int index) const {
  std::optional<proto::Programme> programme;
  if(index >= 0 && index < store_.programme_size()) {
    programme = store_.programme(index);
  }
  return programme;
}

proto::Programme* ProgrammeStore::addProgrammeImpl(
    const std::string& name, const std::string& language) {
  auto programme = store_.add_programme();
  programme->set_name(name);
  programme->set_language(language);
  return programme;
}

proto::Object* ProgrammeStore::addObject(proto::Programme* programme,
                                         const communication::ConnectionId id) {
  auto element = programme->add_element();
  auto object = new proto::Object{};
  object->set_connection_id(id.string());
  element->set_allocated_object(object);
  return object;
}
