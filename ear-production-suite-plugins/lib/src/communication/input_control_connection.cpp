#include "communication/input_control_connection.hpp"
#include "communication/commands.hpp"
#include "log.hpp"
// #include <spdlog/spdlog.h>
#include <functional>
#include <chrono>

using namespace std::chrono_literals;

namespace ear {
namespace plugin {
namespace communication {
InputControlConnection::InputControlConnection(
    std::shared_ptr<spdlog::logger> logger)
    : logger_{std::move(logger)},
      socket_{[this] { connected(); },
              [this] {
                EAR_LOGGER_WARN(logger_, "Socket disconnect");
                disconnected();
              }},
      connected_(false) {}

InputControlConnection::~InputControlConnection() {
  stop();
  disconnect();
}

void InputControlConnection::logger(std::shared_ptr<spdlog::logger> logger) {
  logger_ = logger;
}

void InputControlConnection::setConnectionId(ConnectionId id) {
  std::unique_lock<std::mutex> negotiationLock(negotiationMutex_);
  bool wasConnected;
  bool pipeConnected;
  ConnectionId previousConnectionId;
  std::uint64_t generation;
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    wasConnected = connected_;
    pipeConnected = pipeConnected_;
    previousConnectionId = connectionId_;
    connectionId_ = id;
    generation = ++negotiationGeneration_;
    if (wasConnected) {
      reconfiguring_ = true;
    }
  }

  // An established connection can be closed immediately. During negotiation,
  // leave the current request in flight so its response is consumed and
  // discarded before the new generation reuses the request socket.
  if (wasConnected) {
    socket_.asyncCancel();
  }

  if (!wasConnected) {
    if (pipeConnected) {
      handshake(generation);
    }
    return;
  }

  // The close request is synchronous, so do not hold the negotiation mutex
  // while waiting for its response. Pipe callbacks will defer new handshakes
  // until reconfiguration is complete.
  negotiationLock.unlock();
  disconnect(previousConnectionId);
  negotiationLock.lock();

  bool shouldHandshake = false;
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    reconfiguring_ = false;
    shouldHandshake = pipeConnected_;
    generation = negotiationGeneration_;
  }
  if (shouldHandshake) {
    handshake(generation);
  }
}

ConnectionId InputControlConnection::getConnectionId() const {
  std::lock_guard<std::mutex> lock(stateMutex_);
  return connectionId_;
}

void InputControlConnection::handshake(std::uint64_t generation) {
  ConnectionId requestedId;
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (!pipeConnected_ || reconfiguring_ ||
        generation != negotiationGeneration_) {
      return;
    }
    requestedId = connectionId_;
  }

  EAR_LOGGER_TRACE(logger_, "Requesting connection ID ({})",
                   requestedId.string());
  try {
    socket_.asyncRequestNewConnection(
        requestedId,
        [this, generation](std::error_code ec, nng::Message message) {
          handleNewConnectionResponse(ec, std::move(message), generation);
        });
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_, "Exception starting handshake: {}", e.what());
  }
}

void InputControlConnection::handleNewConnectionResponse(
    std::error_code ec, nng::Message message, std::uint64_t generation) {
  if (ec) {
    if (retryHandshake(generation) || ec.value() == NNG_ECANCELED) {
      return;
    }
    EAR_LOGGER_ERROR(logger_, "Failed to request connection ID: {}",
                     ec.message());
    return;
  }

  try {
    auto reply = parseResponse(message);
    if (!reply.success()) {
      if (retryHandshake(generation)) {
        return;
      }
      EAR_LOGGER_ERROR(logger_, "Failed to start new control connection: {}",
                       reply.errorDescription());
      return;
    }
    auto payload = reply.payloadAs<NewConnectionResponse>();
    if (!payload.connectionId().isValid()) {
      if (retryHandshake(generation)) {
        return;
      }
      EAR_LOGGER_ERROR(logger_,
                       "Failed to start new control connection: invalid "
                       "connection id received");
      return;
    }

    ConnectionId connectionId = payload.connectionId();
    bool staleResponse = false;
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      if (!pipeConnected_ || reconfiguring_ ||
          generation != negotiationGeneration_) {
        staleResponse = true;
      } else {
        connectionId_ = connectionId;
      }
    }
    if (staleResponse) {
      retryHandshake(generation);
      return;
    }
    EAR_LOGGER_DEBUG(logger_, "Got connection ID {}", connectionId.string());
    EAR_LOGGER_TRACE(logger_, "Sending object connection details");
    socket_.asyncRequestObjectDetails(
        connectionId,
        [this, generation](std::error_code ec, nng::Message message) {
          handleObjectDetailsResponse(ec, std::move(message), generation);
        });
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_, "Exception during handshake: {}", e.what());
  }
}

