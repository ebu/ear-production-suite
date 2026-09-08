//
// Created by Richard Bailey on 18/02/2022.
//
#include <functional>
#include "ear-plugin-base/export.h"
#include "communication/commands.hpp"
#include "nng-cpp/nng.hpp"
#include "log.hpp"
#include <system_error>
#include <utility>

#ifndef EAR_PRODUCTION_SUITE_INPUT_CONTROL_SOCKET_HPP
#define EAR_PRODUCTION_SUITE_INPUT_CONTROL_SOCKET_HPP
namespace ear::plugin::communication {
EAR_PLUGIN_BASE_EXPORT class InputControlSocket {
 public:
  using AsyncResponseHandler =
      std::function<void(std::error_code, nng::Message)>;

  InputControlSocket(std::function<void()> const& connectedCallback,
                     std::function<void()> const& disconnectedCallback);
  ~InputControlSocket();

  void open(std::string const& endpoint);
  void requestNewConnection(ConnectionId const& id);
  void requestCloseConnection(ConnectionId const& id);
  void requestObjectDetails(ConnectionId const& id);
  bool asyncRequestNewConnection(const ConnectionId& id,
                                 AsyncResponseHandler handler);
  bool asyncRequestObjectDetails(const ConnectionId& id,
                                 AsyncResponseHandler handler);
  void asyncCancel();
  void asyncStop();
  void stopPipeEvents();

  [[nodiscard]]
  Response receive();

 private:
  template <typename MessageT>
  void send(MessageT const& message) {
    auto buffer = serialize(message);
    socket_.send(buffer);
  }

  template <typename MessageT>
  bool asyncRequest(MessageT const& message, AsyncResponseHandler handler) {
    auto buffer = serialize(message);
    return socket_.asyncRequest(buffer, std::move(handler));
  }

  std::shared_ptr<spdlog::logger> logger_;
  nng::ReqSocket socket_;
};
}  // namespace ear::plugin::communication

#endif  // EAR_PRODUCTION_SUITE_INPUT_CONTROL_SOCKET_HPP
