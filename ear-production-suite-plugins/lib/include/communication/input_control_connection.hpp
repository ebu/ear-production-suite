#pragma once
#include "nng-cpp/nng.hpp"
#include "log.hpp"
#include "communication/common_types.hpp"
#include "communication/input_control_socket.hpp"
#include "ui/item_colour.hpp"
#include <functional>
#include <cstdint>
#include <mutex>
#include <system_error>

namespace ear {
namespace plugin {
namespace communication {

enum class ErrorCode;
class Response;
class Request;

/**
 * @brief Connect an input plugin to a scene master (stub)
 *
 * This class initiates a connection to a `Scene`.
 *
 * This is currently a stub to test the connection management,
 * it should/must be
 *   - split into multiple components
 *   - differntiate between the different input types (objects, directspeakers)
 *   - provide some interface to configure/reconfigure/set "static" information
 * about this input (number of channels, speaker channel layout, track
 * allocation)
 *
 * This stub will probably be turned into a subcomponent of a to-be-written
 * class which will act as a counterpart to the `Scene` on the input side.
 */
class InputControlConnection {
 public:
  using ConnectionEstablishedHandler =
      std::function<void(ConnectionId, std::string)>;
  using ConnectionLostHandler = std::function<void()>;

  EAR_PLUGIN_BASE_EXPORT explicit InputControlConnection(
      std::shared_ptr<spdlog::logger> logger);
  InputControlConnection(const InputControlConnection&) = delete;
  InputControlConnection& operator=(const InputControlConnection&) = delete;
  InputControlConnection(InputControlConnection&&) = delete;
  InputControlConnection& operator=(InputControlConnection&&) = delete;

  EAR_PLUGIN_BASE_EXPORT ~InputControlConnection();

  void start(const std::string& endpoint);
  void stop();

  // Note: this will disconnect if connected and manually restarting will be
  // necessary
  void setConnectionId(ConnectionId id);
  ConnectionId getConnectionId() const;

  void logger(std::shared_ptr<spdlog::logger> logger);

  void onConnectionEstablished(ConnectionEstablishedHandler callback);
  void onConnectionLost(ConnectionLostHandler callback);

 private:
  struct CachedItemProperties {
    std::string name;
    ui::ItemColour colour;
  };
  void connected();
  void disconnected();
  void handshake(std::uint64_t generation);
  void handleNewConnectionResponse(std::error_code ec, nng::Message message,
                                   std::uint64_t generation);
  void handleObjectDetailsResponse(std::error_code ec, nng::Message message,
                                   std::uint64_t generation);
  bool retryHandshake(std::uint64_t completedGeneration);
  void disconnect(ConnectionId connectionId = {});
  std::mutex negotiationMutex_;
  mutable std::mutex stateMutex_;
  std::shared_ptr<spdlog::logger> logger_;
  InputControlSocket socket_;
  ConnectionId connectionId_;
  bool connected_;
  bool pipeConnected_{false};
  bool reconfiguring_{false};
  std::uint64_t negotiationGeneration_{0};
  CachedItemProperties cachedItemProperties_;
  ConnectionEstablishedHandler connectedCallback_;
  ConnectionLostHandler disconnectedCallback_;
};
}  // namespace communication
}  // namespace plugin
}  // namespace ear
