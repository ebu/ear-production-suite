#pragma once

#include "commands.hpp"
#include <cstdint>
#include <map>
#include <memory>

namespace ear {
namespace plugin {
namespace communication {

struct Connection {
  enum State { NEW, ACTIVE };
  const communication::ConnectionType type;
  State state = NEW;
};

/**
 * @brief Manages incoming client connections to a scene master
 *
 * This class manages incoming connections to the scene master plugin:
 * Metadata input plugins register themselves so they can sent metadata for an
 * input element, receive updates and send metadata.
 */
class SceneConnectionRegistry {
 public:
  /**
   * @brief Register a new connection
   *
   * If the connectionId is zero, a new connectionId will be assigned for this
   * connection. Otherwise the connectionId will be reused if possible. If the
   * given connectionId is already in use, a new free on be assigned.
   *
   * @returns The connectionId assigned to the newly added connection
   */

  communication::ConnectionId add(
      communication::ConnectionType type,
      communication::ConnectionId connectionId = communication::ConnectionId{});
  /**
   * @brief Remove a connection
   *
   * If no connection with the given idea exists this method is a no-op.
   */
  void remove(communication::ConnectionId connectionId);

  /// Number of connection
  std::size_t size() const { return connections_.size(); }
  /**
   * Returns true if `connectionId` refers to a registered connection
   */
  bool has(communication::ConnectionId connectionId) const;

  /**
   * Access the details of a connection.
   *
   * @throws an exception if no connection with `connectionId` exists.
   */
  Connection& get(communication::ConnectionId connectionId) {
    return connections_.at(connectionId);
  }
  /**
   * Access the details of a connection.
   *
   * @throws an exception if no connection with `connectionId` exists.
   */
  const Connection& get(communication::ConnectionId connectionId) const {
    return connections_.at(connectionId);
  }

 private:
  std::map<communication::ConnectionId, Connection> connections_;
};

}  // namespace communication
}  // namespace plugin
}  // namespace ear
