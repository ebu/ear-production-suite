#include "communication/scene_command_receiver.hpp"
#include "communication/commands.hpp"

//#include <spdlog/spdlog.h>

namespace ear {
namespace plugin {
namespace communication {

SceneCommandReceiver::SceneCommandReceiver(
    std::shared_ptr<spdlog::logger> logger)
    : logger_(logger) {}

void SceneCommandReceiver::run(const std::string& endpoint,
                               const RequestHandler& handler) {
  handler_ = handler;
  socket_.listen(endpoint.c_str());
  EAR_LOGGER_INFO(logger_, "Listening for control connections on {}", endpoint);
  waitForCommand();
}

void SceneCommandReceiver::checkEndpoint(const std::string& endpoint) {
  socket_.checkEndpoint(endpoint.c_str());
}

void SceneCommandReceiver::waitForCommand() {
  socket_.asyncRead(std::bind(&SceneCommandReceiver::handleReceive, this,
                              nng::placeholders::ErrorCode,
                              nng::placeholders::Message));
}

void SceneCommandReceiver::handleReceive(std::error_code ec,
                                         const nng::Message& message) {
  if (ec) {
    EAR_LOGGER_ERROR(logger_, ec.message());
    // For _some_ (few) errors it makes sense to just cary on waiting for
    // requests.
    if (ec.value() == NNG_ETIMEDOUT) {
      waitForCommand();
    }
    return;
  }
  try {
    const RequestVariant requestVariant = parseRequest(message);
    auto response = handler_(requestVariant);
    sendResponse(response);
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_, "failed to parse request: {}", e.what());
    sendResponse(ErrorCode::UNKOWN_ERROR, e.what());
  }
}

void SceneCommandReceiver::sendResponse(const Response& response) {
  auto buffer = serialize(response);
  socket_.asyncSend(buffer, [this](std::error_code ec,
                                   const nng::Message& message) {
    if (ec) {
      EAR_LOGGER_ERROR(logger_, "Failed to send response: {}", ec.message());
      // TODO
      // check: maybe retry to send message here ...
    }
    waitForCommand();
  });
}

void SceneCommandReceiver::sendResponse(ErrorCode ec,
                                        const std::string& description) {
  sendResponse(Response{ec, description});
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
