#include "communication/monitoring_control_connection.hpp"
#include "communication/commands.hpp"
#include "log.hpp"
// #include <spdlog/spdlog.h>
#include <functional>

using namespace std::chrono_literals;

namespace ear {
namespace plugin {
namespace communication {

MonitoringControlConnection::MonitoringControlConnection() : connected_(false) {
  socket_.setOpt(nng::options::RecvTimeout, 1000ms);
  socket_.setOpt(nng::options::SendTimeout, 100ms);
  socket_.setOpt(nng::options::ReconnectMinTime, 250ms);
  socket_.setOpt(nng::options::ReconnectMaxTime, 0ms);
  socket_.onPipeEvent(nng::PipeEvent::postAdd,
                      std::bind(&MonitoringControlConnection::connected, this));
  socket_.onPipeEvent(
      nng::PipeEvent::postRemove,
      std::bind(&MonitoringControlConnection::disconnected, this));
}

MonitoringControlConnection::~MonitoringControlConnection() {
  stop();
}

void MonitoringControlConnection::logger(
    std::shared_ptr<spdlog::logger> logger) {
  std::lock_guard<std::mutex> lock(stateMutex_);
  logger_ = logger;
}

void MonitoringControlConnection::handshake(std::uint64_t generation) {
  ConnectionId requestedId;
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (!pipeConnected_ || generation != negotiationGeneration_) {
      return;
    }
    requestedId = connectionId_;
  }

  EAR_LOGGER_TRACE(logger_, "Requesting connection ID ({})",
                   requestedId.string());
  try {
    auto sendBuffer = serialize(
        NewConnectionMessage{ConnectionType::MONITORING, requestedId});
    socket_.asyncRequest(sendBuffer, [this, generation](std::error_code ec,
                                                        nng::Message message) {
      handleNewConnectionResponse(ec, std::move(message), generation);
    });
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_, "Exception starting handshake: {}", e.what());
  }
}

void MonitoringControlConnection::handleNewConnectionResponse(
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
    auto resp = parseResponse(message);
    if (!resp.success()) {
      if (retryHandshake(generation)) {
        return;
      }
      EAR_LOGGER_ERROR(logger_, "Failed to start new control connection: {}",
                       resp.errorDescription());
      return;
    }
    auto payload = resp.payloadAs<NewConnectionResponse>();
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
      if (!pipeConnected_ || generation != negotiationGeneration_) {
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
    EAR_LOGGER_TRACE(logger_, "Sending monitoring connection details");
    auto sendBuffer =
        serialize(MonitoringConnectionDetailsMessage{connectionId});
    socket_.asyncRequest(sendBuffer, [this, generation](std::error_code ec,
                                                        nng::Message message) {
      handleConnectionDetailsResponse(ec, std::move(message), generation);
    });
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_, "Exception during handshake: {}", e.what());
  }
}

void MonitoringControlConnection::handleConnectionDetailsResponse(
    std::error_code ec, nng::Message message, std::uint64_t generation) {
  if (ec) {
    if (retryHandshake(generation) || ec.value() == NNG_ECANCELED) {
      return;
    }
    EAR_LOGGER_ERROR(logger_,
                     "Failed to request monitoring connection details: {}",
                     ec.message());
    return;
  }

  try {
    auto resp = parseResponse(message);
    if (!resp.success()) {
      if (retryHandshake(generation)) {
        return;
      }
      EAR_LOGGER_ERROR(logger_, "Failed to start new control connection: {}",
                       resp.errorDescription());
      return;
    }
    auto payload = resp.payloadAs<MonitoringConnectionDetailsResponse>();
    auto streamEndpoint = payload.metadataEndpoint();
    ConnectionId connectionId;
    ConnectionEstablishedHandler callback;
    bool staleResponse = false;
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      if (!pipeConnected_ || generation != negotiationGeneration_) {
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
                     "Received {} as metadata scene stream source endpoint",
                     streamEndpoint);

    if (callback) {
      callback(connectionId, streamEndpoint);
    }
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_, "Exception during handshake: {}", e.what());
  }
}

void MonitoringControlConnection::start(const std::string& endpoint) {
  EAR_LOGGER_INFO(logger_, "Connecting to {}", endpoint);

  socket_.dial(endpoint.c_str(), nng::Flags::nonblock);
}

void MonitoringControlConnection::stop() {
  socket_.stopPipeEvents();
  socket_.asyncStop();
}

void MonitoringControlConnection::connected() {
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

void MonitoringControlConnection::disconnected() {
  ConnectionLostHandler callback;
  {
    std::lock_guard<std::mutex> negotiationLock(negotiationMutex_);
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      pipeConnected_ = false;
      connected_ = false;
      ++negotiationGeneration_;
      callback = disconnectedCallback_;
    }
    socket_.asyncCancel();
  }
  EAR_LOGGER_WARN(logger_, "Lost connection to scene master");
  if (callback) {
    callback();
  }
}

bool MonitoringControlConnection::isConnected() const {
  std::lock_guard<std::mutex> lock(stateMutex_);
  return connected_;
}

bool MonitoringControlConnection::retryHandshake(
    std::uint64_t completedGeneration) {
  std::lock_guard<std::mutex> negotiationLock(negotiationMutex_);
  std::uint64_t generation;
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (!pipeConnected_ || completedGeneration == negotiationGeneration_) {
      return false;
    }
    generation = negotiationGeneration_;
  }
  handshake(generation);
  return true;
}

void MonitoringControlConnection::onConnectionEstablished(
    ConnectionEstablishedHandler callback) {
  std::lock_guard<std::mutex> lock(stateMutex_);
  connectedCallback_ = callback;
}
void MonitoringControlConnection::onConnectionLost(
    ConnectionLostHandler callback) {
  std::lock_guard<std::mutex> lock(stateMutex_);
  disconnectedCallback_ = callback;
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
