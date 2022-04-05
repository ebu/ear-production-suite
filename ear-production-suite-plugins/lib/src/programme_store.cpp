//
// Created by Richard Bailey on 23/02/2022.
//

#include "../include/programme_store.hpp"
#include <algorithm>
#include "helper/move.hpp"
#include "helper/weak_ptr.hpp"
#include "store_metadata.hpp"

using namespace ear::plugin;

proto::ProgrammeStore const& ProgrammeStore::get() const {
  return store_;
}

std::optional<proto::Programme> ProgrammeStore::programmeAtIndex(
    int index) const {
  return programmeAtIndexImpl(index);
}

int ProgrammeStore::programmeCount() const {
  return store_.programme_size();
}

void ProgrammeStore::set(proto::ProgrammeStore const& store) {
  store_ = store;
  metadata_.changeStore(store_);
}

void ProgrammeStore::setAutoMode(bool enable) {
  auto old = store_.auto_mode();
  if(old != enable) {
    store_.set_auto_mode(enable);
    metadata_.setAutoMode(enable);
  }
}

void ProgrammeStore::selectProgramme(int index) {
  auto oldIndex = store_.selected_programme_index();
  if(oldIndex != index) {
    store_.set_selected_programme_index(index);
    if(index >= 0 && index < store_.programme_size()) {
      auto prog = store_.programme(index);
      metadata_.selectProgramme(index, prog);
    }
  }
}

void ProgrammeStore::addProgramme() {
  std::string name{"Programme_"};
  auto index = store_.programme_size();
  name.append(std::to_string(index));
  auto programme = addProgrammeImpl(name, "");
  bool selected = (index == store_.selected_programme_index());

  metadata_.addProgramme({index, selected}, *programme);
}

void ProgrammeStore::moveProgramme(int oldIndex, int newIndex) {
  auto programmes = store_.mutable_programme();
  auto size = programmes->size();
  if (oldIndex >= 0 && newIndex >= 0 && oldIndex < size && newIndex < size &&
      oldIndex != newIndex) {
    move(programmes->begin(), oldIndex, newIndex);
    auto programme = programmes->at(newIndex);
    metadata_.moveProgramme({oldIndex, newIndex}, programme);
  }
}

void ProgrammeStore::removeProgramme(int index) {
  auto programme = store_.mutable_programme();
  assert(index < store_.programme_size());
  programme->erase(programme->begin() + index);
  auto selected_index = store_.selected_programme_index();
  metadata_.removeProgramme(index);

  if(selected_index >= programme->size()) {
    auto newIndex = std::max<int>(programme->size() - 1, 0);
    store_.set_selected_programme_index(newIndex);
    auto prog = store_.programme(newIndex);
    metadata_.selectProgramme(newIndex, prog);
  }
}

void ProgrammeStore::setProgrammeName(int index, const std::string& name) {
  assert(index < store_.programme_size());
  if(name != store_.programme(index).name()) {
    store_.mutable_programme(index)->set_name(name);
    auto prog = store_.programme(index);
    auto selectedIndex = store_.selected_programme_index();
    metadata_.updateProgramme(index, prog);
  }
}

void ProgrammeStore::setProgrammeLanguage(int programmeIndex,
                                          const std::string& language) {
  if(!(store_.programme(programmeIndex).has_language() &&
        store_.programme(programmeIndex).language() == language)) {
    store_.mutable_programme(programmeIndex)->set_language(language);
    auto prog = store_.programme(programmeIndex);
    metadata_.updateProgramme(programmeIndex, prog);
  }
}

void ProgrammeStore::clearProgrammeLanguage(int programmeIndex) {
  if(store_.programme(programmeIndex).has_language()) {
    store_.mutable_programme(programmeIndex)->clear_language();
    auto prog = store_.programme(programmeIndex);
    metadata_.updateProgramme(programmeIndex, prog);
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

  metadata_.addItems({programmeIndex, true}, elements);
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
    status.isSelected = programmeIndex == store_.selected_programme_index();
    hasObject = true;
  }
  elements->erase(elements->begin() + elementIndex);
  if(hasObject) {
      metadata_.removeItem(status, element.object());
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
    metadata_.updateItem(status, element);
  }
}

void ProgrammeStore::removeElementFromProgramme(int programmeIndex, const communication::ConnectionId& id) {
  auto const& elements =
      store_.programme(programmeIndex).element();
  auto element =
      std::find_if(elements.begin(), elements.end(), [id](auto const& entry) {
        return communication::ConnectionId(entry.object().connection_id()) ==
               id;
      });
  if (element != elements.end()) {
    auto elementIndex = static_cast<int>(std::distance(elements.begin(), element));
    removeElementFromProgramme(programmeIndex, elementIndex);
  }
}

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
    metadata_.updateProgramme(programmeIndex, programme);
  }
}

void ProgrammeStore::autoUpdateFrom(const RouteMap& itemsSortedByRoute) {
  if (store_.auto_mode()) {
    auto programmeCount = store_.programme_size();
    store_.clear_programme();
    for(auto i = 0; i != programmeCount; ++i) {
        metadata_.removeProgramme(i);
    }
    auto defaultProgramme = addProgrammeImpl("Default", "");
    store_.set_selected_programme_index(0);
    metadata_.addProgramme({0, true}, *defaultProgramme);
    for (auto const& routeItem : itemsSortedByRoute) {
      auto object = addObject(defaultProgramme, routeItem.second);
      metadata_.addItems({0, true}, {*object});
    }
    auto index = store_.selected_programme_index();
    auto const prog = *defaultProgramme;
    metadata_.updateProgramme(index, prog);
  }
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
