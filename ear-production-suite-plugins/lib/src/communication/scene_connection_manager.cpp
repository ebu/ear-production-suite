#include "communication/scene_connection_manager.hpp"
#include "communication/commands.hpp"
#include "log.hpp"
#include "detail/constants.hpp"
//#include <spdlog/spdlog.h>

namespace ear {
namespace plugin {
namespace communication {

struct SceneConnectionManager::RequestDispatcher
    : public boost::static_visitor<communication::Response> {
  RequestDispatcher(SceneConnectionManager* manager) : manager_(manager) {}

  template <typename T>
  communication::Response operator()(const T& payload) const {
    try {
      auto responsePayload = manager_->doHandle(payload);
      return communication::Response{responsePayload};
    } catch (const std::runtime_error& e) {
      return communication::Response{communication::ErrorCode::UNKOWN_ERROR,
                                     e.what()};
    }
  }

 private:
  SceneConnectionManager* manager_;
};

communication::Response SceneConnectionManager::handle(
    const communication::Request& request) {
  return boost::apply_visitor(RequestDispatcher(this), request);
}

communication::NewConnectionResponse SceneConnectionManager::doHandle(
    const communication::NewConnectionMessage& message) {
  auto assignedId =
      inputConnections_.add(message.type(), message.connectionId());
  return communication::NewConnectionResponse{assignedId};
}

communication::CloseConnectionResponse SceneConnectionManager::doHandle(
    const communication::CloseConnectionMessage& message) {
  if (!inputConnections_.has(message.connectionId())) {
    throw std::runtime_error("unkown connection id");
  }
  auto type = inputConnections_.get(message.connectionId()).type;
  inputConnections_.remove(message.connectionId());
  if (type == communication::ConnectionType::METADATA_INPUT) {
    notify(Event::INPUT_REMOVED, message.connectionId());
  } else {
    notify(Event::MONITORING_REMOVED, message.connectionId());
  }
  return communication::CloseConnectionResponse(message.connectionId());
}

communication::ConnectionDetailsResponse SceneConnectionManager::doHandle(
    const communication::ObjectDetailsMessage& message) {
  if (!inputConnections_.has(message.connectionId())) {
    throw std::runtime_error("unkown connection id");
  }
  if (inputConnections_.get(message.connectionId()).state != Connection::NEW) {
    throw std::runtime_error("Connection already configured");
  }
  if (inputConnections_.get(message.connectionId()).type !=
      communication::ConnectionType::METADATA_INPUT) {
    throw std::runtime_error("Connection type mismatch");
  }
  inputConnections_.get(message.connectionId()).state = Connection::ACTIVE;
  notify(Event::INPUT_ADDED, message.connectionId());
  return communication::ConnectionDetailsResponse(
      message.connectionId(), detail::SCENE_MASTER_METADATA_ENDPOINT);
}

communication::MonitoringConnectionDetailsResponse
SceneConnectionManager::doHandle(
    const communication::MonitoringConnectionDetailsMessage& message) {
  if (!inputConnections_.has(message.connectionId())) {
    throw std::runtime_error("unkown connection id");
  }
  if (inputConnections_.get(message.connectionId()).state != Connection::NEW) {
    throw std::runtime_error("Connection already configured");
  }
  if (inputConnections_.get(message.connectionId()).type !=
      communication::ConnectionType::MONITORING) {
    throw std::runtime_error("Connection type mismatch");
  }
  inputConnections_.get(message.connectionId()).state = Connection::ACTIVE;
  notify(Event::MONITORING_ADDED, message.connectionId());
  return communication::MonitoringConnectionDetailsResponse(
      message.connectionId(), detail::SCENE_MASTER_SCENE_STREAM_ENDPOINT);
}

void SceneConnectionManager::notify(SceneConnectionManager::Event event,
                                    communication::ConnectionId id) const {
  if (callback_) {
    callback_(event, id);
  }
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
