#include "scene_backend.hpp"
#include "ui/scene_frontend_backend_connector.hpp"
#include "detail/constants.hpp"
#include "programme_store_adm_serializer.hpp"
#include <functional>
#include <memory>
#include <google/protobuf/util/message_differencer.h>
#include <cassert>
#include "routing_overlap.hpp"

using std::placeholders::_1;
using std::placeholders::_2;

namespace ear {
namespace plugin {
SceneBackend::SceneBackend(ui::SceneFrontendBackendConnector* frontend, Metadata& data)
    : data_(data), connectionManager_(), frontendConnector_(frontend),
      sceneStore_(std::make_shared<SceneStore>([this](proto::SceneStore const& store) {
          triggerMetadataSend(store);
      })) {
  logger_ = createLogger(fmt::format("Scene Master @{}", (const void*)this));
#ifdef EPS_ENABLE_LOGGING
  logger_->set_level(spdlog::level::trace);
#else
  logger_->set_level(spdlog::level::off);
#endif
  EAR_LOGGER_DEBUG(logger_, "SceneBackend created");

  commandReceiver_.setLogger(logger_);
  metadataReceiver_.setLogger(logger_);
  data_.addListener(sceneStore_);
}

SceneBackend::~SceneBackend() { metadataSender_.asyncStop(); }

//communication::MessageBuffer SceneBackend::getMessage() {
//  if (rebuildSceneStore_) {
//    rebuildSceneStore_ = false;
//    updateSceneStore();
//  }
//  communication::MessageBuffer buffer =
//      communication::allocBuffer(sceneStore_.ByteSizeLong());
//  sceneStore_.SerializeToArray(buffer.data(), buffer.size());
//  return buffer;
//}

void SceneBackend::triggerMetadataSend(const proto::SceneStore &store) {
    communication::MessageBuffer buffer =
            communication::allocBuffer(store.ByteSizeLong());
    store.SerializeToArray(buffer.data(), buffer.size());
    metadataSender_.asyncWait();
    metadataSender_.asyncSend(
            buffer, [this](std::error_code ec, const nng::Message&) {
                if (ec) {
                    EAR_LOGGER_WARN(this->logger_, "Sending scene metadata failed: {}",
                                    ec.message());
                }
            });
}

void SceneBackend::triggerMetadataSend() {
    sceneStore_->triggerSend();
}
//  auto message = getMessage();
//  metadataSender_.asyncWait();
//  metadataSender_.asyncSend(
//      message, [this](std::error_code ec, const nng::Message&) {
//        if (ec) {
//          EAR_LOGGER_WARN(this->logger_, "Sending scene metadata failed: {}",
//                          ec.message());
//        }
//      });
//}

//void SceneBackend::changesCleared() {
//  rebuildSceneStore_ = true;
//}

void SceneBackend::setup() {
  try {
    connectionManager_.setEventHandler(
        std::bind(&SceneBackend::onConnectionEvent, this, _1, _2));
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

  using google::protobuf::util::MessageDifferencer;
  try {
    metadataReceiver_.run(
        detail::SCENE_MASTER_METADATA_ENDPOINT,
        [this](communication::ConnectionId id, proto::InputItemMetadata item) {
          EAR_LOGGER_DEBUG(this->logger_,
                           "Received metadata from connection {}", id.string());
          data_.withItemStore([&id, &item](auto& store) {
            store.setItem(id, item);
          });
//          auto previous = itemStore_.setItem(id, item);
//          // UPDATE ITEM
//          if(!previous || !MessageDifferencer::ApproximatelyEquivalent(*previous, item)) {
////              frontendConnector_->updateItem(id, item);
//              rebuildSceneStore_ = true;
//            }
        });
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_,
                     "Scene Master: Failed to start metadata receiver: {}",
                     e.what());
    frontendConnector_->setMultipleScenePluginsOverlayVisible(true);
  }

  try {
    metadataSender_.listen(detail::SCENE_MASTER_SCENE_STREAM_ENDPOINT);
//    if (frontendConnector_) {
//      frontendConnector_->onProgrammeStoreChanged(
//          std::bind(&SceneBackend::onProgrammeStoreChanged, this, _1));
//    }
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(
        logger_, "Scene Master: Failed to start metadata sender: {}", e.what());
    frontendConnector_->setMultipleScenePluginsOverlayVisible(true);
  }
}

void SceneBackend::updateSceneStore() {
//  sceneStore_.clear_monitoring_items();
//  if (auto programme = programmeStore_.selectedProgramme()) {
//    for (auto const& element : programme->element()) {
//      addGroupToSceneStore(element);
//      addToggleToSceneStore(element);
//      addElementToSceneStore(element);
//    }
//  }
//
//  sceneStore_.clear_all_available_items();
//  addAvailableInputItemsToSceneStore();
//
//  auto overlaps = getOverlapIds(sceneStore_);
//  if (overlappingIds_ != overlaps) {
//    flagChangedOverlaps(overlappingIds_, overlaps, sceneStore_);
//  }
//  overlappingIds_ = overlaps;
//
//  previousScene_.clear();
//  for (auto const& item : sceneStore_.monitoring_items()) {
//    previousScene_.emplace(item.connection_id());
//  }
}

void SceneBackend::addGroupToSceneStore(
    proto::ProgrammeElement const& element) {
//  if (element.has_group()) {
//    for (auto& progItem : element.group().element()) {
//      addElementToSceneStore(progItem);
//    }
//  }
}

void SceneBackend::addToggleToSceneStore(
    proto::ProgrammeElement const& element) {
//  if (element.has_toggle()) {
//    auto toggle = element.toggle();
//    if (toggle.has_selected_element_index()) {
//      assert(toggle.selected_element_index() < toggle.element().size());
//      auto& progItem = toggle.element(toggle.selected_element_index());
//      addElementToSceneStore(progItem);
//    }
//  }
}

void SceneBackend::onConnectionEvent(
    communication::SceneConnectionManager::Event event,
    communication::ConnectionId id) {
  if (event == communication::SceneConnectionManager::Event::INPUT_ADDED) {
    EAR_LOGGER_INFO(logger_, "Got new input connection {}", id.string());
    auto& info = connectionManager_.connectionInfo(id);
  } else if (event ==
             communication::SceneConnectionManager::Event::INPUT_REMOVED) {
    EAR_LOGGER_INFO(logger_, "Input {} disconnected", id.string());
    data_.withItemStore([&id](auto& store) {
      store.removeItem(id);
    });
    rebuildSceneStore_ = true;
//    frontendConnector_->removeItem(id);
  } else if (event ==
             communication::SceneConnectionManager::Event::MONITORING_ADDED) {
    EAR_LOGGER_INFO(logger_, "Got new monitoring connection {}", id.string());
    sceneStore_->triggerSend();
  } else if (event ==
             communication::SceneConnectionManager::Event::MONITORING_REMOVED) {
    EAR_LOGGER_INFO(logger_, "Monitoring {} disconnected", id.string());
  }
}

}  // namespace plugin
}  // namespace ear