void InputControlConnection::handleObjectDetailsResponse(
    std::error_code ec, nng::Message message, std::uint64_t generation) {
  if (ec) {
    if (retryHandshake(generation) || ec.value() == NNG_ECANCELED) {
      return;
    }
    EAR_LOGGER_ERROR(logger_, "Failed to request object connection details: {}",
                     ec.message());
    return;
  }

  try {
    auto reply = parseResponse(message);
    if (!reply.success()) {
      if (retryHandshake(generation)) {
        return;
      }
      EAR_LOGGER_ERROR(logger_, "Failed to start new control connection: {}",
                       reply.errorDescription());
      return;
    }
    auto payload = reply.payloadAs<ConnectionDetailsResponse>();
    auto streamEndpoint = payload.metadataEndpoint();
    ConnectionId connectionId;
    ConnectionEstablishedHandler callback;
    bool staleResponse = false;
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      if (!pipeConnected_ || reconfiguring_ ||
          generation != negotiationGeneration_) {
        staleResponse = true;
      } else {
        connected_ = true;
        connectionId = connectionId_;
        callback = connectedCallback_;
      }
    }
    if (staleResponse) {
      retryHandshake(generation);
      return;
    }
    EAR_LOGGER_DEBUG(logger_,
                     "Received {} as target endpoint for metadata streaming",
                     streamEndpoint);

    if (callback) {
      callback(connectionId, streamEndpoint);
    } else {
      EAR_LOGGER_WARN(logger_, "Connected with {} but no callback provided",
                      connectionId.string());
    }
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_, "Exception during handshake: {}", e.what());
  }
}

void InputControlConnection::start(const std::string& endpoint) {
  EAR_LOGGER_INFO(logger_, "Connecting to {}", endpoint);
  socket_.open(endpoint);
}

void InputControlConnection::stop() {
  socket_.stopPipeEvents();
  socket_.asyncStop();
}

void InputControlConnection::connected() {
  std::uint64_t generation;
  {
    std::lock_guard<std::mutex> negotiationLock(negotiationMutex_);
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      pipeConnected_ = true;
      connected_ = false;
      generation = ++negotiationGeneration_;
    }
    socket_.asyncCancel();
    EAR_LOGGER_DEBUG(logger_, "Now connected to scene master");
    handshake(generation);
  }
}

void InputControlConnection::disconnected() {
  ConnectionLostHandler callback;
  ConnectionId connectionId;
  {
    std::lock_guard<std::mutex> negotiationLock(negotiationMutex_);
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      pipeConnected_ = false;
      connected_ = false;
      ++negotiationGeneration_;
      callback = disconnectedCallback_;
      connectionId = connectionId_;
    }
    socket_.asyncCancel();
  }

  EAR_LOGGER_TRACE(logger_, "Lost connection to scene master");
  if (callback) {
    callback();
  } else {
    EAR_LOGGER_WARN(logger_, "Disconnected from {} but no callback provided",
                    connectionId.string());
  }
}

void InputControlConnection::disconnect(ConnectionId connectionId) {
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (!connected_) {
      return;
    }
    connected_ = false;
    if (!connectionId.isValid()) {
      connectionId = connectionId_;
    }
  }

  socket_.requestCloseConnection(connectionId);
  auto reply = socket_.receive();
  if (!reply.success()) {
    EAR_LOGGER_ERROR(logger_, "Failed to start close control connection: {}",
                     reply.errorDescription());
    return;
  }

  EAR_LOGGER_TRACE(logger_, "Disconnect successfully completed");
  ConnectionLostHandler callback;
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    callback = disconnectedCallback_;
  }
  if (callback) {
    callback();
  } else {
    EAR_LOGGER_WARN(logger_, "Disconnected from {} but no callback provided",
                    connectionId.string());
  }
}

bool InputControlConnection::retryHandshake(std::uint64_t completedGeneration) {
  std::lock_guard<std::mutex> negotiationLock(negotiationMutex_);
  std::uint64_t generation;
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (!pipeConnected_ || reconfiguring_ ||
        completedGeneration == negotiationGeneration_) {
      return false;
    }
    generation = negotiationGeneration_;
  }
  handshake(generation);
  return true;
}

void InputControlConnection::onConnectionEstablished(
    ConnectionEstablishedHandler callback) {
  std::lock_guard<std::mutex> lock(stateMutex_);
  connectedCallback_ = std::move(callback);
}

void InputControlConnection::onConnectionLost(ConnectionLostHandler callback) {
  std::lock_guard<std::mutex> lock(stateMutex_);
  disconnectedCallback_ = std::move(callback);
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
