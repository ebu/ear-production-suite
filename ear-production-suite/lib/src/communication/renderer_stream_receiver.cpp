#include "communication/monitoring_stream_receiver.hpp"
#include "scene_store.pb.h"
#include <functional>

namespace ear {
namespace plugin {
namespace communication {

MonitoringStreamReceiver::MonitoringStreamReceiver(
    std::shared_ptr<spdlog::logger> logger)
    : logger_(logger) {}

MonitoringStreamReceiver::~MonitoringStreamReceiver() {
  // stop any pending async operation, otherwise
  // the callback will try to call already destructed parts
  // the shutdown() method alone is not helpful, as
  // the call to asyncCancel() does not block (on purpose)
  // and the destructor might be called immediatly afterwards
  socket_.asyncStop();
}

void MonitoringStreamReceiver::start(const std::string& endpoint,
                                     const RequestHandler& handler) {
  // Subscribe to "all" (== "") notifications, without setting this option
  // _nothing_ will be received!
  handler_ = handler;
  socket_.setOpt(nng::options::SubSubscribe, "", 0);
  socket_.dial(endpoint.c_str());
  waitForMetadata();
}

void MonitoringStreamReceiver::waitForMetadata() {
  socket_.asyncRead(std::bind(&MonitoringStreamReceiver::handleReceive, this,
                              nng::placeholders::ErrorCode,
                              nng::placeholders::Message));
}

void MonitoringStreamReceiver::shutdown() {
  // we can only use the non-blocking cancel here
  // as this function might be called from within another
  // nng callback handler, which would result in a deadlock.
  // Thus, we just cancel the operation and shut the socket down
  // from wtihin the handler
  // Although tempting, don't try to use asyncWait() or asyncStop()
  // here, as this will block forever as well.
  socket_.asyncCancel();
}

void MonitoringStreamReceiver::handleReceive(std::error_code ec,
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
      if (message.size() == 0) {
        throw std::runtime_error(
            "Failed to parse Scene Object: Invalid Buffer - size == 0");
      }
      if (!sceneStore.ParseFromArray(message.data(), message.size())) {
        throw std::runtime_error("Failed to parse Scene Object");
      }
      handler_(sceneStore);
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
