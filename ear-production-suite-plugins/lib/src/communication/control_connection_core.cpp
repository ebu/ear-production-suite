#include "communication/control_connection_core.hpp"

#include "log.hpp"

#include <stdexcept>
#include <utility>

namespace ear {
namespace plugin {
namespace communication {

namespace {

bool isNngError(const std::system_error& error) {
  return error.code().category() ==
         nng::makeErrorCode(error.code().value()).category();
}

bool isExpectedCloseError(const std::system_error& error) {
  if (!isNngError(error)) {
    return false;
  }

  switch (error.code().value()) {
    case NNG_ECANCELED:
    case NNG_ECLOSED:
    case NNG_ECONNABORTED:
    case NNG_ECONNRESET:
    case NNG_ECONNSHUT:
    case NNG_ESTATE:
    case NNG_ETIMEDOUT:
      return true;
    default:
      return false;
  }
}

}  // namespace

ControlConnectionCore::ControlConnectionCore(
    Operations operations, ConnectionType connectionType,
    std::string detailsType, DetailsRequestFactory detailsRequestFactory,
    DetailsResponseParser detailsResponseParser,
    std::shared_ptr<spdlog::logger> logger, LoggingOptions loggingOptions)
    : operations_(std::move(operations)),
      connectionType_(connectionType),
      detailsType_(std::move(detailsType)),
      detailsRequestFactory_(std::move(detailsRequestFactory)),
      detailsResponseParser_(std::move(detailsResponseParser)),
      loggingOptions_(loggingOptions),
      logger_(std::move(logger)) {}

ControlConnectionCore::~ControlConnectionCore() { stop(); }

void ControlConnectionCore::logger(std::shared_ptr<spdlog::logger> logger) {
  std::lock_guard<std::mutex> lock(stateMutex_);
  logger_ = std::move(logger);
}

std::shared_ptr<spdlog::logger> ControlConnectionCore::logger() const {
  return loggerSnapshot();
}

std::shared_ptr<spdlog::logger> ControlConnectionCore::loggerSnapshot() const {
  std::lock_guard<std::mutex> lock(stateMutex_);
  return logger_;
}

void ControlConnectionCore::start(const std::string& endpoint) {
  auto logger = loggerSnapshot();
  EAR_LOGGER_INFO(logger, "Connecting to {}", endpoint);
  operations_.start(endpoint);
}

void ControlConnectionCore::stop() {
  {
    std::lock_guard<std::mutex> negotiationLock(negotiationMutex_);
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      if (stopped_) {
        return;
      }
      pipeConnected_ = false;
      connected_ = false;
      reconfiguring_ = false;
      stopped_ = true;
      ++negotiationGeneration_;
    }
  }

  operations_.stop();
}

void ControlConnectionCore::connected() {
  std::uint64_t generation;
  {
    std::lock_guard<std::mutex> negotiationLock(negotiationMutex_);
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      if (stopped_) {
        return;
      }
      pipeConnected_ = true;
      connected_ = false;
      generation = ++negotiationGeneration_;
    }
    operations_.cancel();
    auto logger = loggerSnapshot();
    EAR_LOGGER_DEBUG(logger, "Now connected to scene master");
    handshake(generation);
  }
}

void ControlConnectionCore::disconnected() {
  ConnectionId connectionId;
  {
    std::lock_guard<std::mutex> negotiationLock(negotiationMutex_);
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      if (stopped_) {
        return;
      }
      pipeConnected_ = false;
      connected_ = false;
      ++negotiationGeneration_;
      connectionId = connectionId_;
    }
    operations_.cancel();
  }

  auto logger = loggerSnapshot();
  if (loggingOptions_.warnOnDisconnect) {
    EAR_LOGGER_WARN(logger, "Lost connection to scene master");
  } else {
    EAR_LOGGER_TRACE(logger, "Lost connection to scene master");
  }
  notifyConnectionLost(connectionId);
}

