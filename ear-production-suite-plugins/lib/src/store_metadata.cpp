//
// Created by Richard Bailey on 01/03/2022.
//

#include "store_metadata.hpp"
#include "helper/move.hpp"
#include "programme_internal_id.hpp"

#include <algorithm>

using namespace ear::plugin;

namespace {
    template<typename ItT>
    auto findObjectWithId(ItT begin, ItT end, communication::ConnectionId const &connId) {
        return std::find_if(begin, end, [&connId](auto const &element) {
            return element.has_object() && element.object().connection_id() == connId.string();
        });
    }
}

std::pair<proto::ProgrammeStore, ItemMap> Metadata::stores() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return{programmeStore_, itemStore_};
}

void Metadata::refresh() {
    std::lock_guard<std::mutex> lock(mutex_);
    fireEvent(&MetadataListener::notifyDataReset,
              programmeStore_,
              itemStore_);
    fireEvent(&MetadataListener::notifyDuplicateScene,
              isDuplicateScene_);
}

void Metadata::setDuplicateScene(bool isDuplicate) {
    std::lock_guard<std::mutex> lock(mutex_);
    isDuplicateScene_ = isDuplicate;
    fireEvent(&MetadataListener::notifyDuplicateScene,
              isDuplicateScene_);
}


void Metadata::setInputItemMetadata(
        const communication::ConnectionId& connId,
        const proto::InputItemMetadata& item) {
    std::lock_guard<std::mutex> lock(mutex_);

    assert(connId.string() == item.connection_id());
    if (auto result = itemStore_.emplace(connId, item); result.second) {
        EAR_LOGGER_TRACE(logger_, "addItem id {}", item.connection_id());
        fireEvent(&MetadataListener::notifyInputAdded,
                  InputItem{item.connection_id(), item}, programmeStore_.auto_mode());
    } else {
        auto previousItem = result.first->second;
        result.first->second = item;
        doChangeInputItem(previousItem, item);
    }
    itemStore_[connId].set_changed(false);
}

void Metadata::removeInput(const communication::ConnectionId& connId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(auto it = itemStore_.find(connId); it != itemStore_.end()) {
        auto item = it->second;
        removeElementFromAllProgrammes(connId);
        itemStore_.erase(it);
        fireEvent(&MetadataListener::notifyInputRemoved,
                  connId);
    }
}

void Metadata::setStore(proto::ProgrammeStore const& store) {
    std::lock_guard<std::mutex> lock(mutex_);
    programmeStore_ = store;
    for(int i = 0; i < programmeStore_.programme_size(); i++) {
      if(!programmeStore_.programme(i).has_programme_internal_id()) {
        auto id = newProgrammeInternalId();
        programmeStore_.mutable_programme(i)->set_programme_internal_id(id);
      }
    }
    if(!programmeStore_.has_selected_programme_internal_id() && programmeStore_.programme_size() > 0) {
      programmeStore_.set_selected_programme_internal_id(programmeStore_.programme(0).programme_internal_id());
    }
    auto items = itemStore_;
    fireEvent(&MetadataListener::notifyDataReset,
              programmeStore_, items);
}

void Metadata::addProgramme() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string name{"Programme_"};
    auto index = programmeStore_.programme_size();
    name.append(std::to_string(index));
    addProgrammeImpl(name, "");
}

void Metadata::removeProgramme(const ProgrammeInternalId &progId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto programmes = programmeStore_.mutable_programme();
    auto origIndex = getProgrammeIndex(progId);
    if(origIndex >= 0) {
      programmes->erase(programmes->begin() + origIndex);
      auto newSelectedId = programmeStore_.selected_programme_internal_id();
      auto newSelectedIndex =  getProgrammeIndex(newSelectedId);
      if(newSelectedIndex < 0) {
        // Programme we want to select no longer exists - pick a suitable alternative
        if(origIndex < programmeStore_.programme_size()) {
          newSelectedIndex = origIndex;
        } else {
          newSelectedIndex = programmeStore_.programme_size() - 1;
        }
        assert(newSelectedIndex >= 0); // should always have >0 progs
        newSelectedId = programmes->at(newSelectedIndex).programme_internal_id();
      }
      programmeStore_.set_selected_programme_internal_id(newSelectedId);
      fireEvent(&MetadataListener::notifyProgrammeRemoved,
                ProgrammeStatus{ progId, false });
      auto prog = programmeStore_.programme(newSelectedIndex);
      doSelectProgramme(prog);
    }
}

