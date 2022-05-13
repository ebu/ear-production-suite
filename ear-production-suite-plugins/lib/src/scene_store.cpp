//
// Created by Richard Bailey on 24/03/2022.
//

#include "../include/scene_store.hpp"
#include "routing_overlap.hpp"
#include "programme_internal_id.hpp"
#include "helper/container_helpers.hpp"
#include <algorithm>

using namespace ear::plugin;
SceneStore::SceneStore(std::function<void(proto::SceneStore const&)> update) :
    updateCallback_{std::move(update)} {
}

void SceneStore::dataReset(const ear::plugin::proto::ProgrammeStore &programmes,
                           const ear::plugin::ItemMap &items) {
    for(const auto& item : store_.all_available_items()) {
      itemsChangedSinceLastSend.insert(item.connection_id());
    }
    store_ = {};
    addAvailableInputItemsToSceneStore(items);
    auto selectedId = programmes.selected_programme_internal_id();
    auto selectedProgramme = getProgrammeWithId(programmes, selectedId);
    if(selectedProgramme) {
        for(auto const& element : selectedProgramme->element()) {
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
}

void ear::plugin::SceneStore::programmeSelected(const ear::plugin::ProgrammeObjects &objects) {
    store_.clear_monitoring_items();
    for(auto const& object : objects) {
        addMonitoringItem(object.inputMetadata);
    }
    flagOverlaps();
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
            itemsChangedSinceLastSend.insert(id);
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
    itemsChangedSinceLastSend.insert(id);
    auto availableItems = store_.mutable_all_available_items();
    if(auto existingItem = findItem(availableItems, id.string());
       existingItem != availableItems->end()) {
          availableItems->erase(existingItem);
    }
}

void SceneStore::inputUpdated(const InputItem &item, proto::InputItemMetadata const& oldItem) {
    auto availableItems = store_.mutable_all_available_items();
    if(auto existingItem = findItem(availableItems, item.id.string());
            existingItem == availableItems->end()) {
        itemsChangedSinceLastSend.insert(item.data.connection_id());
        auto newItem = store_.add_all_available_items();
        newItem->CopyFrom(item.data);
    } else {
        if(item.data.changed()) {
          itemsChangedSinceLastSend.insert(item.data.connection_id());
        }
        existingItem->CopyFrom(item.data);
        // Update monitoring items here as events are asynchronous,
        // otherwise we risk an update between reducing channel count
        // here and updating rendered items via programmeItemUpdated.
        updateMonitoringItem(item.data);
    }
}

void ear::plugin::SceneStore::inputAdded(const InputItem & item, bool autoModeState)
{
    auto availableItems = store_.mutable_all_available_items();
    if(auto existingItem = findItem(availableItems, item.id.string());
       existingItem == availableItems->end()) {
      itemsChangedSinceLastSend.insert(item.data.connection_id());
      auto newItem = store_.add_all_available_items();
      newItem->CopyFrom(item.data);
    }
}

void SceneStore::addAvailableInputItemsToSceneStore(const ear::plugin::ItemMap& items) {
    for (auto const& itemPair : items) {
        auto& itemStoreInputItem = itemPair.second;
        auto sceneStoreInputItem = store_.add_all_available_items();
        sceneStoreInputItem->CopyFrom(itemStoreInputItem);
        itemsChangedSinceLastSend.insert(itemStoreInputItem.connection_id());
    }
}

void SceneStore::setMonitoringItemFrom(proto::MonitoringItemMetadata& monitoringItem,
                                       proto::InputItemMetadata const& inputItem) {
    monitoringItem.set_connection_id(inputItem.connection_id());
    monitoringItem.set_routing(inputItem.routing());
    monitoringItem.set_changed(inputItem.changed());
    if(inputItem.changed()) {
      itemsChangedSinceLastSend.insert(inputItem.connection_id());
    }
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
    itemsChangedSinceLastSend.insert(inputItem.connection_id());
}

void SceneStore::addGroup(const proto::ProgrammeElement &element) {

}

void SceneStore::addToggle(const proto::ProgrammeElement &element) {

}

void SceneStore::sendUpdate() {
  auto& mutableMonitoringItemMetadata = *store_.mutable_monitoring_items();
  for(auto& item : mutableMonitoringItemMetadata) {
    if(contains(itemsChangedSinceLastSend, communication::ConnectionId(item.connection_id()))) {
      item.set_changed(true);
    }
  }
  auto& mutableInputItemMetadata = *store_.mutable_all_available_items();
  for(auto& item : mutableInputItemMetadata) {
    if(contains(itemsChangedSinceLastSend, communication::ConnectionId(item.connection_id()))) {
      item.set_changed(true);
    }
  }
  updateCallback_(store_);
  itemsChangedSinceLastSend.clear();
}

void SceneStore::triggerSend() {
  bool doSend = true;
  switch(exportingSendState) {
    case ExportingSendState::EXPORT_START:
      exportingSendState = ExportingSendState::EXPORTING;
      doSend = true; // Force one-time update - next time falls to EXPORTING case
      break;
    case ExportingSendState::EXPORT_END:
      exportingSendState = ExportingSendState::NOT_EXPORTING;
      doSend = true; // Force one-time update - next time falls to NOT_EXPORTING case
      break;
    case ExportingSendState::EXPORTING:
      doSend = false;
      break;
    default: // NOT_EXPORTING - depends upon changes to items
      doSend = (itemsChangedSinceLastSend.size() > 0);
      break;
  }

  if(doSend) {
      sendUpdate();
  }
}

void SceneStore::exporting(bool isExporting) {
    if(isExporting) {
        store_.set_is_exporting(true);
        exportingSendState = ExportingSendState::EXPORT_START;
    } else {
        store_.set_is_exporting(false);
        exportingSendState = ExportingSendState::EXPORT_END;
    }
}

void SceneStore::flagOverlaps() {
    auto overlaps = getOverlapIds(store_);
    if (overlappingIds_ != overlaps) {
        flagChangedOverlaps(overlappingIds_, overlaps, store_);
    }
    overlappingIds_ = overlaps;

}