void ControlConnectionCore::setConnectionId(
    ConnectionId id, CloseConnectionHandler closeConnection) {
  if (!closeConnection) {
    throw std::invalid_argument("close connection handler is required");
  }

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
      connected_ = false;
    }
  }

  bool reconfigurationActive = wasConnected;
  try {
    if (wasConnected) {
      operations_.cancel();
    }

    if (!wasConnected) {
      if (pipeConnected) {
        handshake(generation);
      }
      return;
    }

    // The pipe callback must be able to update the state while the
    // synchronous close request is in flight.
    negotiationLock.unlock();

    bool shouldCloseConnection = false;
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      shouldCloseConnection = pipeConnected_ && !stopped_;
    }

    bool disconnectedSuccessfully = false;
    if (shouldCloseConnection) {
      try {
        disconnectedSuccessfully = closeConnection(previousConnectionId);
      } catch (const std::system_error& error) {
        if (!isExpectedCloseError(error)) {
          throw;
        }
        auto logger = loggerSnapshot();
        EAR_LOGGER_WARN(
            logger,
            "Close control connection interrupted by NNG state change: {}",
            error.code().message());
      }
    }

    if (disconnectedSuccessfully) {
      auto logger = loggerSnapshot();
      EAR_LOGGER_TRACE(logger, "Disconnect successfully completed");
      notifyConnectionLost(previousConnectionId);
    }

    negotiationLock.lock();

    bool shouldHandshake = false;
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      reconfiguring_ = false;
      reconfigurationActive = false;
      shouldHandshake = pipeConnected_ && !stopped_;
      generation = negotiationGeneration_;
    }
    if (shouldHandshake) {
      handshake(generation);
    }
  } catch (...) {
    // A close handler may perform synchronous NNG operations. Do not leave
    // future handshakes blocked if one of those operations fails.
    if (reconfigurationActive) {
      if (!negotiationLock.owns_lock()) {
        negotiationLock.lock();
      }
      std::lock_guard<std::mutex> lock(stateMutex_);
      reconfiguring_ = false;
    }
    throw;
  }
}

ConnectionId ControlConnectionCore::getConnectionId() const {
  std::lock_guard<std::mutex> lock(stateMutex_);
  return connectionId_;
}

bool ControlConnectionCore::isConnected() const {
  std::lock_guard<std::mutex> lock(stateMutex_);
  return connected_;
}

void ControlConnectionCore::onConnectionEstablished(
    ConnectionEstablishedHandler callback) {
  std::lock_guard<std::mutex> lock(stateMutex_);
  connectedCallback_ = std::move(callback);
}

void ControlConnectionCore::onConnectionLost(ConnectionLostHandler callback) {
  std::lock_guard<std::mutex> lock(stateMutex_);
  disconnectedCallback_ = std::move(callback);
}

void ControlConnectionCore::handshake(std::uint64_t generation) {
  ConnectionId requestedId;
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (!pipeConnected_ || reconfiguring_ || stopped_ ||
        generation != negotiationGeneration_) {
      return;
    }
    requestedId = connectionId_;
  }

  auto logger = loggerSnapshot();
  EAR_LOGGER_TRACE(logger, "Requesting connection ID ({})",
                   requestedId.string());
  try {
    auto sendBuffer =
        serialize(NewConnectionMessage{connectionType_, requestedId});
    operations_.request(sendBuffer, [this, generation](std::error_code ec,
                                                       nng::Message message) {
      handleNewConnectionResponse(ec, std::move(message), generation);
    });
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger, "Exception starting handshake: {}", e.what());
  }
}

