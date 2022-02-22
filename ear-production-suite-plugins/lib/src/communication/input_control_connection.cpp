#include "communication/input_control_connection.hpp"
#include "communication/commands.hpp"
#include "log.hpp"
//#include <spdlog/spdlog.h>
#include <functional>
#include <chrono>

using namespace std::chrono_literals;

namespace ear {
namespace plugin {
namespace communication {
InputControlConnection::InputControlConnection() :
    socket_{ [this]{connected();},
             [this]{disconnected();}
    },
    connected_(false) {
}

InputControlConnection::~InputControlConnection() { disconnect(); }

void InputControlConnection::logger(std::shared_ptr<spdlog::logger> logger) {
  logger_ = logger;
}

void InputControlConnection::setConnectionId(ConnectionId id) {
  if (isConnected()) {
    disconnect();
  }
  connectionId_ = id;
  handshake();
}

void InputControlConnection::handshake() {
  try {
    {
      EAR_LOGGER_TRACE(logger_, "Requesting connection ID ({})",
                       connectionId_.string());
      socket_.requestNewConnection(connectionId_);
      auto reply = socket_.receive();
      auto payload = reply.payloadAs<NewConnectionResponse>();
      if (!payload.connectionId().isValid()) {
        EAR_LOGGER_ERROR(logger_,
                         "Failed to start new control connection: invalid "
                         "connection id received");
        return;
      }

      connectionId_ = payload.connectionId();
      EAR_LOGGER_DEBUG(logger_, "Got connection ID {}", connectionId_.string());
    }
    {
      EAR_LOGGER_TRACE(logger_, "Sending object connection details");
      socket_.requestObjectDetails(connectionId_);
      auto reply = socket_.receive();
      if (!reply.success()) {
        EAR_LOGGER_ERROR(logger_, "Failed to start new control connection: {}",
                         reply.errorDescription());
        return;
      }
      auto payload = reply.payloadAs<ConnectionDetailsResponse>();
      auto streamEndpoint = payload.metadataEndpoint();
      EAR_LOGGER_DEBUG(logger_,
                       "Received {} as target endpoint for metadata streaming",
                       streamEndpoint);
      connected_ = true;

      if (connectedCallback_) {
        connectedCallback_(connectionId_, streamEndpoint);
      }
    }
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_, "Exception during handshake: {}", e.what());
  }
}

void InputControlConnection::start(const std::string& endpoint) {
  EAR_LOGGER_INFO(logger_, "Connecting to {}", endpoint);
  socket_.open(endpoint);
}

void InputControlConnection::stop() { disconnect(); }

void InputControlConnection::connected() {
  EAR_LOGGER_DEBUG(logger_, "Now connected to scene master");
  handshake();
}

void InputControlConnection::disconnected() {
  EAR_LOGGER_WARN(logger_, "Lost connection to scene master");
  connected_ = false;
  if (disconnectedCallback_) {
    disconnectedCallback_();
  }
}

void InputControlConnection::disconnect() {
  if (connected_) {
    socket_.requestCloseConnection(connectionId_);
    connected_ = false;
    auto reply = socket_.receive();
    if (!reply.success()) {
      EAR_LOGGER_ERROR(logger_, "Failed to start close control connection: {}",
                       reply.errorDescription());
      return;
    } else {
      disconnected();
    }
  }
}

void InputControlConnection::onConnectionEstablished(
    ConnectionEstablishedHandler callback) {
  connectedCallback_ = callback;
}

void InputControlConnection::onConnectionLost(ConnectionLostHandler callback) {
  disconnectedCallback_ = callback;
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
