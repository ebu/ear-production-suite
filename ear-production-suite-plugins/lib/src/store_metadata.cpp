//
// Created by Richard Bailey on 01/03/2022.
//

#include "store_metadata.hpp"
#include "helper/move.hpp"
#include <algorithm>

using namespace ear::plugin;

namespace {
    template<typename ItT>
    auto findObjectWithId(ItT begin, ItT end, communication::ConnectionId const &id) {
        return std::find_if(begin, end, [&id](auto const &element) {
            return element.has_object() && element.object().connection_id() == id.string();
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
        const communication::ConnectionId& id,
        const proto::InputItemMetadata& item) {
    std::lock_guard<std::mutex> lock(mutex_);

    assert(id.string() == item.connection_id());
    if (auto result = itemStore_.emplace(id, item); result.second) {
        EAR_LOGGER_TRACE(logger_, "addItem id {}", item.connection_id());
        fireEvent(&MetadataListener::notifyInputAdded,
                  InputItem{item.connection_id(), item});
    } else {
        auto previousItem = result.first->second;
        result.first->second = item;
        doChangeInputItem(previousItem, item);
    }
    itemStore_[id].set_changed(false);
}

void Metadata::removeInput(const communication::ConnectionId& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(auto it = itemStore_.find(id); it != itemStore_.end()) {
        auto item = it->second;
        removeElementFromAllProgrammes(id);
        itemStore_.erase(it);
        fireEvent(&MetadataListener::notifyInputRemoved,
                  id);
    }
}

void Metadata::setStore(proto::ProgrammeStore const& store) {
    std::lock_guard<std::mutex> lock(mutex_);
    programmeStore_ = store;
    auto items = itemStore_;
    fireEvent(&MetadataListener::notifyDataReset,
              store, items);
}

void Metadata::addProgramme() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string name{"Programme_"};
    auto index = programmeStore_.programme_size();
    name.append(std::to_string(index));
    addProgrammeImpl(name, "");
}

void Metadata::removeProgramme(int index) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto programme = programmeStore_.mutable_programme();
    assert(index < programmeStore_.programme_size());
    programme->erase(programme->begin() + index);
    auto newIndex = programmeStore_.selected_programme_index();
    if(newIndex >= programme->size()) {
        newIndex = std::max<int>(programme->size() - 1, 0);
    }
    programmeStore_.set_selected_programme_index(newIndex);
    fireEvent(&MetadataListener::notifyProgrammeRemoved,
              index);
    auto prog = programmeStore_.programme(newIndex);
    doSelectProgramme(newIndex, prog);
}

void Metadata::moveProgramme(int oldIndex, int newIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto programmes = programmeStore_.mutable_programme();
    auto size = programmes->size();
    if (oldIndex >= 0 && newIndex >= 0 && oldIndex < size && newIndex < size &&
        oldIndex != newIndex) {
        move(programmes->begin(), oldIndex, newIndex);
        auto programme = programmes->at(newIndex);
        fireEvent(&MetadataListener::notifyProgrammeMoved,
                  Movement{oldIndex, newIndex}, programme);
    }
}

void Metadata::selectProgramme(int index) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto oldIndex = programmeStore_.selected_programme_index();
    if(oldIndex != index) {
        programmeStore_.set_selected_programme_index(index);
        if(index >= 0 && index < programmeStore_.programme_size()) {
            auto prog = programmeStore_.programme(index);
            doSelectProgramme(index, prog);
        }
    }
}

void Metadata::setAutoMode(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    programmeStore_.set_auto_mode(enable);
    fireEvent(&MetadataListener::notifyAutoModeChanged,
              enable);
}

void Metadata::setProgrammeName(int index, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    assert(index < programmeStore_.programme_size());
    if(name != programmeStore_.programme(index).name()) {
        programmeStore_.mutable_programme(index)->set_name(name);
        auto prog = programmeStore_.programme(index);
        auto selectedIndex = programmeStore_.selected_programme_index();
        fireEvent(&MetadataListener::notifyProgrammeUpdated,
                  ProgrammeStatus{index, index == selectedIndex}, prog);
    }
}

void Metadata::setProgrammeLanguage(int programmeIndex,
                                    const std::string& language) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(!(programmeStore_.programme(programmeIndex).has_language() &&
         programmeStore_.programme(programmeIndex).language() == language)) {
        programmeStore_.mutable_programme(programmeIndex)->set_language(language);
        auto prog = programmeStore_.programme(programmeIndex);
        auto selectedIndex = programmeStore_.selected_programme_index();
        fireEvent(&MetadataListener::notifyProgrammeUpdated,
                  ProgrammeStatus{programmeIndex, programmeIndex == selectedIndex}, prog);
    }
}

void Metadata::clearProgrammeLanguage(int programmeIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(programmeStore_.programme(programmeIndex).has_language()) {
        programmeStore_.mutable_programme(programmeIndex)->clear_language();
        auto prog = programmeStore_.programme(programmeIndex);
        auto selectedIndex = programmeStore_.selected_programme_index();
        fireEvent(&MetadataListener::notifyProgrammeUpdated,
                  ProgrammeStatus{programmeIndex, programmeIndex == selectedIndex}, prog);
    }
}

void Metadata::addItemsToSelectedProgramme(std::vector<
        communication::ConnectionId> const& ids) {
    std::lock_guard<std::mutex> lock(mutex_);
    doAddItemsToSelectedProgramme(ids);
}

