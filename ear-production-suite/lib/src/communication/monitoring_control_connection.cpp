#include "communication/monitoring_control_connection.hpp"
#include "communication/commands.hpp"
#include "log.hpp"
//#include <spdlog/spdlog.h>
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

MonitoringControlConnection::~MonitoringControlConnection() { disconnect(); }

void MonitoringControlConnection::logger(
    std::shared_ptr<spdlog::logger> logger) {
  logger_ = logger;
}

void MonitoringControlConnection::handshake() {
  try {
    {
      EAR_LOGGER_TRACE(logger_, "Requesting connection ID ({})",
                       connectionId_.string());
      NewConnectionMessage request{ConnectionType::MONITORING, connectionId_};
      auto sendBuffer = serialize(request);
      socket_.send(sendBuffer);
      auto buffer = socket_.read();

      auto resp = parseResponse(buffer);
      if (!resp.success()) {
        EAR_LOGGER_ERROR(logger_, "Failed to start new control connection: {}",
                         resp.errorDescription());
        return;
      }
      auto payload = resp.payloadAs<NewConnectionResponse>();

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
      EAR_LOGGER_TRACE(logger_, "Sending monitoring connection details");
      MonitoringConnectionDetailsMessage request{connectionId_};
      auto sendBuffer = serialize(request);
      socket_.send(sendBuffer);
      auto buffer = socket_.read();
      auto resp = parseResponse(buffer);
      if (!resp.success()) {
        EAR_LOGGER_ERROR(logger_, "Failed to start new control connection: {}",
                         resp.errorDescription());
        return;
      }
      auto payload = resp.payloadAs<MonitoringConnectionDetailsResponse>();
      auto streamEndpoint = payload.metadataEndpoint();
      EAR_LOGGER_DEBUG(logger_,
                       "Received {} as metadata scene stream source endpoint",
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

void MonitoringControlConnection::start(const std::string& endpoint) {
  EAR_LOGGER_INFO(logger_, "Connecting to {}", endpoint);

  socket_.dial(endpoint.c_str(), nng::Flags::nonblock);
}

void MonitoringControlConnection::connected() {
  EAR_LOGGER_DEBUG(logger_, "Now connected to scene master");
  handshake();
}

void MonitoringControlConnection::disconnected() {
  EAR_LOGGER_WARN(logger_, "Lost connection to scene master");
  connected_ = false;
  if (disconnectedCallback_) {
    disconnectedCallback_();
  }
}

void MonitoringControlConnection::disconnect() {
  if (connected_) {
    EAR_LOGGER_DEBUG(logger_, "Disconnecting from scene master");
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
    }
  }
}

void MonitoringControlConnection::onConnectionEstablished(
    ConnectionEstablishedHandler callback) {
  connectedCallback_ = callback;
}
void MonitoringControlConnection::onConnectionLost(
    ConnectionLostHandler callback) {
  disconnectedCallback_ = callback;
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
