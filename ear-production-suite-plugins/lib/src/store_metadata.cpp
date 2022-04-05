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
    auto programme = addProgrammeImpl(name, "");
    bool selected = (index == programmeStore_.selected_programme_index());

    doAddProgramme({index, selected}, *programme);
}

void Metadata::removeProgramme(int index) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto programme = programmeStore_.mutable_programme();
    assert(index < programmeStore_.programme_size());
    programme->erase(programme->begin() + index);
    auto selected_index = programmeStore_.selected_programme_index();
    doRemoveProgramme(index);

    if(selected_index >= programme->size()) {
        auto newIndex = std::max<int>(programme->size() - 1, 0);
        programmeStore_.set_selected_programme_index(newIndex);
        auto prog = programmeStore_.programme(newIndex);
        doSelectProgramme(newIndex, prog);
    }
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
    auto old = programmeStore_.auto_mode();
    if(old != enable) {
        programmeStore_.set_auto_mode(enable);
        fireEvent(&MetadataListener::notifyAutoModeChanged,
                  enable);
    }
}

void Metadata::setProgrammeName(int index, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    assert(index < programmeStore_.programme_size());
    if(name != programmeStore_.programme(index).name()) {
        programmeStore_.mutable_programme(index)->set_name(name);
        auto prog = programmeStore_.programme(index);
        auto selectedIndex = programmeStore_.selected_programme_index();
        doUpdateProgramme(index, prog);
    }
}

void Metadata::setProgrammeLanguage(int programmeIndex,
                                    const std::string& language) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(!(programmeStore_.programme(programmeIndex).has_language() &&
         programmeStore_.programme(programmeIndex).language() == language)) {
        programmeStore_.mutable_programme(programmeIndex)->set_language(language);
        auto prog = programmeStore_.programme(programmeIndex);
        doUpdateProgramme(programmeIndex, prog);
    }
}

void Metadata::clearProgrammeLanguage(int programmeIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(programmeStore_.programme(programmeIndex).has_language()) {
        programmeStore_.mutable_programme(programmeIndex)->clear_language();
        auto prog = programmeStore_.programme(programmeIndex);
        doUpdateProgramme(programmeIndex, prog);
    }
}

void Metadata::addItemsToSelectedProgramme(std::vector<
        communication::ConnectionId> const& ids) {
    std::lock_guard<std::mutex> lock(mutex_);
    assert(!ids.empty());
    auto programmeIndex = 0;
    programmeIndex = programmeStore_.selected_programme_index();
    auto programme = programmeStore_.mutable_programme(programmeIndex);
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
            programmeStore_.programme(programmeIndex).element();
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
            programmeStore_.mutable_programme(programmeIndex)
                    ->mutable_element();
    if (oldIndex >= 0 &&  //
        newIndex >= 0 &&  //
        oldIndex < elements->size() &&  //
        newIndex < elements->size() &&  //
        oldIndex != newIndex) {
        move(elements->begin(), oldIndex, newIndex);
        auto programme = programmeStore_.programme(programmeIndex);
        doUpdateProgramme(programmeIndex, programme);
    }
}

void Metadata::updateElement(const communication::ConnectionId& id,
                                   const proto::Object& element) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto programmeIndex = programmeStore_.selected_programme_index();
    auto elements = programmeStore_.mutable_programme(programmeIndex)->mutable_element();

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

        auto const& item = itemStore_.at(id);
        fireEvent(&MetadataListener::notifyProgrammeItemUpdated,
                  status, ProgrammeObject{element, item});
    }
}

void Metadata::autoUpdateFrom(const RouteMap& itemsSortedByRoute) {
    if (programmeStore_.auto_mode()) {
        auto programmeCount = programmeStore_.programme_size();
        programmeStore_.clear_programme();
        for(auto i = 0; i != programmeCount; ++i) {
            doRemoveProgramme(i);
        }
        auto defaultProgramme = addProgrammeImpl("Default", "");
        programmeStore_.set_selected_programme_index(0);
        doAddProgramme({0, true}, *defaultProgramme);
        for (auto const& routeItem : itemsSortedByRoute) {
            auto object = addObject(defaultProgramme, routeItem.second);
            doAddItems({0, true}, {*object});
        }
        auto index = programmeStore_.selected_programme_index();
        auto const prog = *defaultProgramme;
        doUpdateProgramme(index, prog);
    }
}

void Metadata::removeElementFromAllProgrammes(const communication::ConnectionId& id) {
    for (int programmeIndex = 0;
         programmeIndex != programmeStore_.programme_size();
         ++programmeIndex) {
        removeElementFromProgramme(programmeIndex, id);
    }
}

void Metadata::removeElementFromProgramme(int programmeIndex, int elementIndex) {
    assert(programmeIndex >=0 && programmeIndex < programmeStore_.programme_size());
    auto elements =
            programmeStore_.mutable_programme(programmeIndex)
                    ->mutable_element();
    assert(elementIndex >= 0 && elementIndex < std::distance(elements->begin(), elements->end()));
    auto element = programmeStore_.programme(programmeIndex).element(elementIndex);
    ProgrammeStatus status{};
    bool hasObject{false};
    if(element.has_object()) {
        status.index = programmeIndex;
        status.isSelected = programmeIndex == programmeStore_.selected_programme_index();
        hasObject = true;
    }
    elements->erase(elements->begin() + elementIndex);
    if(hasObject) {
        auto id = element.object().connection_id();
        EAR_LOGGER_TRACE(logger_, "remove programme item id {}", id);
        fireEvent(&MetadataListener::notifyItemRemovedFromProgramme,
                  status, id);
    }
}

proto::Programme* Metadata::addProgrammeImpl(
        const std::string& name, const std::string& language) {
    auto programme = programmeStore_.add_programme();
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

void Metadata::doAddProgramme(
    ProgrammeStatus status, const proto::Programme& programme) {
  fireEvent(&MetadataListener::notifyProgrammeAdded,
            status.index, programme);
}

void Metadata::doRemoveProgramme(int index) {
  fireEvent(&MetadataListener::notifyProgrammeRemoved,
            index);
}

void Metadata::doSelectProgramme(int index, proto::Programme const& programme) {
  ProgrammeObjects objects({index, true}, programme, itemStore_);
  for(auto& object : objects) {
      object.inputMetadata.set_changed(true);
  }
  fireEvent(&MetadataListener::notifyProgrammeSelected,
            objects);
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
  auto selectedIndex = programmeStore_.selected_programme_index();
  for(auto i = 0; i != programmeStore_.programme_size(); ++i) {
    auto const& programme = programmeStore_.programme(i);
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
