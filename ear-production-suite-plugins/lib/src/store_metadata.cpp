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
                  InputItem{item.connection_id(), item});
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

void Metadata::removeProgramme(const std::string &progId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto programmes = programmeStore_.mutable_programme();
    auto origIndex = getProgrammeIndex(progId);
    if(origIndex >= 0) {
      programmes->erase(programmes->begin() + origIndex);
      auto newSelectedId = programmeStore_.selected_programme_internal_id();
      auto newSelectedIndex =  getProgrammeIndex(newSelectedId);
      if(programmeStore_.selected_programme_internal_id() == progId) {
        newSelectedIndex = std::max<int>(programmeStore_.programme_size() - 1, origIndex);
        assert(newSelectedIndex >= 0); // should always have >0 progs
        newSelectedId = programmes->at(newSelectedIndex).programme_internal_id();
      }
      programmeStore_.set_selected_programme_internal_id(newSelectedId);
      fireEvent(&MetadataListener::notifyProgrammeRemoved,
                ProgrammeStatus{ origIndex, progId, false });
      auto prog = programmeStore_.programme(newSelectedIndex);
      doSelectProgramme(prog);
    }
}

void Metadata::moveProgramme(const std::string &progId, int newIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto oldIndex = getProgrammeIndex(progId);
    if(oldIndex >= 0) {
      auto programmes = programmeStore_.mutable_programme();
      auto size = programmes->size();
      if(oldIndex >= 0 && newIndex >= 0 && oldIndex < size && newIndex < size &&
         oldIndex != newIndex) {
        move(programmes->begin(), oldIndex, newIndex);
        auto programme = programmes->at(newIndex);
        fireEvent(&MetadataListener::notifyProgrammeMoved,
                  ProgrammeStatus{ newIndex, progId, progId == programmeStore_.selected_programme_internal_id() },
                  Movement{ oldIndex, newIndex }, programme);
      }
    }
}

void Metadata::selectProgramme(const std::string &progId) {
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

void Metadata::setProgrammeName(const std::string &progId, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto index = getProgrammeIndex(progId);
    if(index >= 0 && name != programmeStore_.programme(index).name()) {
        programmeStore_.mutable_programme(index)->set_name(name);
        auto prog = programmeStore_.programme(index);
        fireEvent(&MetadataListener::notifyProgrammeUpdated,
                  ProgrammeStatus{index, progId, progId == programmeStore_.selected_programme_internal_id()}, prog);
    }
}

void Metadata::setProgrammeLanguage(const std::string &progId,
                                    const std::string& language) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto index = getProgrammeIndex(progId);
    if(index >= 0 && !(programmeStore_.programme(index).has_language() &&
                       programmeStore_.programme(index).language() == language)) {
      programmeStore_.mutable_programme(index)->set_language(language);
      auto prog = programmeStore_.programme(index);
      fireEvent(&MetadataListener::notifyProgrammeUpdated,
                ProgrammeStatus{index, progId, progId == programmeStore_.selected_programme_internal_id()}, prog);

    }
}

void Metadata::clearProgrammeLanguage(const std::string &progId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto index = getProgrammeIndex(progId);
    if(index >= 0 && programmeStore_.programme(index).has_language()) {
      programmeStore_.mutable_programme(index)->clear_language();
      auto prog = programmeStore_.programme(index);
      fireEvent(&MetadataListener::notifyProgrammeUpdated,
                ProgrammeStatus{index, progId, progId == programmeStore_.selected_programme_internal_id()}, prog);

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
        std::vector<proto::Object> elements;
        elements.reserve(connIds.size());
        for(auto const& connId : connIds) {
          if(auto element = std::find_if(elements.begin(), elements.end(),
                                         [&connId](auto const& checkElement) {
            return checkElement.connection_id() == connId.string(); });
             element == elements.end()) {
            auto object = addObject(programme, connId);
              elements.push_back(*object);
          }
        }
      doAddItems({ programmeIndex, programmeStore_.programme(programmeIndex).programme_internal_id(), true }, elements);
    }
}

void Metadata::removeElementFromProgramme(const std::string &progId, const communication::ConnectionId& connId) {
    auto programmeIndex = getProgrammeIndex(progId);
    assert(programmeIndex >= 0);
    std::lock_guard<std::mutex> lock(mutex_);
    doRemoveElementFromProgramme(programmeIndex, connId);
}

