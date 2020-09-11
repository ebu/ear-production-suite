#include "communication/scene_metadata_receiver.hpp"

//#include <spdlog/spdlog.h>
#include <boost/variant/static_visitor.hpp>
#include <functional>

namespace ear {
namespace plugin {
namespace communication {

SceneMetadataReceiver::SceneMetadataReceiver(
    std::shared_ptr<spdlog::logger> logger)
    : logger_(logger) {}

SceneMetadataReceiver::~SceneMetadataReceiver() { socket_.asyncStop(); }

void SceneMetadataReceiver::run(const std::string& endpoint,
                                const RequestHandler& handler) {
  handler_ = handler;
  EAR_LOGGER_INFO(logger_, "Listening for metatdata on {}", endpoint);
  socket_.listen(endpoint.c_str());
  waitForMetadata();
}

void SceneMetadataReceiver::checkEndpoint(const std::string& endpoint) {
  socket_.checkEndpoint(endpoint.c_str());
}

void SceneMetadataReceiver::waitForMetadata() {
  socket_.asyncRead(std::bind(&SceneMetadataReceiver::handleReceive, this,
                              nng::placeholders::ErrorCode,
                              nng::placeholders::Message));
}

void SceneMetadataReceiver::handleReceive(std::error_code ec,
                                          nng::Message message) {
  if (ec) {
    EAR_LOGGER_ERROR(logger_, "Receiving metadata failed: {}", ec.message());
    return;  // stop receiving ... todo: check: maybe there are some error
             // conditions that allow us to keep going
  }
  try {
    proto::InputItemMetadata inputItem;
    if (message.size() > std::numeric_limits<int>::max()) {
      throw std::runtime_error("Incoming message too large");
    }
    if (!inputItem.ParseFromArray(message.data(),
                                  static_cast<int>(message.size()))) {
      throw std::runtime_error("Failed to parse Scene Store Metadata");
    }
    handler_(inputItem.connection_id(), inputItem);
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_, "Failed to parse and dispatch metadata: {}",
                     e.what());
  }
  waitForMetadata();
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