void Metadata::setProgrammeOrder(std::vector<ProgrammeInternalId> const & order)
{

  auto targetIndexOf = [=](proto::Programme const& prog, std::vector<ProgrammeInternalId> const& order) {
    ProgrammeInternalId progId = prog.programme_internal_id();
    for(int i = 0; i < order.size(); i++) {
      if(order[i] == progId) {
        return i;
      }
    }
    // Not found in new list - use existing order after the sorted list
    for(int i = 0; i < programmeStore_.programme_size(); i++) {
      if(programmeStore_.programme(i).programme_internal_id() == progId) {
        return (int)order.size() + i;
      }
    }
    // Still not found (can't happen, but complete return routes)- move to very end
    return (int)order.size() + programmeStore_.programme_size();
  };

  auto programmes = programmeStore_.mutable_programme();
  std::stable_sort(programmes->begin(), programmes->end(),
                   [&order, targetIndexOf](auto const& lhs, auto const& rhs) {
    return targetIndexOf(lhs, order) < targetIndexOf(rhs, order);
  });

  std::vector<ProgrammeStatus> programmeStatuses;
  auto selectedProgId = programmeStore_.selected_programme_internal_id();
  for(const auto& programme : programmeStore_.programme()) {
      auto progId = programme.programme_internal_id();
      programmeStatuses.push_back(ProgrammeStatus{ progId, selectedProgId == progId });
  }

  fireEvent(&MetadataListener::notifyProgrammeOrderChanged, programmeStatuses);
}

void Metadata::selectProgramme(const ProgrammeInternalId &progId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto index = getProgrammeIndex(progId);
    if(index >= 0 && programmeStore_.selected_programme_internal_id() != progId) {
      programmeStore_.set_selected_programme_internal_id(progId);
      auto index = getProgrammeIndex(progId);
      doSelectProgramme(programmeStore_.programme(index));
    }
}

void Metadata::setAutoMode(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    programmeStore_.set_auto_mode(enable);
    fireEvent(&MetadataListener::notifyAutoModeChanged,
              enable);
}

void Metadata::setProgrammeName(const ProgrammeInternalId &progId, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto index = getProgrammeIndex(progId);
    if(index >= 0 && name != programmeStore_.programme(index).name()) {
        programmeStore_.mutable_programme(index)->set_name(name);
        auto prog = programmeStore_.programme(index);
        fireEvent(&MetadataListener::notifyProgrammeUpdated,
                  ProgrammeStatus{progId, progId == programmeStore_.selected_programme_internal_id()}, prog);
    }
}

void Metadata::setProgrammeLanguage(const ProgrammeInternalId &progId,
                                    const std::string& language) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto index = getProgrammeIndex(progId);
    if(index >= 0 && !(programmeStore_.programme(index).has_language() &&
                       programmeStore_.programme(index).language() == language)) {
      programmeStore_.mutable_programme(index)->set_language(language);
      auto prog = programmeStore_.programme(index);
      fireEvent(&MetadataListener::notifyProgrammeUpdated,
                ProgrammeStatus{progId, progId == programmeStore_.selected_programme_internal_id()}, prog);

    }
}

void Metadata::clearProgrammeLanguage(const ProgrammeInternalId &progId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto index = getProgrammeIndex(progId);
    if(index >= 0 && programmeStore_.programme(index).has_language()) {
      programmeStore_.mutable_programme(index)->clear_language();
      auto prog = programmeStore_.programme(index);
      fireEvent(&MetadataListener::notifyProgrammeUpdated,
                ProgrammeStatus{progId, progId == programmeStore_.selected_programme_internal_id()}, prog);

    }
}

void Metadata::addItemsToSelectedProgramme(std::vector<
        communication::ConnectionId> const& connIds) {
    std::lock_guard<std::mutex> lock(mutex_);
    doAddItemsToSelectedProgramme(connIds);
}

