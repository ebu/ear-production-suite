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
  void send(NewConnectionMessage const& message);
  void send(ObjectDetailsMessage const& message);
  void send(CloseConnectionMessage const& message);

  [[nodiscard]]
  Response receive();

 private:
  void send(MessageBuffer const& buffer);
  nng::ReqSocket socket_;
};
}

#endif  // EAR_PRODUCTION_SUITE_INPUT_CONTROL_SOCKET_HPP
