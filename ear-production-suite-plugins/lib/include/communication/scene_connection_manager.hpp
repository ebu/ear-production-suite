#pragma once

#include "scene_connection_registry.hpp"
#include "communication/common_types.hpp"

namespace ear {
namespace plugin {
namespace communication {
class Response;
class Request;

/**
 * @brief Control command handler and plugin connection manager
 *
 * This class manages all incoming control connections from input plugins
 * and monitoring plugins.
 *
 * Users of this class can register a event handler to get notified about
 * certain events, currently being
 *   - a new input item (plugin) was added
 *   - the connection to an input item was lost
 *
 * Apart from simply managing the connections, it is envisioned to store and
 * cache additional connection metadata within this class.
 * This might include the connection type (object, directSpeakers), number of
 * channels, track allocation.
 *
 * One important part will be that plugins can re-register with a previously
 * assigned connectionId, which enables them to reuse already stored connection
 * data (i.e. track allocation). This might be usefull to ensure that we don't
 * "loose" information and force the user to re-configure things (i.e. when
 * removing the scene master plugin and adding it again)
 *
 *
 * @sa SceneCommandReceiver
 * @sa InputConnectionRegistry
 */
class SceneConnectionManager {
 public:
  enum class Event {
    INPUT_ADDED,
    INPUT_REMOVED,
    MONITORING_ADDED,
    MONITORING_REMOVED
  };

  using EventCallback = std::function<void(Event, communication::ConnectionId)>;
  void setEventHandler(EventCallback callback) { callback_ = callback; }

  communication::Response handle(const communication::Request& request);

  const Connection& connectionInfo(communication::ConnectionId id) const {
    return inputConnections_.get(id);
  }

 private:
  void notify(Event eventType, communication::ConnectionId id) const;

  struct RequestDispatcher;
  communication::NewConnectionResponse doHandle(
      const communication::NewConnectionMessage&);
  communication::ConnectionDetailsResponse doHandle(
      const communication::ObjectDetailsMessage&);
  communication::MonitoringConnectionDetailsResponse doHandle(
      const communication::MonitoringConnectionDetailsMessage&);
  communication::CloseConnectionResponse doHandle(
      const communication::CloseConnectionMessage&);
  template <typename T>
  communication::Response doHandle(
      const T& /*catch-all-remove-me-later-or-make-an-static-assert*/) {
    return communication::Response(communication::ErrorCode::UNKOWN_ERROR,
                                   "not-implemented-yet-error");
  }

  EventCallback callback_;
  SceneConnectionRegistry inputConnections_;
};

}  // namespace communication
}  // namespace plugin
}  // namespace ear