void Metadata::doAddItemsToSelectedProgramme(std::vector<communication::ConnectionId> const& connIds) {
    assert(!connIds.empty());
    auto programmeIndex = getProgrammeIndex(programmeStore_.selected_programme_internal_id());
    if(programmeIndex >= 0) {
      auto programme = programmeStore_.mutable_programme(programmeIndex);
      std::vector<proto::Object> newElements;
      newElements.reserve(connIds.size());
      for(auto const& connId : connIds) {
        auto programmeElements = programme->element();
        auto element = std::find_if(programmeElements.begin(), programmeElements.end(),
                                    [&connId](auto const& checkElement) {
          return checkElement.has_object() && checkElement.object().connection_id() == connId.string();
        });
        if(element == programmeElements.end()) {
          auto object = addObject(programme, connId);
          newElements.push_back(*object);
        }
      }
      doAddItems({ programmeStore_.programme(programmeIndex).programme_internal_id(), true }, newElements);
    }
}

void Metadata::removeElementFromProgramme(const ProgrammeInternalId &progId, const communication::ConnectionId& connId) {
    auto programmeIndex = getProgrammeIndex(progId);
    assert(programmeIndex >= 0);
    std::lock_guard<std::mutex> lock(mutex_);
    doRemoveElementFromProgramme(programmeIndex, connId);
}

void Metadata::updateElement(const communication::ConnectionId& connId,
                                   const proto::Object& element) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto programmeIndex = getProgrammeIndex(programmeStore_.selected_programme_internal_id());
    if(programmeIndex >= 0) {
      auto elements = programmeStore_.mutable_programme(programmeIndex)->mutable_element();

      if(auto it = findObjectWithId(elements->begin(), elements->end(), connId);
         it != elements->end() && it->has_object()) {
        *(it->mutable_object()) = element;
        ProgrammeStatus status{ programmeStore_.programme(programmeIndex).programme_internal_id(), true };
        auto const& item = itemStore_.at(connId);
        fireEvent(&MetadataListener::notifyProgrammeItemUpdated,
                  status, ProgrammeObject{ element, item });
      }
    }
}

int ear::plugin::Metadata::getProgrammeIndex(const ProgrammeInternalId& progId)
{
  return getProgrammeIndexFromId(programmeStore_, progId);
}

void Metadata::addUIListener(std::weak_ptr<MetadataListener> listener) {
    std::lock_guard<std::mutex> lock(mutex_);
    uiListeners_.push_back(std::move(listener));
}

void Metadata::addBackendListener(std::weak_ptr<MetadataListener> listener) {
    backendListeners_.push_back(std::move(listener));
}

void Metadata::ensureDefaultProgrammePresent() {
    if(programmeStore_.programme_size() == 0) {
        addProgrammeImpl("Default", "");
        auto defProgId = programmeStore_.programme(0).programme_internal_id();
        programmeStore_.set_selected_programme_internal_id(defProgId);
        doSelectProgramme(programmeStore_.programme(0));
    }
}

void Metadata::removeElementFromAllProgrammes(const communication::ConnectionId& connId) {
    for (int programmeIndex = 0;
         programmeIndex != programmeStore_.programme_size();
         ++programmeIndex) {
        doRemoveElementFromProgramme(programmeIndex, connId);
    }
}

void Metadata::doRemoveElementFromProgramme(int programmeIndex, const communication::ConnectionId& connId) {
    auto elements =
            programmeStore_.mutable_programme(programmeIndex)->mutable_element();
    auto selectedIndex = getProgrammeIndex(programmeStore_.selected_programme_internal_id());
    if(auto it = findObjectWithId(elements->begin(), elements->end(), connId); it != elements->end()) {
        ProgrammeStatus status {
            programmeStore_.programme(programmeIndex).programme_internal_id(),
            programmeIndex == selectedIndex
        };
        elements->erase(it);
        EAR_LOGGER_TRACE(logger_, "remove programme item id {}", connId.string());
        fireEvent(&MetadataListener::notifyItemRemovedFromProgramme,
                  status, connId);
    }
}

void Metadata::addProgrammeImpl(
        const std::string& name,
        const std::string& language) {
    auto programme = programmeStore_.add_programme();
    programme->set_name(name);
    programme->set_language(language);
    auto progId = newProgrammeInternalId();
    programme->set_programme_internal_id(progId);
    fireEvent(&MetadataListener::notifyProgrammeAdded,
              ProgrammeStatus{progId, false}, *programme);
}

