//
// Created by rsjbailey on 17/02/2022.
//
#include "communication/input_handshake.hpp"
#include "communication/input_control_connection.hpp"

using namespace ear::plugin::communication;

InitState::InitState(InputControlConnection *connection)
    : controlConnection{connection} {}

void InitState::process() const {

  NewConnectionMessage request{ConnectionType::METADATA_INPUT,
                               controlConnection->getConnectionId()};
  auto sendBuffer = serialize(request);
  controlConnection->send(sendBuffer);
  processResponse();
}

void InitState::processResponse() const {
  auto resp = controlConnection->receive();
  if (!resp.success()) {
    throw std::runtime_error("Failed to start new control connection: " + resp.errorDescription());
  }
  boost::apply_visitor(*this, resp.payload());
}

void InitState::operator()(NewConnectionResponse response) const {
  if(!response.connectionId().isValid()) {
    throw std::runtime_error( "Failed to start new control connection: invalid "
                              "connection id received");
  }
  controlConnection->setConnectionId(response.connectionId(), true);
  controlConnection->setState(std::make_unique<ConnectingState>(controlConnection));
}


ConnectingState::ConnectingState(InputControlConnection *connection)
    : controlConnection(connection) {
}

void ConnectingState::process() const {
//  EAR_LOGGER_TRACE(logger_, "Sending object connection details");
  ObjectDetailsMessage request{controlConnection->getConnectionId()};
  auto sendBuffer = serialize(request);
  controlConnection->send(sendBuffer);
  processResponse();
}

void ConnectingState::processResponse() const {
  auto resp = controlConnection->receive();
  if (!resp.success()) {
    throw std::runtime_error("Failed to start new control connection: " + resp.errorDescription());
  }

  auto payloadVariant = resp.payload();
  boost::apply_visitor(*this, resp.payload());
}

void ConnectingState::operator()(
    NewConnectionResponse response) const {
  processResponse();
}

void ConnectingState::operator()(
    ConnectionDetailsResponse response) const {
  auto streamEndpoint = response.metadataEndpoint();
  controlConnection->setConnected(response.metadataEndpoint());
//  EAR_LOGGER_DEBUG(logger_,
//                   "Received {} as target endpoint for metadata streaming",
//                   streamEndpoint);

}
