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
  std::lock_guard<std::mutex> lock(mutex_);
  return store_;
}

std::optional<proto::Programme> ProgrammeStore::selectedProgramme() const {
  std::lock_guard<std::mutex> lock(mutex_);
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
  std::lock_guard<std::mutex> lock(mutex_);
  return programmeAtIndexImpl(index);
}

int ProgrammeStore::programmeCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return store_.programme_size();
}

std::string ProgrammeStore::programmeName(int index) const {
  std::lock_guard<std::mutex> lock(mutex_);
  assert(index < store_.programme_size());
  return store_.programme(index).name();
}

bool ProgrammeStore::autoModeEnabled() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return store_.auto_mode();
}

void ProgrammeStore::set(proto::ProgrammeStore store) {
  std::lock_guard<std::mutex> lock(mutex_);
  store_ = std::move(store);
  notifyStoreChanged(store_);
}

void ProgrammeStore::setAutoMode(bool enable) {
  std::lock_guard<std::mutex> lock(mutex_);
  store_.set_auto_mode(enable);
}

void ProgrammeStore::selectProgramme(int index) {
  std::lock_guard<std::mutex> lock(mutex_);
  store_.set_selected_programme_index(index);
}

std::pair<int, std::string> ProgrammeStore::addProgramme() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::string name{"Programme_"};
  auto index = store_.programme_size();
  name.append(std::to_string(index));
  auto programme = addProgrammeImpl(name, "");
  return {index, name};
}

bool ProgrammeStore::moveProgramme(int oldIndex, int newIndex) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto programmes = store_.mutable_programme();
  auto size = programmes->size();
  bool moved = false;
  if (oldIndex >= 0 && newIndex >= 0 && oldIndex < size && newIndex < size &&
      oldIndex != newIndex) {
    move(programmes->begin(), oldIndex, newIndex);
    moved = true;
  }
  return moved;
}

void ProgrammeStore::removeProgramme(int index) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto programme = store_.mutable_programme();
  assert(index < store_.programme_size());
  programme->erase(programme->begin() + index);
  auto selected_index = store_.selected_programme_index();
  if(selected_index >= programme->size()) {
    store_.set_selected_programme_index(std::max<int>(programme->size() - 1, 0));
  }
}

void ProgrammeStore::setProgrammeName(int index, const std::string& name) {
  std::lock_guard<std::mutex> lock(mutex_);
  assert(index < store_.programme_size());
  store_.mutable_programme(index)->set_name(name);
}

void ProgrammeStore::setProgrammeLanguage(int programmeIndex,
                                          const std::string& language) {
  std::lock_guard<std::mutex> lock(mutex_);
  store_.mutable_programme(programmeIndex)->set_language(language);
}

void ProgrammeStore::clearProgrammeLanguage(int programmeIndex) {
  std::lock_guard<std::mutex> lock(mutex_);
  store_.mutable_programme(programmeIndex)->clear_language();
}

std::pair<int, proto::Object> ProgrammeStore::addItemToSelectedProgramme(
    communication::ConnectionId const& id) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto programmeIndex =
      store_.selected_programme_index();
  auto programme =
      store_.mutable_programme(programmeIndex);
  return {programmeIndex, *addObject(programme, id)};
}

void ProgrammeStore::removeElement(int programmeIndex, int elementIndex) {
  std::lock_guard<std::mutex> lock(mutex_);
  assert(programmeIndex >=0 && programmeIndex < store_.programme_size());
  auto elements =
      store_.mutable_programme(programmeIndex)
          ->mutable_element();
  assert(elementIndex >= 0 && elementIndex < std::distance(elements->begin(), elements->end()));
  elements->erase(elements->begin() + elementIndex);
}

bool ProgrammeStore::updateElement(const communication::ConnectionId& id,
                                   const proto::Object& element) {
  std::lock_guard<std::mutex> lock(mutex_);
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
    return true;
  }
  return false;
}

std::vector<int> ProgrammeStore::removeFromAllProgrammes(
    const communication::ConnectionId& id) {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<int> changedProgrammeIndices;
  for (int programmeIndex = 0;
       programmeIndex != store_.programme_size();
       ++programmeIndex) {
    auto elements =
        store_.mutable_programme(programmeIndex)->mutable_element();
    auto element =
        std::find_if(elements->begin(), elements->end(), [id](auto const& entry) {
          return communication::ConnectionId(entry.object().connection_id()) ==
                 id;
        });
    if (element != elements->end()) {
      auto elementIndex = std::distance(elements->begin(), element);
      elements->erase(elements->begin() + elementIndex);
      changedProgrammeIndices.push_back(programmeIndex);
    }
  }
  return changedProgrammeIndices;
}

bool ProgrammeStore::moveElement(int programmeIndex, int oldIndex,
                                 int newIndex) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto elements =
      store_.mutable_programme(programmeIndex)
          ->mutable_element();
  if (oldIndex >= 0 &&  //
      newIndex >= 0 &&  //
      oldIndex < elements->size() &&  //
      newIndex < elements->size() &&  //
      oldIndex != newIndex) {
    move(elements->begin(), oldIndex, newIndex);
    return true;
  }
  return false;
}

void ProgrammeStore::autoUpdateFrom(const ItemStore& itemStore) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (store_.auto_mode()) {
    store_.clear_programme();
    auto defaultProgramme = addProgrammeImpl("Default", "");
    store_.set_selected_programme_index(0);
    auto itemsSortedByRoute = itemStore.routeMap();
    for (auto const& routeItem : itemsSortedByRoute) {
      addObject(defaultProgramme, routeItem.second);
    }
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
  notifyObjectAdded(*object);
  return object;
}

void ProgrammeStore::notifyObjectAdded(const proto::Object& object) {
  removeDeadPointersAfter(listeners_,
      [&object](auto const& listener) {
        listener->objectAdded(object);
      });
}

void ProgrammeStore::notifyStoreChanged(const proto::ProgrammeStore& store) {
  removeDeadPointersAfter(listeners_,
      [&store](auto const& listener) {
        listener->storeChanged(store);
      });
}
