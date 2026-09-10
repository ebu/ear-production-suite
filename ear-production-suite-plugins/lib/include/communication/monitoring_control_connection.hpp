#pragma once
#include "communication/control_connection_core.hpp"

namespace ear {
namespace plugin {
namespace communication {

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
      ControlConnectionCore::ConnectionEstablishedHandler;
  using ConnectionLostHandler = ControlConnectionCore::ConnectionLostHandler;

  EAR_PLUGIN_BASE_EXPORT MonitoringControlConnection();
  MonitoringControlConnection(const MonitoringControlConnection&) = delete;
  MonitoringControlConnection& operator=(const MonitoringControlConnection&) =
      delete;
  MonitoringControlConnection(MonitoringControlConnection&&) = delete;
  MonitoringControlConnection& operator=(MonitoringControlConnection&&) =
      delete;

  EAR_PLUGIN_BASE_EXPORT ~MonitoringControlConnection();

  void start(const std::string& endpoint);
  void stop();
  void logger(std::shared_ptr<spdlog::logger> logger);

  void onConnectionEstablished(ConnectionEstablishedHandler callback);
  void onConnectionLost(ConnectionLostHandler callback);

  bool isConnected() const;

 private:
  nng::ReqSocket socket_;
  ControlConnectionCore core_;
};
}  // namespace communication
}  // namespace plugin
}  // namespace ear
