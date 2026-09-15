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

  auto change = prepareConnectionIdChange(id);
  if (!change.wasConnected) {
    return;
  }

  try {
    closeConnectionDuringReconfiguration(change, closeConnection);
    finishConnectionIdChange();
  } catch (...) {
    abortConnectionIdChange();
    throw;
  }
}

ControlConnectionCore::ConnectionIdChange
ControlConnectionCore::prepareConnectionIdChange(ConnectionId id) {
  ConnectionIdChange change;
  std::lock_guard<std::mutex> negotiationLock(negotiationMutex_);
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    change.wasConnected = connected_;
    change.pipeConnected = pipeConnected_;
    change.previousConnectionId = connectionId_;
    connectionId_ = id;
    change.generation = ++negotiationGeneration_;
    if (change.wasConnected) {
      reconfiguring_ = true;
      connected_ = false;
    }
  }

  if (!change.wasConnected) {
    if (change.pipeConnected) {
      handshake(change.generation);
    }
    return change;
  }

  try {
    operations_.cancel();
  } catch (...) {
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      reconfiguring_ = false;
    }
    throw;
  }

  return change;
}

void ControlConnectionCore::closeConnectionDuringReconfiguration(
    const ConnectionIdChange& change,
    const CloseConnectionHandler& closeConnection) {
  // Keep the negotiation mutex available: the synchronous close operation can
  // trigger a pipe callback that needs to update the connection state.
  bool shouldCloseConnection = false;
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    shouldCloseConnection = pipeConnected_ && !stopped_;
  }
  if (!shouldCloseConnection) {
    return;
  }

  bool disconnectedSuccessfully = false;
  try {
    disconnectedSuccessfully = closeConnection(change.previousConnectionId);
  } catch (const std::system_error& error) {
    if (!isExpectedCloseError(error)) {
      throw;
    }
    auto logger = loggerSnapshot();
    EAR_LOGGER_WARN(
        logger, "Close control connection interrupted by NNG state change: {}",
        error.code().message());
  }

  if (disconnectedSuccessfully) {
    auto logger = loggerSnapshot();
    EAR_LOGGER_TRACE(logger, "Disconnect successfully completed");
    notifyConnectionLost(change.previousConnectionId);
  }
}

void ControlConnectionCore::finishConnectionIdChange() {
  std::lock_guard<std::mutex> negotiationLock(negotiationMutex_);
  bool shouldHandshake = false;
  std::uint64_t generation;
  {
    std::lock_guard<std::mutex> lock(stateMutex_);
    reconfiguring_ = false;
    shouldHandshake = pipeConnected_ && !stopped_;
    generation = negotiationGeneration_;
  }
  if (shouldHandshake) {
    handshake(generation);
  }
}

void ControlConnectionCore::abortConnectionIdChange() {
  std::lock_guard<std::mutex> negotiationLock(negotiationMutex_);
  std::lock_guard<std::mutex> lock(stateMutex_);
  reconfiguring_ = false;
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
    if (shouldRetryOrIgnore(ec, generation)) {
      return;
    }
    EAR_LOGGER_ERROR(logger, "Failed to request connection ID: {}",
                     ec.message());
    return;
  }

  try {
    auto response = parseResponse(message);
    ConnectionId connectionId;
    if (!processNewConnectionResponse(response, generation, logger,
                                      connectionId)) {
      return;
    }
    requestConnectionDetails(connectionId, generation, logger);
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger, "Exception during handshake: {}", e.what());
  }
}

bool ControlConnectionCore::shouldRetryOrIgnore(std::error_code ec,
                                                std::uint64_t generation) {
  return retryHandshake(generation) || ec.value() == NNG_ECANCELED;
}

bool ControlConnectionCore::processNewConnectionResponse(
    const Response& response, std::uint64_t generation,
    const std::shared_ptr<spdlog::logger>& logger, ConnectionId& connectionId) {
  if (!response.success()) {
    if (retryHandshake(generation)) {
      return false;
    }
    EAR_LOGGER_ERROR(logger, "Failed to start new control connection: {}",
                     response.errorDescription());
    return false;
  }

  connectionId = response.payloadAs<NewConnectionResponse>().connectionId();
  if (!connectionId.isValid()) {
    if (retryHandshake(generation)) {
      return false;
    }
    EAR_LOGGER_ERROR(
        logger,
        "Failed to start new control connection: invalid connection id "
        "received");
    return false;
  }

  if (!storeConnectionIdIfCurrent(connectionId, generation)) {
    retryHandshake(generation);
    return false;
  }
  return true;
}

