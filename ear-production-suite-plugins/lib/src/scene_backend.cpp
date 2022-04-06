#include "scene_backend.hpp"
#include "ui/scene_frontend_backend_connector.hpp"
#include "detail/constants.hpp"
#include <functional>
#include <memory>
#include "routing_overlap.hpp"

using std::placeholders::_1;
using std::placeholders::_2;

namespace ear {
namespace plugin {
SceneBackend::SceneBackend(Metadata& data)
    : data_(data), connectionManager_(),
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

void SceneBackend::setup() {
  try {
    connectionManager_.setEventHandler(
        std::bind(&SceneBackend::onConnectionEvent, this, _1, _2));
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_, "Scene Master: Failed to set event handler: {}",
                     e.what());
    data_.setDuplicateScene(true);
  }

  try {
    commandReceiver_.checkEndpoint(detail::SCENE_MASTER_CONTROL_ENDPOINT);
    metadataReceiver_.checkEndpoint(detail::SCENE_MASTER_METADATA_ENDPOINT);
    metadataSender_.checkEndpoint(detail::SCENE_MASTER_SCENE_STREAM_ENDPOINT);
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_, "Scene Master: Checking endpoints failed: {}",
                     e.what());

    connectionManager_.setEventHandler(nullptr);
    data_.setDuplicateScene(true);
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
    data_.setDuplicateScene(true);
  }

  try {
    metadataReceiver_.run(
        detail::SCENE_MASTER_METADATA_ENDPOINT,
        [this](communication::ConnectionId id, proto::InputItemMetadata item) {
          EAR_LOGGER_DEBUG(this->logger_,
                           "Received metadata from connection {}", id.string());
            data_.setInputItemMetadata(id, item);
        });
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_,
                     "Scene Master: Failed to start metadata receiver: {}",
                     e.what());
    data_.setDuplicateScene(true);
  }

  try {
    metadataSender_.listen(detail::SCENE_MASTER_SCENE_STREAM_ENDPOINT);
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(
        logger_, "Scene Master: Failed to start metadata sender: {}", e.what());
    data_.setDuplicateScene(true);
  }
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
      data_.removeInput(id);
  } else if (event ==
             communication::SceneConnectionManager::Event::MONITORING_ADDED) {
    EAR_LOGGER_INFO(logger_, "Got new monitoring connection {}", id.string());
      data_.refresh();
  } else if (event ==
             communication::SceneConnectionManager::Event::MONITORING_REMOVED) {
    EAR_LOGGER_INFO(logger_, "Monitoring {} disconnected", id.string());
  }
}

}  // namespace plugin
}  // namespace ear
