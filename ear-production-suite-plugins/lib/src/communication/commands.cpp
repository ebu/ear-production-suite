#include "communication/commands.hpp"
#include "connection_commands.pb.h"

#include <boost/functional/overloaded_function.hpp>

namespace ear {
namespace plugin {
namespace communication {

namespace detail {
struct SerializeVisitor : boost::static_visitor<MessageBuffer> {
  template <typename T>
  MessageBuffer operator()(const T& payload) const {
    return serialize(payload);
  }
};
}  // namespace detail

MessageBuffer serialize(const Response& response) {
  if (!response.success()) {
    return serializeErrorResponse(response.errorCode(),
                                  response.errorDescription());
  }
  return boost::apply_visitor(detail::SerializeVisitor(), response.payload());
}

template <typename T>
MessageBuffer serialize(const T& payload) {
  MessageBuffer buffer = allocBuffer(payload.ByteSizeLong());
  payload.SerializeToArray(buffer.data(), buffer.size());
  return buffer;
}

MessageBuffer serialize(const GenericResponse& msg) {
  proto::CmdResponse response;
  response.set_status(proto::STATUS_CMD_OK);
  return serialize(response);
}

MessageBuffer serialize(const NewConnectionMessage& msg) {
  proto::CmdRequest request;
  auto payload =
      request.MutableExtension(proto::CmdConnectionReq::cmdConnectionReq);
  payload->set_connection_id(msg.connectionId().string());
  if (msg.type() == ConnectionType::METADATA_INPUT) {
    payload->set_plugin_type(proto::PluginType::INPUT_PLUGIN);
  } else if (msg.type() == ConnectionType::MONITORING) {
    payload->set_plugin_type(proto::PluginType::MONITORING_PLUGIN);
  }
  payload->set_protocol_version(msg.protocolVersion());
  return serialize(request);
}

MessageBuffer serialize(const NewConnectionResponse& msg) {
  proto::CmdResponse response;
  auto payload =
      response.MutableExtension(proto::CmdConnectionResp::cmdConnectionResp);
  payload->set_connection_id(msg.connectionId().string());
  return serialize(response);
}

MessageBuffer serialize(const CloseConnectionMessage& msg) {
  proto::CmdRequest request;
  auto payload = request.MutableExtension(
      proto::CmdCloseConnectionReq::cmdCloseConnectionReq);
  payload->set_connection_id(msg.connectionId().string());
  return serialize(request);
}

MessageBuffer serialize(const CloseConnectionResponse& msg) {
  proto::CmdResponse response;
  auto payload = response.MutableExtension(
      proto::CmdCloseConnectionResp::cmdCloseConnectionResp);
  payload->set_connection_id(msg.connectionId().string());
  return serialize(response);
}

MessageBuffer serialize(const ObjectDetailsMessage& msg) {
  proto::CmdRequest request;
  auto payload = request.MutableExtension(
      proto::CmdConnectionDetailsReq::cmdConnectionDetailsReq);
  payload->set_connection_id(msg.connectionId().string());
  payload->set_item_type(proto::ADM_OBJECT);
  return serialize(request);
}

MessageBuffer serialize(const ConnectionDetailsResponse& msg) {
  proto::CmdResponse response;
  auto payload = response.MutableExtension(
      proto::CmdConnectionDetailsResp::cmdConnectionDetailsResp);
  payload->set_connection_id(msg.connectionId().string());
  payload->set_metadata_endpoint(msg.metadataEndpoint());
  return serialize(response);
}

MessageBuffer serialize(const MonitoringConnectionDetailsMessage& msg) {
  proto::CmdRequest request;
  auto payload =
      request.MutableExtension(proto::CmdMonitoringConnectionDetailsReq::
                                   cmdMonitoringConnectionDetailsReq);
  payload->set_connection_id(msg.connectionId().string());
  return serialize(request);
}

MessageBuffer serialize(const MonitoringConnectionDetailsResponse& msg) {
  proto::CmdResponse response;
  auto payload =
      response.MutableExtension(proto::CmdMonitoringConnectionDetailsResp::
                                    cmdMonitoringConnectionDetailsResp);
  payload->set_connection_id(msg.connectionId().string());
  payload->set_metadata_endpoint(msg.metadataEndpoint());
  return serialize(response);
}

MessageBuffer serializeErrorResponse(ErrorCode errorCode,
                                     const std::string& message) {
  proto::CmdResponse resp;
  // we only have "one" error code for now, but we should provide a reasonable
  // translation at one point here
  resp.set_status(proto::ERROR_OTHER);
  resp.set_error_description(message);
  return serialize(resp);
}

RequestVariant parseRequest(const void* data, int size) {
  proto::CmdRequest request;
  if (!request.ParseFromArray(data, size)) {
    throw std::runtime_error("Failed to parse Request");
  }
  if (request.HasExtension(proto::CmdConnectionReq::cmdConnectionReq)) {
    auto ext = request.GetExtension(proto::CmdConnectionReq::cmdConnectionReq);
    auto connectionId = communication::ConnectionId{ext.connection_id()};
    auto protocolVersion = ext.protocol_version();
    if (protocolVersion != CURRENT_PROTOCOL_VERSION) {
      throw std::runtime_error(
          "Failed to parse request: protocol version mismatch");
    }
    if (ext.plugin_type() == proto::INPUT_PLUGIN) {
      return NewConnectionMessage(ConnectionType::METADATA_INPUT, connectionId);
    } else if (ext.plugin_type() == proto::MONITORING_PLUGIN) {
      return NewConnectionMessage(ConnectionType::MONITORING, connectionId);
    } else {
      throw std::runtime_error(
          "Failed to parse request: unhandled plugin type");
    }
  }
  if (request.HasExtension(
          proto::CmdCloseConnectionReq::cmdCloseConnectionReq)) {
    auto ext = request.GetExtension(
        proto::CmdCloseConnectionReq::cmdCloseConnectionReq);
    auto connectionId = communication::ConnectionId{ext.connection_id()};
    return CloseConnectionMessage(connectionId);
  }
  if (request.HasExtension(
          proto::CmdConnectionDetailsReq::cmdConnectionDetailsReq)) {
    auto ext = request.GetExtension(
        proto::CmdConnectionDetailsReq::cmdConnectionDetailsReq);
    auto connectionId = communication::ConnectionId{ext.connection_id()};
    auto adm_type = ext.item_type();
    switch (adm_type) {
      case proto::ADM_OBJECT:
        return ObjectDetailsMessage(connectionId);
    }
  }
  if (request.HasExtension(proto::CmdMonitoringConnectionDetailsReq::
                               cmdMonitoringConnectionDetailsReq)) {
    auto ext = request.GetExtension(proto::CmdMonitoringConnectionDetailsReq::
                                        cmdMonitoringConnectionDetailsReq);
    auto connectionId = communication::ConnectionId{ext.connection_id()};
    return MonitoringConnectionDetailsMessage(connectionId);
  }
  throw std::runtime_error("Failed to parse request: unhandled subcommand");
}

Response parseResponse(const void* data, int size) {
  proto::CmdResponse response;
  if (!response.ParseFromArray(data, size)) {
    return Response(ErrorCode::MALFORMED_RESPONSE, "Failed to parse response");
  }
  if (response.status() != proto::STATUS_CMD_OK) {
    if (response.has_error_description()) {
      return Response(ErrorCode::UNKOWN_ERROR, response.error_description());
    } else {
      return Response(ErrorCode::UNKOWN_ERROR);
    }
  }
  if (response.HasExtension(proto::CmdConnectionResp::cmdConnectionResp)) {
    auto ext =
        response.GetExtension(proto::CmdConnectionResp::cmdConnectionResp);
    auto connectionId = communication::ConnectionId{ext.connection_id()};
    auto payload = NewConnectionResponse(connectionId);
    return Response(payload);
  }
  if (response.HasExtension(
          proto::CmdCloseConnectionResp::cmdCloseConnectionResp)) {
    auto ext = response.GetExtension(
        proto::CmdCloseConnectionResp::cmdCloseConnectionResp);
    auto connectionId = communication::ConnectionId{ext.connection_id()};
    auto payload = CloseConnectionResponse(connectionId);
    return Response(payload);
  }
  if (response.HasExtension(
          proto::CmdConnectionDetailsResp::cmdConnectionDetailsResp)) {
    auto ext = response.GetExtension(
        proto::CmdConnectionDetailsResp::cmdConnectionDetailsResp);
    auto id = communication::ConnectionId{ext.connection_id()};
    auto endpoint = ext.metadata_endpoint();
    auto payload = ConnectionDetailsResponse(id, endpoint);
    return Response(payload);
  }
  if (response.HasExtension(proto::CmdMonitoringConnectionDetailsResp::
                                cmdMonitoringConnectionDetailsResp)) {
    auto ext = response.GetExtension(proto::CmdMonitoringConnectionDetailsResp::
                                         cmdMonitoringConnectionDetailsResp);
    auto id = communication::ConnectionId{ext.connection_id()};
    auto endpoint = ext.metadata_endpoint();
    auto payload = MonitoringConnectionDetailsResponse(id, endpoint);
    return Response(payload);
  }

  return Response(ErrorCode::MALFORMED_RESPONSE,
                  "Failed to parse response: unhandled subpayload");
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
