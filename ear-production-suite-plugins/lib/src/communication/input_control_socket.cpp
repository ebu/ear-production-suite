//
// Created by Richard Bailey on 18/02/2022.
//

#include "communication/input_control_socket.hpp"
#include <chrono>
using namespace std::chrono_literals;
using namespace ear::plugin::communication;

InputControlSocket::InputControlSocket(std::function<void()> const& connectedCallback,
                         std::function<void()> const& disconnectedCallback)
    {
  socket_.setOpt(nng::options::RecvTimeout, 1000ms);
  socket_.setOpt(nng::options::SendTimeout, 100ms);
  socket_.setOpt(nng::options::ReconnectMinTime, 250ms);
  socket_.setOpt(nng::options::ReconnectMaxTime, 0ms);
  socket_.onPipeEvent(nng::PipeEvent::postAdd,
                      [connectedCallback](nng::Pipe, nng::PipeEvent){
                        connectedCallback();}),
  socket_.onPipeEvent(nng::PipeEvent::postRemove,
                      [disconnectedCallback](nng::Pipe, nng::PipeEvent){
                        disconnectedCallback();});
}

void InputControlSocket::open(const std::string& endpoint) {
  socket_.dial(endpoint.c_str(), nng::Flags::nonblock);
}

void InputControlSocket::send(const NewConnectionMessage& message) {
  auto buffer = serialize(message);
  send(buffer);
}

void InputControlSocket::send(const ObjectDetailsMessage& message) {
  auto buffer = serialize(message);
  send(buffer);
}

void InputControlSocket::send(const CloseConnectionMessage& message) {
  auto buffer = serialize(message);
  send(buffer);
}

Response InputControlSocket::receive() {
  auto response = parseResponse(socket_.read());
  return response;
}

void InputControlSocket::send(const MessageBuffer& buffer) {
  // Note, return code ignored for send as we're not blocking
  // and only handled code is NNG_EAGAIN, which is only produced for
  // non-blocking sends.
  socket_.send(buffer);
}

