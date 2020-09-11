#pragma once
#include "nng-cpp/nng.hpp"
#include "log.hpp"
#include "communication/common_types.hpp"
#include <functional>

namespace ear {
namespace plugin {
namespace communication {

enum class ErrorCode;
class Response;
class Request;

/**
 * @brief Connect an monitoring plugin to a scene master
 *
 * This class initiates a connection to a `Scene`
 * and negotiates connection details on how to receive scene metadata which
 * should be rendered.
 *
 * Scene Connection/Disconnection events can be monitored by registering
 * callbacks using `onConnectionEstablished()` and `onConnectionLost()`.
 * The registered callbacks will be called by a thread of the internal
 * communication event loop.
 *
 */
class MonitoringControlConnection {
 public:
  using ConnectionEstablishedHandler =
      std::function<void(communication::ConnectionId, std::string)>;
  using ConnectionLostHandler = std::function<void()>;

  EAR_PLUGIN_BASE_EXPORT MonitoringControlConnection();
  MonitoringControlConnection(const MonitoringControlConnection&) = delete;
  MonitoringControlConnection& operator=(const MonitoringControlConnection&) =
      delete;
  MonitoringControlConnection(MonitoringControlConnection&&) = delete;
  MonitoringControlConnection& operator=(MonitoringControlConnection&&) =
      delete;

  EAR_PLUGIN_BASE_EXPORT ~MonitoringControlConnection();

  void start(const std::string& endpoint);
  void logger(std::shared_ptr<spdlog::logger> logger);

  void onConnectionEstablished(ConnectionEstablishedHandler callback);
  void onConnectionLost(ConnectionLostHandler callback);

  bool isConnected() { return connected_; }

 private:
  void connected();
  void disconnected();
  void handshake();
  void disconnect();
  nng::ReqSocket socket_;
  std::shared_ptr<spdlog::logger> logger_;
  ConnectionId connectionId_;
  bool connected_;
  ConnectionEstablishedHandler connectedCallback_;
  ConnectionLostHandler disconnectedCallback_;
};
}  // namespace communication
}  // namespace plugin
}  // namespace ear