void Metadata::moveElement(const std::string &progId, int oldIndex,
                                 int newIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto programmeIndex = getProgrammeIndex(progId);
    assert(programmeIndex >= 0);
    auto elements =
            programmeStore_.mutable_programme(programmeIndex)
                    ->mutable_element();
    if (oldIndex >= 0 &&  //
        newIndex >= 0 &&  //
        oldIndex < elements->size() &&  //
        newIndex < elements->size() &&  //
        oldIndex != newIndex) {
        move(elements->begin(), oldIndex, newIndex);
        auto programme = programmeStore_.programme(programmeIndex);
        fireEvent(&MetadataListener::notifyProgrammeUpdated,
                  ProgrammeStatus{programmeIndex, progId, progId == programmeStore_.selected_programme_internal_id()}, programme);
    }
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
        ProgrammeStatus status{
                programmeIndex,
                programmeStore_.programme(programmeIndex).programme_internal_id(),
                true
        };

        auto const& item = itemStore_.at(connId);
        fireEvent(&MetadataListener::notifyProgrammeItemUpdated,
                  status, ProgrammeObject{ element, item });
      }
    }
}

int ear::plugin::Metadata::getSelectedProgrammeIndex()
{
  std::lock_guard<std::mutex> lock(mutex_);
  return getProgrammeIndex(programmeStore_.selected_programme_internal_id());
}

std::string ear::plugin::Metadata::getSelectedProgrammeId()
{
  std::lock_guard<std::mutex> lock(mutex_);
  return programmeStore_.selected_programme_internal_id();
}

bool ear::plugin::Metadata::programmeHasElement(const std::string &progId, communication::ConnectionId const & connId)
{
  std::lock_guard<std::mutex> lock(mutex_);
  auto programmeIndex = getProgrammeIndex(progId);
  assert(programmeIndex >= 0);
  auto elements = programmeStore_.mutable_programme(programmeIndex)->mutable_element();
  return findObjectWithId(elements->begin(), elements->end(), connId) != elements->end();
}

bool ear::plugin::Metadata::getAutoMode()
{
  return programmeStore_.auto_mode();
}

int ear::plugin::Metadata::getProgrammeIndex(const std::string& progId)
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
            programmeIndex,
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
    auto index = programmeStore_.programme_size();
    auto programme = programmeStore_.add_programme();
    programme->set_name(name);
    programme->set_language(language);
    auto progId = newProgrammeInternalId();
    programme->set_programme_internal_id(progId);
    fireEvent(&MetadataListener::notifyProgrammeAdded,
              ProgrammeStatus{index, progId, false}, *programme);
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
  auto index = getProgrammeIndex(programme.programme_internal_id());
  ProgrammeObjects objects({index, programme.programme_internal_id(), true}, programme, itemStore_);
  fireEvent(&MetadataListener::notifyProgrammeSelected,
            objects);
}

void Metadata::doChangeInputItem(
    const proto::InputItemMetadata& oldItem,
    const proto::InputItemMetadata& newItem) {
  auto const& id = newItem.connection_id();
  fireEvent(&MetadataListener::notifyInputUpdated,
          InputItem{id, newItem}, oldItem);
  auto selectedIndex = getProgrammeIndex(programmeStore_.selected_programme_internal_id());

  for (auto i = 0; i != programmeStore_.programme_size(); ++i) {
      auto const &elements = programmeStore_.programme(i).element();
      if (auto it = findObjectWithId(elements.begin(), elements.end(), id);
              it != elements.end()) {
          fireEvent(&MetadataListener::notifyProgrammeItemUpdated,
                    ProgrammeStatus{i, programmeStore_.programme(i).programme_internal_id(), i == selectedIndex},
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

void Metadata::setElementOrder(const std::string &progId, const std::vector<communication::ConnectionId> &order) {
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
              ProgrammeStatus{programmeIndex, programmeStore_.programme(programmeIndex).programme_internal_id(), programmeIndex == selectedIndex},
              programmeStore_.programme(programmeIndex));
}

void Metadata::setExporting(bool exporting) {
    fireEvent(&MetadataListener::notifyExporting,
              exporting);
}

