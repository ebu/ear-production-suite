//
// Created by Richard Bailey on 18/02/2022.
//
#include <functional>
#include "ear-plugin-base/export.h"
#include "communication/commands.hpp"
#include "nng-cpp/nng.hpp"

#ifndef EAR_PRODUCTION_SUITE_INPUT_CONTROL_SOCKET_HPP
#define EAR_PRODUCTION_SUITE_INPUT_CONTROL_SOCKET_HPP
namespace ear::plugin::communication {
EAR_PLUGIN_BASE_EXPORT class InputControlSocket {
 public:
  InputControlSocket(std::function<void()> const& connectedCallback,
              std::function<void()> const& disconnectedCallback);

  void open(std::string const& endpoint);
  void requestNewConnection(ConnectionId const& id);
  void requestCloseConnection(ConnectionId const& id);
  void requestObjectDetails(ConnectionId const& id);

  [[nodiscard]]
  Response receive();

 private:
  template<typename MessageT>
  void send(MessageT const& message) {
    auto buffer = serialize(message);
    socket_.send(buffer);
  }

  nng::ReqSocket socket_;
};
}

#endif  // EAR_PRODUCTION_SUITE_INPUT_CONTROL_SOCKET_HPP
