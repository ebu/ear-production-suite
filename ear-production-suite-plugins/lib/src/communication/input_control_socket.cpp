//
// Created by Richard Bailey on 18/02/2022.
//

#include "communication/input_control_socket.hpp"
#include "log.hpp"
#include <chrono>
using namespace std::chrono_literals;
using namespace ear::plugin::communication;

InputControlSocket::InputControlSocket(std::function<void()> const& connectedCallback,
                         std::function<void()> const& disconnectedCallback)
    : logger_(createLogger(fmt::format("InputControlSocket@{}", (const void*)this)))
{
    logger_->set_level(spdlog::level::trace);
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
    EAR_LOGGER_TRACE(logger_, "Connecting to {}", endpoint.c_str());
  socket_.dial(endpoint.c_str(), nng::Flags::nonblock);
}

void InputControlSocket::requestNewConnection(const ConnectionId& id) {
    EAR_LOGGER_TRACE(logger_, "Requesting  new connection with id {}", id.string());
  auto message = NewConnectionMessage{ConnectionType::METADATA_INPUT, id};
  send(message);
}

void InputControlSocket::requestObjectDetails(const ConnectionId& id) {
    EAR_LOGGER_TRACE(logger_, "Requesting metadata endpoint for id {}", id.string());
  auto message = ObjectDetailsMessage{id};
  send(message);
}

void InputControlSocket::requestCloseConnection(const ConnectionId& id) {
    EAR_LOGGER_TRACE(logger_, "Requesting close connection for id {}", id.string());
  auto message = CloseConnectionMessage{id};
  send(message);
}

Response InputControlSocket::receive() {
  auto response = parseResponse(socket_.read());
  return response;
}
