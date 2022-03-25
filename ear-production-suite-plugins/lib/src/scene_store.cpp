//
// Created by Richard Bailey on 24/03/2022.
//

#include "../include/scene_store.hpp"
#include "routing_overlap.hpp"
#include <algorithm>
using namespace ear::plugin;
SceneStore::SceneStore(std::function<void(proto::SceneStore const&)> update) :
    updateCallback_{std::move(update)} {
}

void SceneStore::dataReset(const ear::plugin::proto::ProgrammeStore &programmes,
                                        const ear::plugin::ItemMap &items) {
    std::lock_guard<std::mutex> lock(mutex_);
    store_ = {};
    addAvailableInputItemsToSceneStore(items);
    auto selectedIndex = programmes.selected_programme_index();
    if(selectedIndex >= 0) {
        assert(selectedIndex < programmes.programme_size());
        auto const& selectedProgramme = programmes.programme(selectedIndex);
        for (auto const& element : selectedProgramme.element()) {
            if(element.has_object()) {
                auto const& object = element.object();
                auto const& id = object.connection_id();
                auto const& itemLocation = items.find(id);
                if(itemLocation != items.end()) {
                    addMonitoringItem(itemLocation->second);
                }
            }
        }
    }
    auto overlaps = getOverlapIds(store_);
    if (overlappingIds_ != overlaps) {
        flagChangedOverlaps(overlappingIds_, overlaps, store_);
    }
    overlappingIds_ = overlaps;

    sendUpdate();
}

void ear::plugin::SceneStore::programmeSelected(const ear::plugin::ProgrammeObjects &objects) {
    std::lock_guard<std::mutex> lock(mutex_);
    store_.clear_monitoring_items();
    for(auto const& object : objects) {
        addMonitoringItem(object.inputMetadata);
    }
    sendUpdate();
}

void ear::plugin::SceneStore::itemsAddedToProgramme(ear::plugin::ProgrammeStatus status,
                                                    const std::vector<ProgrammeObject> &objects) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(status.isSelected) {
        for (auto const &object: objects) {
            addMonitoringItem(object.inputMetadata);
        }
    }
    sendUpdate();
}

void ear::plugin::SceneStore::itemRemovedFromProgramme(ear::plugin::ProgrammeStatus status,
                                                       const ear::plugin::communication::ConnectionId &id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(status.isSelected) {
        auto monitoringItems = store_.mutable_monitoring_items();
        auto item = std::find_if(monitoringItems->begin(), monitoringItems->end(),
                                 [&id](auto const &item) {
                                     return item.connection_id() == id.string();
                                 });
        if(item != monitoringItems->end()) {
            monitoringItems->erase(item);
        }
    }
    sendUpdate();
}

void ear::plugin::SceneStore::programmeItemUpdated(ear::plugin::ProgrammeStatus status,
                                                   const ear::plugin::ProgrammeObject &object) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(status.isSelected) {
        auto monitoringItems = store_.mutable_monitoring_items();
        auto item = std::find_if(monitoringItems->begin(), monitoringItems->end(),
                                 [&object](auto const &item) {
                                     return item.connection_id() == object.inputMetadata.connection_id();
                                 });
        if(item != monitoringItems->end()) {
            setMonitoringItemFrom(*item, object.inputMetadata);
        } else {
            addMonitoringItem(object.inputMetadata);
        }
    }
    sendUpdate();
}

void SceneStore::inputRemoved(const communication::ConnectionId &id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto availableItems = store_.mutable_all_available_items();
    auto existingItem = std::find_if(availableItems->begin(),
                                     availableItems->end(),
                                     [&id](auto const& item) {
                                         return item.connection_id() == id.string();
                                     });
    if(existingItem != availableItems->end()) {
        availableItems->erase(existingItem);
    }
    sendUpdate();
}

void SceneStore::inputUpdated(const InputItem &item) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto availableItems = store_.mutable_all_available_items();
    auto const& itemId = item.id;
    auto existingItem = std::find_if(availableItems->begin(),
                                     availableItems->end(),
                                     [&itemId](auto const& item) {
                                         return item.connection_id() == itemId.string();
                                     });
    if(existingItem == availableItems->end()) {
        auto newItem = store_.add_all_available_items();
        newItem->CopyFrom(item.data);
    }
}

void SceneStore::addAvailableInputItemsToSceneStore(const ear::plugin::ItemMap& items) {
    for (auto const& itemPair : items) {
        auto& itemStoreInputItem = itemPair.second;
        auto sceneStoreInputItem = store_.add_all_available_items();
        sceneStoreInputItem->CopyFrom(itemStoreInputItem);
    }
}

void SceneStore::setMonitoringItemFrom(proto::MonitoringItemMetadata& monitoringItem,
                                       proto::InputItemMetadata const& inputItem) {
    monitoringItem.set_connection_id(inputItem.connection_id());
    monitoringItem.set_routing(inputItem.routing());
    monitoringItem.set_changed(true);
    if (inputItem.has_ds_metadata()) {
        monitoringItem.set_allocated_ds_metadata(
                new proto::DirectSpeakersTypeMetadata{inputItem.ds_metadata()});
    } else if (inputItem.has_mtx_metadata()) {
        monitoringItem.set_allocated_mtx_metadata(
                new proto::MatrixTypeMetadata{inputItem.mtx_metadata()});
    } else if (inputItem.has_obj_metadata()) {
        monitoringItem.set_allocated_obj_metadata(
                new proto::ObjectsTypeMetadata{inputItem.obj_metadata()});
    } else if (inputItem.has_hoa_metadata()) {
        monitoringItem.set_allocated_hoa_metadata(
                new proto::HoaTypeMetadata{inputItem.hoa_metadata()});
    } else if (inputItem.has_bin_metadata()) {
        monitoringItem.set_allocated_bin_metadata(
                new proto::BinauralTypeMetadata{inputItem.bin_metadata()});
    }
}

void SceneStore::addMonitoringItem(proto::InputItemMetadata const& inputItem) {
    auto monitoringItem = store_.add_monitoring_items();
    setMonitoringItemFrom(*monitoringItem, inputItem);
}

void SceneStore::addGroup(const proto::ProgrammeElement &element) {

}

void SceneStore::addToggle(const proto::ProgrammeElement &element) {

}

void SceneStore::sendUpdate() {
  updateCallback_(store_);
  auto& mutableItems = *store_.mutable_monitoring_items();
  for(auto& item : mutableItems) {
     item.set_changed(false);
  }
}

void SceneStore::triggerSend() {
  std::lock_guard lock(mutex_);
  sendUpdate();
}



