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
InputControlConnection::InputControlConnection() : handshakeState_(std::make_unique<InitState>(this)), connected_(false) {
  socket_.setOpt(nng::options::RecvTimeout, 1000ms);
  socket_.setOpt(nng::options::SendTimeout, 100ms);
  socket_.setOpt(nng::options::ReconnectMinTime, 250ms);
  socket_.setOpt(nng::options::ReconnectMaxTime, 0ms);
  socket_.onPipeEvent(nng::PipeEvent::postAdd,
                      std::bind(&InputControlConnection::connected, this));
  socket_.onPipeEvent(nng::PipeEvent::postRemove,
                      std::bind(&InputControlConnection::disconnected, this));
}

InputControlConnection::~InputControlConnection() { disconnect(); }

void InputControlConnection::logger(std::shared_ptr<spdlog::logger> logger) {
  logger_ = logger;
}

void InputControlConnection::setConnectionId(ConnectionId id, bool skipReset) {
  if (!skipReset && isConnected()) {
    disconnect();
  }
  connectionId_ = id;
  if(!skipReset) {
    handshake();
  }
}


void InputControlConnection::handshake() {
//  try {
//    {
//      EAR_LOGGER_TRACE(logger_, "Requesting connection ID ({})",
//                       connectionId_.string());
//      NewConnectionMessage request{ConnectionType::METADATA_INPUT,
//                                   connectionId_};
//      auto sendBuffer = serialize(request);
//      socket_.send(sendBuffer);
//      auto buffer = socket_.read();
//
//      auto resp = parseResponse(buffer);
//      if (!resp.success()) {
//        EAR_LOGGER_ERROR(logger_, "Failed to start new control connection: {}",
//                         resp.errorDescription());
//        return;
//      }
//      auto payload = resp.payloadAs<NewConnectionResponse>();
//
//      if (!payload.connectionId().isValid()) {
//        EAR_LOGGER_ERROR(logger_,
//                         "Failed to start new control connection: invalid "
//                         "connection id received");
//        return;
//      }
//
//      connectionId_ = payload.connectionId();
//      EAR_LOGGER_DEBUG(logger_, "Got connection ID {}", connectionId_.string());
//    }
//    {
//      EAR_LOGGER_TRACE(logger_, "Sending object connection details");
//      ObjectDetailsMessage request{connectionId_};
//      auto sendBuffer = serialize(request);
//      socket_.send(sendBuffer);
//      auto buffer = socket_.read();
//      auto resp = parseResponse(buffer);
//      if (!resp.success()) {
//        EAR_LOGGER_ERROR(logger_, "Failed to start new control connection: {}",
//                         resp.errorDescription());
//        return;
//      }
//      auto payloadVariant = resp.payload();
//      auto payload = boost::get<ConnectionDetailsResponse>(&payloadVariant);
//      if(!payload) {
//        ResponsePayloadPrinter printer;
//        boost::apply_visitor(printer, payloadVariant);
//        throw std::runtime_error("bad connection response");
//      }
//      auto streamEndpoint = payload->metadataEndpoint();
//      EAR_LOGGER_DEBUG(logger_,
//                       "Received {} as target endpoint for metadata streaming",
//                       streamEndpoint);
//      connected_ = true;
//
//      if (connectedCallback_) {
//        connectedCallback_(connectionId_, streamEndpoint);
//      }
//    }
//  } catch (const std::runtime_error& e) {
//    EAR_LOGGER_ERROR(logger_, "Exception during handshake: {}", e.what());
//  }
  std::lock_guard<std::mutex> lock{handshakeMutex_};
  handshakeState_ = std::make_unique<InitState>(this);
  try {
    while (!isConnected()) {
      handshakeState_->process();
    }
  } catch (const std::runtime_error& e) {
    EAR_LOGGER_ERROR(logger_, "Exception during handshake: {}", e.what());
  }
}

void InputControlConnection::start(const std::string& endpoint) {
  EAR_LOGGER_INFO(logger_, "Connecting to {}", endpoint);

  socket_.dial(endpoint.c_str(), nng::Flags::nonblock);
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
  std::lock_guard<std::mutex> lock(handshakeMutex_);
  if (connected_) {
    CloseConnectionMessage request{connectionId_};
    auto sendBuffer = serialize(request);
    socket_.send(sendBuffer);
    connected_ = false;
    auto buffer = socket_.read();
    auto resp = parseResponse(buffer);
    if (!resp.success()) {
      EAR_LOGGER_ERROR(logger_, "Failed to start close control connection: {}",
                       resp.errorDescription());
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
void InputControlConnection::send(MessageBuffer& buffer) {
  socket_.send(buffer);
}
Response InputControlConnection::receive() {
  auto buffer = socket_.read();
  return parseResponse(buffer);
}
void InputControlConnection::setState(std::unique_ptr<HandshakeState> state) {
  handshakeState_ = std::move(state);
}
void InputControlConnection::setConnected(const std::string& metadataEndpoint) {
  connected_ = true;
  if (connectedCallback_) {
    connectedCallback_(connectionId_, metadataEndpoint);
  }
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
