#include "scene_backend.hpp"
#include "ui/scene_frontend_backend_connector.hpp"
#include "detail/constants.hpp"
#include "programme_store_adm_serializer.hpp"
#include <functional>
#include <google/protobuf/util/message_differencer.h>
#include <cassert>

using std::placeholders::_1;
using std::placeholders::_2;

namespace ear {
namespace plugin {
SceneBackend::SceneBackend(ui::SceneFrontendBackendConnector* frontend)
    : connectionManager_(), frontendConnector_(frontend) {
  logger_ = createLogger(fmt::format("Scene Master @{}", (const void*)this));
#ifndef NDEBUG
  logger_->set_level(spdlog::level::trace);
#else
  logger_->set_level(spdlog::level::off);
#endif
  EAR_LOGGER_DEBUG(logger_, "SceneBackend created");

  commandReceiver_.setLogger(logger_);
  metadataReceiver_.setLogger(logger_);
}

SceneBackend::~SceneBackend() { metadataSender_.asyncStop(); }

communication::MessageBuffer SceneBackend::getMessage() {
  std::lock_guard<std::mutex> lock(storeMutex_);
  if (rebuildSceneStore_) {
    rebuildSceneStore_ = false;
    updateSceneStore();
  }
  communication::MessageBuffer buffer =
      communication::allocBuffer(sceneStore_.ByteSizeLong());
  sceneStore_.SerializeToArray(buffer.data(), buffer.size());
  return buffer;
}

void SceneBackend::triggerMetadataSend() {
  auto message = getMessage();
  metadataSender_.asyncWait();
  metadataSender_.asyncSend(
      message, [this](std::error_code ec, const nng::Message&) {
        if (ec) {
          EAR_LOGGER_WARN(this->logger_, "Sending scene metadata failed: {}",
                          ec.message());
        }
      });
  std::lock_guard<std::mutex> lock(storeMutex_);
  for (auto item : itemStore_) {
    item.second.set_changed(false);
  }
}

void SceneBackend::setup() {
  try {
    connectionManager_.setEventHandler(
        std ::bind(&SceneBackend::onConnectionEvent, this, _1, _2));
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_, "Scene Master: Failed to set event handler: {}",
                     e.what());
    frontendConnector_->setMultipleScenePluginsOverlayVisible(true);
  }

  try {
    commandReceiver_.checkEndpoint(detail::SCENE_MASTER_CONTROL_ENDPOINT);
    metadataReceiver_.checkEndpoint(detail::SCENE_MASTER_METADATA_ENDPOINT);
    metadataSender_.checkEndpoint(detail::SCENE_MASTER_SCENE_STREAM_ENDPOINT);
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_, "Scene Master: Checking endpoints failed: {}",
                     e.what());

    connectionManager_.setEventHandler(nullptr);
    frontendConnector_->setMultipleScenePluginsOverlayVisible(true);
    throw;
  }

  try {
    commandReceiver_.run(
        detail::SCENE_MASTER_CONTROL_ENDPOINT,
        std::bind(&communication::SceneConnectionManager::handle,
                  &connectionManager_, _1));
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_,
                     "Scene Master: Failed to start command receiver: {}",
                     e.what());
    frontendConnector_->setMultipleScenePluginsOverlayVisible(true);
  }

  try {
    metadataReceiver_.run(
        detail::SCENE_MASTER_METADATA_ENDPOINT,
        [this](communication::ConnectionId id, proto::InputItemMetadata item) {
          EAR_LOGGER_DEBUG(this->logger_,
                           "Received metadata from connection {}", id.string());
          std::lock_guard<std::mutex> lock(storeMutex_);
          if (itemStore_.find(id) != itemStore_.end()) {
            // UPDATE ITEM
            if (!google::protobuf::util::MessageDifferencer::
                    ApproximatelyEquivalent(itemStore_.at(id), item)) {
              itemStore_[id] = item;
              frontendConnector_->updateItem(id, item);
              rebuildSceneStore_ = true;
            }
          } else {
            // NEW ITEM
            itemStore_[id] = item;
            frontendConnector_->updateItem(id, item);
            rebuildSceneStore_ = true;
          }
        });
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_,
                     "Scene Master: Failed to start metadata receiver: {}",
                     e.what());
    frontendConnector_->setMultipleScenePluginsOverlayVisible(true);
  }

  try {
    metadataSender_.listen(detail::SCENE_MASTER_SCENE_STREAM_ENDPOINT);
    if (frontendConnector_) {
      frontendConnector_->onProgrammeStoreChanged(
          std::bind(&SceneBackend::onProgrammeStoreChanged, this, _1));
    }
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(
        logger_, "Scene Master: Failed to start metadata sender: {}", e.what());
    frontendConnector_->setMultipleScenePluginsOverlayVisible(true);
  }
}

inline ItemStore::const_iterator findObject(proto::Object const& object,
                                            ItemStore const& itemStore) {
  auto id = communication::ConnectionId{object.connection_id()};
  return itemStore.find(id);
}