void ControlConnectionCore::handleNewConnectionResponse(
    std::error_code ec, nng::Message message, std::uint64_t generation) {
  auto logger = loggerSnapshot();
  if (ec) {
    if (retryHandshake(generation) || ec.value() == NNG_ECANCELED) {
      return;
    }
    EAR_LOGGER_ERROR(logger, "Failed to request connection ID: {}",
                     ec.message());
    return;
  }

  try {
    auto response = parseResponse(message);
    if (!response.success()) {
      if (retryHandshake(generation)) {
        return;
      }
      EAR_LOGGER_ERROR(logger, "Failed to start new control connection: {}",
                       response.errorDescription());
      return;
    }
    auto payload = response.payloadAs<NewConnectionResponse>();
    if (!payload.connectionId().isValid()) {
      if (retryHandshake(generation)) {
        return;
      }
      EAR_LOGGER_ERROR(
          logger,
          "Failed to start new control connection: invalid connection id "
          "received");
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

    EAR_LOGGER_DEBUG(logger, "Got connection ID {}", connectionId.string());
    EAR_LOGGER_TRACE(logger, "Sending {} connection details", detailsType_);
    auto sendBuffer = detailsRequestFactory_(connectionId);
    operations_.request(sendBuffer, [this, generation](std::error_code ec,
                                                       nng::Message message) {
      handleDetailsResponse(ec, std::move(message), generation);
    });
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger, "Exception during handshake: {}", e.what());
  }
}

void ControlConnectionCore::handleDetailsResponse(std::error_code ec,
                                                  nng::Message message,
                                                  std::uint64_t generation) {
  auto logger = loggerSnapshot();
  if (ec) {
    if (retryHandshake(generation) || ec.value() == NNG_ECANCELED) {
      return;
    }
    EAR_LOGGER_ERROR(logger, "Failed to request {} connection details: {}",
                     detailsType_, ec.message());
    return;
  }

  try {
    auto response = parseResponse(message);
    if (!response.success()) {
      if (retryHandshake(generation)) {
        return;
      }
      EAR_LOGGER_ERROR(logger, "Failed to start new control connection: {}",
                       response.errorDescription());
      return;
    }

    auto streamEndpoint = detailsResponseParser_(response);
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

    EAR_LOGGER_DEBUG(logger, "Received {} as {} metadata endpoint",
                     streamEndpoint, detailsType_);
    if (callback) {
      dispatchCallback([callback = std::move(callback), connectionId,
                        streamEndpoint = std::move(streamEndpoint),
                        logger]() mutable {
        try {
          callback(connectionId, std::move(streamEndpoint));
        } catch (const std::runtime_error& error) {
          EAR_LOGGER_ERROR(
              logger, "Exception during connection established callback: {}",
              error.what());
        }
      });
    } else if (loggingOptions_.warnWhenNoConnectionEstablishedCallback) {
      EAR_LOGGER_WARN(logger, "Connected with {} but no callback provided",
                      connectionId.string());
    }
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger, "Exception during handshake: {}", e.what());
  }
}

bool ControlConnectionCore::retryHandshake(std::uint64_t completedGeneration) {
  std::lock_guard<std::mutex> negotiationLock(negotiationMutex_);
  std::uint64_t generation;
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (!pipeConnected_ || reconfiguring_ || stopped_ ||
        completedGeneration == negotiationGeneration_) {
      return false;
    }
    generation = negotiationGeneration_;
  }
  handshake(generation);
  return true;
}

void ControlConnectionCore::notifyConnectionLost(ConnectionId connectionId) {
  ConnectionLostHandler callback;
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    callback = disconnectedCallback_;
  }
  if (callback) {
    dispatchCallback([callback = std::move(callback)] { callback(); });
  } else if (loggingOptions_.warnWhenNoConnectionLostCallback) {
    auto logger = loggerSnapshot();
    EAR_LOGGER_WARN(logger, "Disconnected from {} but no callback provided",
                    connectionId.string());
  }
}

void ControlConnectionCore::dispatchCallback(std::function<void()> callback) {
  if (!operations_.dispatch) {
    callback();
    return;
  }

  if (!operations_.dispatch(std::move(callback))) {
    auto logger = loggerSnapshot();
    EAR_LOGGER_DEBUG(logger, "Discarding connection callback during shutdown");
  }
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