proto::Object* Metadata::addObject(proto::Programme* programme,
                                   const communication::ConnectionId& connId) {
  auto element = programme->add_element();
  auto object = new proto::Object{};
  object->set_connection_id(connId.string());
  element->set_allocated_object(object);
  return object;
}

void EventDispatcher::dispatchEvent(std::function<void()> event) {
  doDispatch(std::move(event));
}

void Metadata::doAddItems(ProgrammeStatus status,
    const std::vector<proto::Object>& items) {
  std::vector<ProgrammeObject> pairs;
  pairs.reserve(items.size());
  for(auto const& item : items) {
    auto connId = communication::ConnectionId(item.connection_id());
    pairs.push_back({item, itemStore_.at(connId)});
  }

  fireEvent(&MetadataListener::notifyItemsAddedToProgramme,
            status, pairs);
}

void Metadata::doSelectProgramme(proto::Programme const& programme) {
  ProgrammeObjects objects({programme.programme_internal_id(), true}, programme, itemStore_);
  fireEvent(&MetadataListener::notifyProgrammeSelected,
            objects);
}

void Metadata::doChangeInputItem(
    const proto::InputItemMetadata& oldItem,
    const proto::InputItemMetadata& newItem) {
  auto const& id = newItem.connection_id();
  fireEvent(&MetadataListener::notifyInputUpdated,
          InputItem{id, newItem}, oldItem);

  for (const auto& programme : programmeStore_.programme()) {
      auto const &elements = programme.element();
      if (auto it = findObjectWithId(elements.begin(), elements.end(), id);
              it != elements.end()) {
          fireEvent(&MetadataListener::notifyProgrammeItemUpdated,
                    ProgrammeStatus{programme.programme_internal_id(), programme.programme_internal_id() == programmeStore_.selected_programme_internal_id()},
                    ProgrammeObject{it->object(), newItem});
      }
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

void Metadata::setElementOrder(const ProgrammeInternalId &progId, const std::vector<communication::ConnectionId> &order) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto programmeIndex = getProgrammeIndex(progId);
    assert(programmeIndex >= 0);
    doSetElementOrder(programmeIndex, order);
}

// This is horribly inefficient algorithm wise (O(n^2)?).
// OTOH, order.size() is always <= 64 so a linear vector search probably still wins over a map. (it may fit in cache.)
// If this ends up profiling slow, try changing it.
// This should only be called rarely (on a routing change in auto mode or an element move in the GUI),
// so it's not on a hot path.
// An alternative would be so store element ordering separately to data in ProgrammeStore, so we could just modify that
// without having to do a lookup / search at all, and rebuild the gui whenever it changes.
// Probably a better option long term as would make sorting by some other key trivial (just have a sort key field
// and the ordering) - key only needed to indicate what was used in the gui.
void Metadata::doSetElementOrder(int programmeIndex, const std::vector<communication::ConnectionId> &order) {
    auto currentElements = programmeStore_.mutable_programme(programmeIndex)->mutable_element();
    auto targetIndexOf = [](proto::ProgrammeElement const& element, std::vector<communication::ConnectionId> const& order) {
        if(!element.has_object()) return order.size();
        std::size_t i = 0;
        for(; i != order.size(); ++i) {
            if(order[i].string() == element.object().connection_id()) return i;
        }
        return i;
    };

    std::stable_sort(currentElements->begin(), currentElements->end(),
                     [&order, targetIndexOf](auto const& lhs, auto const& rhs) {
                         return targetIndexOf(lhs, order) < targetIndexOf(rhs, order);
                     });

    auto selectedIndex = getProgrammeIndex(programmeStore_.selected_programme_internal_id());
    fireEvent(&MetadataListener::notifyProgrammeUpdated,
              ProgrammeStatus{programmeStore_.programme(programmeIndex).programme_internal_id(), programmeIndex == selectedIndex},
              programmeStore_.programme(programmeIndex));
}

void Metadata::setExporting(bool exporting) {
    fireEvent(&MetadataListener::notifyExporting,
              exporting);
}

