#include "communication/monitoring_control_connection.hpp"

#include "communication/commands.hpp"

#include <chrono>
#include <utility>

using namespace std::chrono_literals;

namespace ear {
namespace plugin {
namespace communication {

MonitoringControlConnection::MonitoringControlConnection()
    : socket_{},
      core_{ControlConnectionCore::Operations{
                [this](const std::string& endpoint) {
                  socket_.dial(endpoint.c_str(), nng::Flags::nonblock);
                },
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
            ConnectionType::MONITORING,
            "monitoring",
            [](ConnectionId id) {
              return serialize(MonitoringConnectionDetailsMessage{id});
            },
            [](const Response& response) {
              return response.payloadAs<MonitoringConnectionDetailsResponse>()
                  .metadataEndpoint();
            },
            nullptr,
            ControlConnectionCore::LoggingOptions{true, false, false}} {
  socket_.setOpt(nng::options::RecvTimeout, 1000ms);
  socket_.setOpt(nng::options::SendTimeout, 100ms);
  socket_.setOpt(nng::options::ReconnectMinTime, 250ms);
  socket_.setOpt(nng::options::ReconnectMaxTime, 0ms);
  socket_.onPipeEvent(nng::PipeEvent::postAdd,
                      [this](nng::Pipe, nng::PipeEvent) { core_.connected(); });
  socket_.onPipeEvent(
      nng::PipeEvent::postRemove,
      [this](nng::Pipe, nng::PipeEvent) { core_.disconnected(); });
}

MonitoringControlConnection::~MonitoringControlConnection() { stop(); }

void MonitoringControlConnection::logger(
    std::shared_ptr<spdlog::logger> logger) {
  core_.logger(std::move(logger));
}

void MonitoringControlConnection::start(const std::string& endpoint) {
  core_.start(endpoint);
}

void MonitoringControlConnection::stop() { core_.stop(); }

bool MonitoringControlConnection::isConnected() const {
  return core_.isConnected();
}

void MonitoringControlConnection::onConnectionEstablished(
    ConnectionEstablishedHandler callback) {
  core_.onConnectionEstablished(std::move(callback));
}

void MonitoringControlConnection::onConnectionLost(
    ConnectionLostHandler callback) {
  core_.onConnectionLost(std::move(callback));
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