void Metadata::doAddItemsToSelectedProgramme(std::vector<communication::ConnectionId> const& ids) {
    assert(!ids.empty());
    auto programmeIndex = programmeStore_.selected_programme_index();
    auto programme = programmeStore_.mutable_programme(programmeIndex);
    std::vector<proto::Object> elements;
    elements.reserve(ids.size());
    for(auto const& id : ids) {
        if( auto element = std::find_if(elements.begin(), elements.end(),
                                        [&id](auto const& checkElement) {
            return checkElement.connection_id() == id.string(); });
            element == elements.end()) {
                auto object = addObject(programme, id);
                elements.push_back(*object);
        }
    }
    doAddItems({programmeIndex, true}, elements);
}

void Metadata::removeElementFromProgramme(int programmeIndex, const communication::ConnectionId& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    doRemoveElementFromProgramme(programmeIndex, id);
}

void Metadata::moveElement(int programmeIndex, int oldIndex,
                                 int newIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
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
        auto selectedIndex = programmeStore_.selected_programme_index();
        fireEvent(&MetadataListener::notifyProgrammeUpdated,
                  ProgrammeStatus{programmeIndex, programmeIndex == selectedIndex}, programme);
    }
}

void Metadata::updateElement(const communication::ConnectionId& id,
                                   const proto::Object& element) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto programmeIndex = programmeStore_.selected_programme_index();
    auto elements = programmeStore_.mutable_programme(programmeIndex)->mutable_element();

    if(auto it = findObjectWithId(elements->begin(), elements->end(), id);
       it != elements->end() && it->has_object()) {
        *(it->mutable_object()) = element;
        ProgrammeStatus status{
                programmeIndex,
                true
        };

        auto const& item = itemStore_.at(id);
        fireEvent(&MetadataListener::notifyProgrammeItemUpdated,
                  status, ProgrammeObject{element, item});
    }
}

void ear::plugin::Metadata::sortProgrammeElements(int programmeIndex)
{
    /////////////////////// TODO
}

int ear::plugin::Metadata::getSelectedProgrammeIndex()
{
  return programmeStore_.selected_programme_index();
}

bool ear::plugin::Metadata::programmeHasElement(int programmeIndex, communication::ConnectionId const & id)
{
  std::lock_guard<std::mutex> lock(mutex_);
  auto elements = programmeStore_.mutable_programme(programmeIndex)->mutable_element();
  return findObjectWithId(elements->begin(), elements->end(), id) != elements->end();
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
        programmeStore_.set_selected_programme_index(0);
        doSelectProgramme(0, programmeStore_.programme(0));
    }
}

void Metadata::removeElementFromAllProgrammes(const communication::ConnectionId& id) {
    for (int programmeIndex = 0;
         programmeIndex != programmeStore_.programme_size();
         ++programmeIndex) {
        doRemoveElementFromProgramme(programmeIndex, id);
    }
}

void Metadata::doRemoveElementFromProgramme(int programmeIndex, const communication::ConnectionId& id) {
    auto elements =
            programmeStore_.mutable_programme(programmeIndex)->mutable_element();

    if(auto it = findObjectWithId(elements->begin(), elements->end(), id); it != elements->end()) {
        ProgrammeStatus status {
            programmeIndex,
            programmeIndex == programmeStore_.selected_programme_index()
        };
        elements->erase(it);
        EAR_LOGGER_TRACE(logger_, "remove programme item id {}", id.string());
        fireEvent(&MetadataListener::notifyItemRemovedFromProgramme,
                  status, id);
    }
}

void Metadata::addProgrammeImpl(
        const std::string& name,
        const std::string& language) {
    auto index = programmeStore_.programme_size();
    auto programme = programmeStore_.add_programme();
    programme->set_name(name);
    programme->set_language(language);
    fireEvent(&MetadataListener::notifyProgrammeAdded,
              ProgrammeStatus{index, false}, *programme);
}

proto::Object* Metadata::addObject(proto::Programme* programme,
                                   const communication::ConnectionId& id) {
  auto element = programme->add_element();
  auto object = new proto::Object{};
  object->set_connection_id(id.string());
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
    auto id = communication::ConnectionId(item.connection_id());
    pairs.push_back({item, itemStore_.at(id)});
  }

  fireEvent(&MetadataListener::notifyItemsAddedToProgramme,
            status, pairs);
}

void Metadata::doSelectProgramme(int index, proto::Programme const& programme) {
  ProgrammeObjects objects({index, true}, programme, itemStore_);
  fireEvent(&MetadataListener::notifyProgrammeSelected,
            objects);
}

void Metadata::doChangeInputItem(
    const proto::InputItemMetadata& oldItem,
    const proto::InputItemMetadata& newItem) {
  auto const& id = newItem.connection_id();
  fireEvent(&MetadataListener::notifyInputUpdated,
          InputItem{id, newItem}, oldItem);
  auto selectedIndex = programmeStore_.selected_programme_index();

  for (auto i = 0; i != programmeStore_.programme_size(); ++i) {
      auto const &elements = programmeStore_.programme(i).element();
      if (auto it = findObjectWithId(elements.begin(), elements.end(), id);
              it != elements.end()) {
          fireEvent(&MetadataListener::notifyProgrammeItemUpdated,
                    ProgrammeStatus{i, i == selectedIndex},
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

void Metadata::setElementOrder(int programmeIndex, const std::vector<communication::ConnectionId> &order) {
    std::lock_guard<std::mutex> lock(mutex_);
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

    auto selectedIndex = programmeStore_.selected_programme_index();
    fireEvent(&MetadataListener::notifyProgrammeUpdated,
              ProgrammeStatus{programmeIndex, programmeIndex == selectedIndex},
              programmeStore_.programme(programmeIndex));
}

void Metadata::setExporting(bool exporting) {
    fireEvent(&MetadataListener::notifyExporting,
              exporting);
}

