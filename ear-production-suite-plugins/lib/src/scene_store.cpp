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
    flagOverlaps();
    changed = true;
}

void ear::plugin::SceneStore::programmeSelected(const ear::plugin::ProgrammeObjects &objects) {
    store_.clear_monitoring_items();
    for(auto const& object : objects) {
        auto inputData = object.inputMetadata;
        inputData.set_changed(true);
        addMonitoringItem(inputData);
    }
    flagOverlaps();
    changed = true;
}

void ear::plugin::SceneStore::itemsAddedToProgramme(ear::plugin::ProgrammeStatus status,
                                                    const std::vector<ProgrammeObject> &objects) {
    if(status.isSelected) {
        for (auto const &object: objects) {
            addMonitoringItem(object.inputMetadata);
        }
        flagOverlaps();
    }
}

namespace {
    template<typename T>
    auto findItem(T mutableRepeatedField, std::string const& connectionId) {
        return std::find_if(mutableRepeatedField->begin(),
                            mutableRepeatedField->end(),
                            [&connectionId](auto const& item) {
            return item.connection_id() == connectionId;
        });
    }
}

void ear::plugin::SceneStore::itemRemovedFromProgramme(ear::plugin::ProgrammeStatus status,
                                                       const ear::plugin::communication::ConnectionId &id) {
    if(status.isSelected) {
        auto monitoringItems = store_.mutable_monitoring_items();
        if(auto item = findItem(monitoringItems, id.string());
                item != monitoringItems->end()) {
            monitoringItems->erase(item);
            changed = true;
            flagOverlaps();
        }
    }
}

bool SceneStore::updateMonitoringItem(proto::InputItemMetadata const& inputItem) {
    auto monitoringItems = store_.mutable_monitoring_items();
    if(auto item = findItem(monitoringItems, inputItem.connection_id());
            item != monitoringItems->end()) {
        auto routingChanged = item->routing() != inputItem.routing();
        setMonitoringItemFrom(*item, inputItem);
        if(routingChanged) {
            flagOverlaps();
        }
        return true;
    } else {
        return false;
    }
}

void ear::plugin::SceneStore::programmeItemUpdated(ear::plugin::ProgrammeStatus status,
                                                   const ear::plugin::ProgrammeObject &object) {
    if(status.isSelected) {
        if(!updateMonitoringItem(object.inputMetadata)) {
            addMonitoringItem(object.inputMetadata);
            flagOverlaps();
        }
    }
}

void SceneStore::inputRemoved(const communication::ConnectionId &id) {
    auto availableItems = store_.mutable_all_available_items();
    if(auto existingItem = findItem(availableItems, id.string());
       existingItem != availableItems->end()) {
          availableItems->erase(existingItem);
          changed = true;
    }
}

void SceneStore::inputUpdated(const InputItem &item, proto::InputItemMetadata const& oldItem) {
    auto availableItems = store_.mutable_all_available_items();
    if(auto existingItem = findItem(availableItems, item.id.string());
            existingItem == availableItems->end()) {
        auto newItem = store_.add_all_available_items();
        newItem->CopyFrom(item.data);
        changed = true;
    } else {
        existingItem->CopyFrom(item.data);
        // Update monitoring items here as events are asynchronous,
        // otherwise we risk an update between reducing channel count
        // here and updating rendered items via programmeItemUpdated.
        updateMonitoringItem(item.data);
        changed = changed || item.data.changed();
    }
}

void SceneStore::addAvailableInputItemsToSceneStore(const ear::plugin::ItemMap& items) {
    for (auto const& itemPair : items) {
        auto& itemStoreInputItem = itemPair.second;
        auto sceneStoreInputItem = store_.add_all_available_items();
        sceneStoreInputItem->CopyFrom(itemStoreInputItem);
        sceneStoreInputItem->set_changed(true);
    }
}

void SceneStore::setMonitoringItemFrom(proto::MonitoringItemMetadata& monitoringItem,
                                       proto::InputItemMetadata const& inputItem) {
    monitoringItem.set_connection_id(inputItem.connection_id());
    monitoringItem.set_routing(inputItem.routing());
    changed = changed || inputItem.changed();
    monitoringItem.set_changed(inputItem.changed());
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
  changed = false;
}

void SceneStore::triggerSend() {
  if(sendData && changed) {
      sendUpdate();
      if(store_.is_exporting()) sendData = false;
  }
}

void SceneStore::exporting(bool isExporting) {
    if(isExporting) {
        store_.set_is_exporting(true);
    } else {
        store_.set_is_exporting(false);
        sendData = true;
    }
    changed = true;
}

void SceneStore::flagOverlaps() {
    auto overlaps = getOverlapIds(store_);
    if (overlappingIds_ != overlaps) {
        flagChangedOverlaps(overlappingIds_, overlaps, store_);
    }
    overlappingIds_ = overlaps;

}
