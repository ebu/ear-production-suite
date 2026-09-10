#include "communication/input_control_connection.hpp"

#include "communication/commands.hpp"
#include "log.hpp"

#include <utility>

namespace ear {
namespace plugin {
namespace communication {

InputControlConnection::InputControlConnection(
    std::shared_ptr<spdlog::logger> logger)
    : socket_{[this] { core_.connected(); },
              [this] {
                auto socketLogger = core_.logger();
                EAR_LOGGER_WARN(socketLogger, "Socket disconnect");
                core_.disconnected();
              }},
      core_{ControlConnectionCore::Operations{
                [this](const std::string& endpoint) { socket_.open(endpoint); },
                [this] {
                  socket_.stopPipeEvents();
                  socket_.asyncStop();
                },
                [this] { socket_.asyncCancel(); },
                [this](const MessageBuffer& buffer,
                       ControlConnectionCore::AsyncResponseHandler handler) {
                  return socket_.asyncRequest(buffer, std::move(handler));
                },
                [this](std::function<void()> callback) {
                  return socket_.post(std::move(callback));
                }},
            ConnectionType::METADATA_INPUT,
            "object",
            [](ConnectionId id) { return serialize(ObjectDetailsMessage{id}); },
            [](const Response& response) {
              return response.payloadAs<ConnectionDetailsResponse>()
                  .metadataEndpoint();
            },
            std::move(logger),
            ControlConnectionCore::LoggingOptions{false, true, true}} {}

InputControlConnection::~InputControlConnection() { stop(); }

void InputControlConnection::logger(std::shared_ptr<spdlog::logger> logger) {
  core_.logger(std::move(logger));
}

void InputControlConnection::setConnectionId(ConnectionId id) {
  core_.setConnectionId(std::move(id), [this](ConnectionId connectionId) {
    return closeConnection(connectionId);
  });
}

ConnectionId InputControlConnection::getConnectionId() const {
  return core_.getConnectionId();
}

void InputControlConnection::start(const std::string& endpoint) {
  core_.start(endpoint);
}

void InputControlConnection::stop() { core_.stop(); }

bool InputControlConnection::closeConnection(ConnectionId connectionId) {
  if (!connectionId.isValid()) {
    connectionId = core_.getConnectionId();
  }

  // nng_aio_cancel() returns before the request callback has completed. Wait
  // for the request socket to leave its previous operation before reusing it
  // synchronously for the close request.
  socket_.asyncWait();
  socket_.requestCloseConnection(connectionId);
  auto response = socket_.receive();
  if (!response.success()) {
    auto logger = core_.logger();
    EAR_LOGGER_ERROR(logger, "Failed to start close control connection: {}",
                     response.errorDescription());
    return false;
  }
  return true;
}

void InputControlConnection::onConnectionEstablished(
    ConnectionEstablishedHandler callback) {
  core_.onConnectionEstablished(std::move(callback));
}

void InputControlConnection::onConnectionLost(ConnectionLostHandler callback) {
  core_.onConnectionLost(std::move(callback));
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
