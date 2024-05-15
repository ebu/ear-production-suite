#include "communication/monitoring_metadata_receiver.hpp"
#include "scene_store.pb.h"
#include <functional>
#include <future>

namespace ear {
namespace plugin {
namespace communication {

MonitoringMetadataReceiver::MonitoringMetadataReceiver(
    std::shared_ptr<spdlog::logger> logger)
    : logger_(logger) {}

MonitoringMetadataReceiver::~MonitoringMetadataReceiver() {
  // stop any pending async operation, otherwise
  // the callback will try to call already destructed parts
  // the shutdown() method alone is not helpful, as
  // the call to asyncCancel() does not block (on purpose)
  // and the destructor might be called immediatly afterwards
  socket_.asyncStop();
}

void MonitoringMetadataReceiver::start(const std::string& endpoint,
                                       const RequestHandler& handler) {
  // Subscribe to "all" (== "") notifications, without setting this option
  // _nothing_ will be received!
  handler_ = handler;
  socket_.setOpt(nng::options::SubSubscribe, "", 0);
  socket_.dial(endpoint.c_str());
  waitForMetadata();
}

void MonitoringMetadataReceiver::waitForMetadata() {
  socket_.asyncRead(std::bind(&MonitoringMetadataReceiver::handleReceive, this,
                              nng::placeholders::ErrorCode,
                              nng::placeholders::Message));
}

void MonitoringMetadataReceiver::shutdown() {
  // we can only use the non-blocking cancel here
  // as this function might be called from within another
  // nng callback handler, which would result in a deadlock.
  // Thus, we just cancel the operation and shut the socket down
  // from wtihin the handler
  // Although tempting, don't try to use asyncWait() or asyncStop()
  // here, as this will block forever as well.
  socket_.asyncCancel();
}

void MonitoringMetadataReceiver::handleReceive(std::error_code ec,
                                               nng::Message message) {
  if (!ec) {
    EAR_LOGGER_TRACE(logger_, "Received scene metadata");
    try {
      proto::SceneStore sceneStore;
      if (message.size() > std::numeric_limits<int>::max()) {
        throw std::runtime_error("Incoming message too large");
      }
      if (message.data() == nullptr) {
        throw std::runtime_error(
            "Failed to parse Scene Object: Invalid Buffer - null pointer");
      }
      if (!sceneStore.ParseFromArray(message.data(), message.size())) {
        throw std::runtime_error("Failed to parse Scene Object");
      }
      // Called by NNG callback on thread with small stack.
      // Launch task in another thread to overcome stack limitation.
      auto future = std::async(std::launch::async, [this, &sceneStore]() {
        handler_(sceneStore);
      });
      future.get(); //blocking
    } catch (const std::runtime_error& e) {
      EAR_LOGGER_ERROR(
          logger_, "Failed to parse and dispatch scene metadata: {}", e.what());
    }
    waitForMetadata();
  } else if (ec.value() == NNG_ECANCELED) {
    EAR_LOGGER_INFO(logger_, "Operation cancelled, stopping stream receiver");
    socket_.close();
  } else {
    EAR_LOGGER_ERROR(logger_, "Failed to receive scene metadata: {}",
                     ec.message());
  }
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