void SceneBackend::updateSceneStore() {
  sceneStore_.clear_items();

  if (auto programme = getSelectedProgramme()) {
    for (auto& element : programme->element()) {
      addGroupToSceneStore(element);
      addToggleToSceneStore(element);
      addElementToSceneStore(element);
    }
  }

  previousScene_.clear();
  for (auto const& item : sceneStore_.items()) {
    previousScene_.emplace(item.connection_id());
  }
}

proto::Programme const* SceneBackend::getSelectedProgramme() {
  if (programmeStore_.has_selected_programme_index()) {
    auto index = programmeStore_.selected_programme_index();
    assert(index < programmeStore_.programme_size());
    auto& programme = programmeStore_.programme(index);
    return &programme;
  }
  return nullptr;
}

void SceneBackend::addGroupToSceneStore(
    proto::ProgrammeElement const& element) {
  if (element.has_group()) {
    for (auto& progItem : element.group().element()) {
      addElementToSceneStore(progItem);
    }
  }
}

void SceneBackend::addToggleToSceneStore(
    proto::ProgrammeElement const& element) {
  if (element.has_toggle()) {
    auto toggle = element.toggle();
    if (toggle.has_selected_element_index()) {
      assert(toggle.selected_element_index() < toggle.element().size());
      auto& progItem = toggle.element(toggle.selected_element_index());
      addElementToSceneStore(progItem);
    }
  }
}

void SceneBackend::addElementToSceneStore(
    proto::ProgrammeElement const& element) {
  if (element.has_object()) {
    auto position = findObject(element.object(), itemStore_);
    if (position != itemStore_.end()) {
      addToSceneStore(position->second);
    }
  }
}

void SceneBackend::addToSceneStore(proto::InputItemMetadata const& inputItem) {
  auto monitoringItem = sceneStore_.add_items();
  monitoringItem->set_connection_id(inputItem.connection_id());
  monitoringItem->set_routing(inputItem.routing());
  auto newItem =
      previousScene_.find(inputItem.connection_id()) == previousScene_.end();
  monitoringItem->set_changed(newItem || inputItem.changed());
  if (inputItem.has_ds_metadata()) {
    monitoringItem->set_allocated_ds_metadata(
        new proto::DirectSpeakersTypeMetadata{inputItem.ds_metadata()});
  } else if (inputItem.has_mtx_metadata()) {
    monitoringItem->set_allocated_mtx_metadata(
        new proto::MatrixTypeMetadata{inputItem.mtx_metadata()});
  } else if (inputItem.has_obj_metadata()) {
    monitoringItem->set_allocated_obj_metadata(
        new proto::ObjectsTypeMetadata{inputItem.obj_metadata()});
  } else if (inputItem.has_hoa_metadata()) {
    monitoringItem->set_allocated_hoa_metadata(
        new proto::HoaTypeMetadata{inputItem.hoa_metadata()});
  } else if (inputItem.has_bin_metadata()) {
    monitoringItem->set_allocated_bin_metadata(
        new proto::BinauralTypeMetadata{inputItem.bin_metadata()});
  }
}

void SceneBackend::onConnectionEvent(
    communication::SceneConnectionManager::Event event,
    communication::ConnectionId id) {
  if (event == communication::SceneConnectionManager::Event::INPUT_ADDED) {
    EAR_LOGGER_INFO(logger_, "Got new input connection {}", id.string());
    auto& info = connectionManager_.connectionInfo(id);
    {
      std::lock_guard<std::mutex> lock(storeMutex_);
      itemStore_[id] = proto::InputItemMetadata{};
    }
    frontendConnector_->addItem(id);
  } else if (event ==
             communication::SceneConnectionManager::Event::INPUT_REMOVED) {
    EAR_LOGGER_INFO(logger_, "Input {} disconnected", id.string());
    {
      std::lock_guard<std::mutex> lock(storeMutex_);
      auto it = itemStore_.find(id);
      if (it != itemStore_.end()) {
        itemStore_.erase(it);
      }
      rebuildSceneStore_ = true;
    }
    frontendConnector_->removeItem(id);
  } else if (event ==
             communication::SceneConnectionManager::Event::MONITORING_ADDED) {
    EAR_LOGGER_INFO(logger_, "Got new monitoring connection {}", id.string());
  } else if (event ==
             communication::SceneConnectionManager::Event::MONITORING_REMOVED) {
    EAR_LOGGER_INFO(logger_, "Monitoring {} disconnected", id.string());
  }
}

void SceneBackend::onProgrammeStoreChanged(proto::ProgrammeStore store) {
  std::lock_guard<std::mutex> lock{storeMutex_};
  programmeStore_ = store;
  rebuildSceneStore_ = true;
}

std::pair<ItemStore, proto::ProgrammeStore> SceneBackend::stores() {
  std::lock_guard<std::mutex> lock{storeMutex_};
  return {itemStore_, programmeStore_};
}

}  // namespace plugin
}  // namespace ear
