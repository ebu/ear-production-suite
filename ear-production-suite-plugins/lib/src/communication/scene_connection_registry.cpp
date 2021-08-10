#include "communication/scene_connection_registry.hpp"
#include <cassert>

namespace ear {
namespace plugin {
namespace communication {

ConnectionId SceneConnectionRegistry::add(ConnectionType connectionType,
                                          ConnectionId connectionId) {
  // assert(connections_.size() < std::numeric_limits<uint32_t>::max);
  if (connectionId.isValid()) {
    if (this->has(connectionId)) {
      // connection with the requested id already exists, reset to assign a new
      // one below
      connectionId = ConnectionId{};
    }
  }
  // if the connectionId was invalid form the beginning or has been reset above
  if (!connectionId.isValid()) {
    connectionId = ConnectionId::generate();
  }
  auto res = connections_.insert(
      {connectionId, Connection{connectionType, Connection::NEW}});
  if (!res.second) {
    throw std::runtime_error("failed to insert connection");
  }
  return connectionId;
}

bool SceneConnectionRegistry::has(ConnectionId connectionId) const {
  return connections_.find(connectionId) != connections_.end();
}

void SceneConnectionRegistry::remove(ConnectionId connectionId) {
  connections_.erase(connectionId);
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