bool ControlConnectionCore::storeConnectionIdIfCurrent(
    ConnectionId connectionId, std::uint64_t generation) {
  std::lock_guard<std::mutex> lock(stateMutex_);
  if (!pipeConnected_ || reconfiguring_ ||
      generation != negotiationGeneration_) {
    return false;
  }
  connectionId_ = connectionId;
  return true;
}

void ControlConnectionCore::requestConnectionDetails(
    ConnectionId connectionId, std::uint64_t generation,
    const std::shared_ptr<spdlog::logger>& logger) {
  EAR_LOGGER_DEBUG(logger, "Got connection ID {}", connectionId.string());
  EAR_LOGGER_TRACE(logger, "Sending {} connection details", detailsType_);
  auto sendBuffer = detailsRequestFactory_(connectionId);
  operations_.request(
      sendBuffer, [this, generation](std::error_code ec, nng::Message message) {
        handleDetailsResponse(ec, std::move(message), generation);
      });
}

void ControlConnectionCore::handleDetailsResponse(std::error_code ec,
                                                  nng::Message message,
                                                  std::uint64_t generation) {
  auto logger = loggerSnapshot();
  if (ec) {
    if (shouldRetryOrIgnore(ec, generation)) {
      return;
    }
    EAR_LOGGER_ERROR(logger, "Failed to request {} connection details: {}",
                     detailsType_, ec.message());
    return;
  }

  try {
    auto response = parseResponse(message);
    std::string streamEndpoint;
    ConnectionId connectionId;
    ConnectionEstablishedHandler callback;
    if (!processDetailsResponse(response, generation, logger, streamEndpoint,
                                connectionId, callback)) {
      return;
    }

    EAR_LOGGER_DEBUG(logger, "Received {} as {} metadata endpoint",
                     streamEndpoint, detailsType_);
    notifyConnectionEstablished(connectionId, std::move(streamEndpoint),
                                std::move(callback), logger);
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger, "Exception during handshake: {}", e.what());
  }
}

bool ControlConnectionCore::processDetailsResponse(
    const Response& response, std::uint64_t generation,
    const std::shared_ptr<spdlog::logger>& logger, std::string& streamEndpoint,
    ConnectionId& connectionId, ConnectionEstablishedHandler& callback) {
  if (!response.success()) {
    if (retryHandshake(generation)) {
      return false;
    }
    EAR_LOGGER_ERROR(logger, "Failed to start new control connection: {}",
                     response.errorDescription());
    return false;
  }

  streamEndpoint = detailsResponseParser_(response);
  if (!establishConnectionIfCurrent(generation, connectionId, callback)) {
    retryHandshake(generation);
    return false;
  }
  return true;
}

bool ControlConnectionCore::establishConnectionIfCurrent(
    std::uint64_t generation, ConnectionId& connectionId,
    ConnectionEstablishedHandler& callback) {
  std::lock_guard<std::mutex> lock(stateMutex_);
  if (!pipeConnected_ || reconfiguring_ ||
      generation != negotiationGeneration_) {
    return false;
  }
  connected_ = true;
  connectionId = connectionId_;
  callback = connectedCallback_;
  return true;
}

void ControlConnectionCore::notifyConnectionEstablished(
    ConnectionId connectionId, std::string streamEndpoint,
    ConnectionEstablishedHandler callback,
    std::shared_ptr<spdlog::logger> logger) {
  if (callback) {
    dispatchCallback([callback = std::move(callback), connectionId,
                      streamEndpoint = std::move(streamEndpoint),
                      logger = std::move(logger)]() mutable {
      try {
        callback(connectionId, std::move(streamEndpoint));
      } catch (const std::runtime_error& error) {
        EAR_LOGGER_ERROR(logger,
                         "Exception during connection established callback: {}",
                         error.what());
      }
    });
  } else if (loggingOptions_.warnWhenNoConnectionEstablishedCallback) {
    EAR_LOGGER_WARN(logger, "Connected with {} but no callback provided",
                    connectionId.string());
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
