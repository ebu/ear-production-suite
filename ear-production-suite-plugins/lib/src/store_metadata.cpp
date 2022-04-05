//
// Created by Richard Bailey on 01/03/2022.
//

#include "store_metadata.hpp"
#include <google/protobuf/util/message_differencer.h>
#include "helper/move.hpp"
#include <algorithm>

using namespace ear::plugin;

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

void Metadata::set(proto::ProgrammeStore const& store) {
    std::lock_guard<std::mutex> lock(mutex_);
    store_ = store;
    doChangeStore();
}

void Metadata::addProgramme() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string name{"Programme_"};
    auto index = store_.programme_size();
    name.append(std::to_string(index));
    auto programme = addProgrammeImpl(name, "");
    bool selected = (index == store_.selected_programme_index());

    doAddProgramme({index, selected}, *programme);
}

void Metadata::removeProgramme(int index) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto programme = store_.mutable_programme();
    assert(index < store_.programme_size());
    programme->erase(programme->begin() + index);
    auto selected_index = store_.selected_programme_index();
    doRemoveProgramme(index);

    if(selected_index >= programme->size()) {
        auto newIndex = std::max<int>(programme->size() - 1, 0);
        store_.set_selected_programme_index(newIndex);
        auto prog = store_.programme(newIndex);
        doSelectProgramme(newIndex, prog);
    }
}

void Metadata::moveProgramme(int oldIndex, int newIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto programmes = store_.mutable_programme();
    auto size = programmes->size();
    if (oldIndex >= 0 && newIndex >= 0 && oldIndex < size && newIndex < size &&
        oldIndex != newIndex) {
        move(programmes->begin(), oldIndex, newIndex);
        auto programme = programmes->at(newIndex);
        doMoveProgramme({oldIndex, newIndex}, programme);
    }
}

void Metadata::selectProgramme(int index) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto oldIndex = store_.selected_programme_index();
    if(oldIndex != index) {
        store_.set_selected_programme_index(index);
        if(index >= 0 && index < store_.programme_size()) {
            auto prog = store_.programme(index);
            doSelectProgramme(index, prog);
        }
    }
}

void Metadata::setAutoMode(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto old = store_.auto_mode();
    if(old != enable) {
        store_.set_auto_mode(enable);
        doSetAutoMode(enable);
    }
}

void Metadata::setProgrammeName(int index, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    assert(index < store_.programme_size());
    if(name != store_.programme(index).name()) {
        store_.mutable_programme(index)->set_name(name);
        auto prog = store_.programme(index);
        auto selectedIndex = store_.selected_programme_index();
        doUpdateProgramme(index, prog);
    }
}

void Metadata::setProgrammeLanguage(int programmeIndex,
                                    const std::string& language) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(!(store_.programme(programmeIndex).has_language() &&
         store_.programme(programmeIndex).language() == language)) {
        store_.mutable_programme(programmeIndex)->set_language(language);
        auto prog = store_.programme(programmeIndex);
        doUpdateProgramme(programmeIndex, prog);
    }
}

void Metadata::clearProgrammeLanguage(int programmeIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(store_.programme(programmeIndex).has_language()) {
        store_.mutable_programme(programmeIndex)->clear_language();
        auto prog = store_.programme(programmeIndex);
        doUpdateProgramme(programmeIndex, prog);
    }
}

void Metadata::addItemsToSelectedProgramme(std::vector<
        communication::ConnectionId> const& ids) {
    std::lock_guard<std::mutex> lock(mutex_);
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

    doAddItems({programmeIndex, true}, elements);
}

void Metadata::removeElementFromProgramme(int programmeIndex, const communication::ConnectionId& id) {
    std::lock_guard<std::mutex> lock(mutex_);
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


void Metadata::moveElement(int programmeIndex, int oldIndex,
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
        auto programme = store_.programme(programmeIndex);
        doUpdateProgramme(programmeIndex, programme);
    }
}

void Metadata::updateElement(const communication::ConnectionId& id,
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
        ProgrammeStatus status{
                programmeIndex,
                true
        };
        doUpdateItem(status, element);
    }
}

void Metadata::autoUpdateFrom(const RouteMap& itemsSortedByRoute) {
    if (store_.auto_mode()) {
        auto programmeCount = store_.programme_size();
        store_.clear_programme();
        for(auto i = 0; i != programmeCount; ++i) {
            doRemoveProgramme(i);
        }
        auto defaultProgramme = addProgrammeImpl("Default", "");
        store_.set_selected_programme_index(0);
        doAddProgramme({0, true}, *defaultProgramme);
        for (auto const& routeItem : itemsSortedByRoute) {
            auto object = addObject(defaultProgramme, routeItem.second);
            doAddItems({0, true}, {*object});
        }
        auto index = store_.selected_programme_index();
        auto const prog = *defaultProgramme;
        doUpdateProgramme(index, prog);
    }
}

proto::ProgrammeStore const& Metadata::get() const {
    return store_;
}

void Metadata::removeElementFromAllProgrammes(const communication::ConnectionId& id) {
    for (int programmeIndex = 0;
         programmeIndex != store_.programme_size();
         ++programmeIndex) {
        removeElementFromProgramme(programmeIndex, id);
    }
}

void Metadata::removeElementFromProgramme(int programmeIndex, int elementIndex) {
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
        doRemoveItem(status, element.object());
    }
}

proto::Programme* Metadata::addProgrammeImpl(
        const std::string& name, const std::string& language) {
    auto programme = store_.add_programme();
    programme->set_name(name);
    programme->set_language(language);
    return programme;
}

proto::Object* Metadata::addObject(proto::Programme* programme,
                                   const communication::ConnectionId id) {
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

void Metadata::doChangeStore() {
  auto items = itemStore_;
  auto store = store_;
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
  autoUpdateFrom(routeMap());
  auto selectedIndex = store_.selected_programme_index();
  for(auto i = 0; i != store_.programme_size(); ++i) {
    auto const& programme = store_.programme(i);
    auto const& elements = programme.element();
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
  removeElementFromAllProgrammes(oldItem.connection_id());
  autoUpdateFrom(routeMap());
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
