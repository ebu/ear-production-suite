#pragma once
#include "message_buffer.hpp"
#include <cstddef>
#include <vector>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <string>
#include <stdexcept>
#include "common_types.hpp"
#include "common_types.pb.h"
#include "ui/item_colour.hpp"

namespace ear {
namespace plugin {
namespace communication {

class GenericResponse {
 public:
  explicit GenericResponse(ConnectionId connectionId)
      : connectionId_(connectionId) {}

  ConnectionId connectionId() const { return connectionId_; }

 private:
  ConnectionId connectionId_;
};

class NewConnectionMessage {
 public:
  NewConnectionMessage(ConnectionType type,
                       communication::ConnectionId connectionId)
      : type_(type),
        protocolVersion_(CURRENT_PROTOCOL_VERSION),
        connectionId_(connectionId) {}

  communication::ConnectionId connectionId() const { return connectionId_; }
  ConnectionType type() const { return type_; }
  uint32_t protocolVersion() const { return protocolVersion_; }

 private:
  communication::ConnectionId connectionId_;
  uint32_t protocolVersion_;
  ConnectionType type_;
};

class NewConnectionResponse {
 public:
  NewConnectionResponse(ConnectionId connectionId)
      : connectionId_(connectionId) {}

  ConnectionId connectionId() const { return connectionId_; }

 private:
  ConnectionId connectionId_;
};

class CloseConnectionMessage {
 public:
  CloseConnectionMessage(ConnectionId connectionId)
      : connectionId_(connectionId) {}

  ConnectionId connectionId() const { return connectionId_; }

 private:
  ConnectionId connectionId_;
};

class CloseConnectionResponse {
 public:
  CloseConnectionResponse(ConnectionId connectionID)
      : connectionId_(connectionID) {}
  ConnectionId connectionId() const { return connectionId_; }

 private:
  ConnectionId connectionId_;

 private:
};

class ObjectDetailsMessage {
 public:
  ObjectDetailsMessage(ConnectionId connectionID)
      : connectionId_(connectionID) {}

  ConnectionId connectionId() const { return connectionId_; }

 private:
  ConnectionId connectionId_;
};

class ConnectionDetailsResponse {
 public:
  ConnectionDetailsResponse(ConnectionId connectionID,
                            const std::string& metadataEndpoint)
      : connectionId_(connectionID), endpoint_(metadataEndpoint) {}
  ConnectionId connectionId() const { return connectionId_; }
  std::string metadataEndpoint() const { return endpoint_; }

 private:
  ConnectionId connectionId_;
  std::string endpoint_;
};

class ItemPropertiesChangedMessage {
 public:
  ItemPropertiesChangedMessage(ConnectionId id, const std::string& name,
                               const ui::ItemColour& colour)
      : connectionId_(id), name_(name), colour_(colour) {}
  ItemPropertiesChangedMessage(ConnectionId id, const std::string& name)
      : connectionId_(id), name_(name) {}
  ItemPropertiesChangedMessage(ConnectionId id) : connectionId_(id) {}

  const boost::optional<std::string>& name() const { return name_; }
  const boost::optional<ui::ItemColour>& colour() const { return colour_; }

  void name(const std::string& name) { name_ = name; }
  void colour(const ui::ItemColour& colour) { colour_ = colour; }

  ConnectionId connectionId() const { return connectionId_; }

 private:
  boost::optional<std::string> name_;
  boost::optional<ui::ItemColour> colour_;
  ConnectionId connectionId_;
};

class MonitoringConnectionDetailsMessage {
 public:
  MonitoringConnectionDetailsMessage(ConnectionId connectionID)
      : connectionId_(connectionID) {}

  ConnectionId connectionId() const { return connectionId_; }

 private:
  ConnectionId connectionId_;
};

class MonitoringConnectionDetailsResponse {
 public:
  MonitoringConnectionDetailsResponse(ConnectionId connectionID,
                                      const std::string& metadataEndpoint)
      : connectionId_(connectionID), endpoint_(metadataEndpoint) {}
  ConnectionId connectionId() const { return connectionId_; }
  std::string metadataEndpoint() const { return endpoint_; }

 private:
  ConnectionId connectionId_;
  std::string endpoint_;
};

using RequestVariant =
    boost::variant<NewConnectionMessage, CloseConnectionMessage,
                   ObjectDetailsMessage, ItemPropertiesChangedMessage,
                   MonitoringConnectionDetailsMessage>;
class Request : public RequestVariant {
 public:
  Request(const RequestVariant& value) : RequestVariant(value) {}
  using RequestVariant::RequestVariant;  // inherit constructor
};
using ResponsePayloadVariant =
    boost::variant<GenericResponse, NewConnectionResponse,
                   CloseConnectionResponse, ConnectionDetailsResponse,
                   MonitoringConnectionDetailsResponse>;

class ResponsePayloadPrinter : public boost::static_visitor<void> {
 public:
  void operator()(GenericResponse const& response) const {
    std::cout << "GenericResponse" << std::endl;
  }
  void operator()(NewConnectionResponse const& response) const {
    std::cout << "NewConnectionResponse" << std::endl;
  }
  void operator()(CloseConnectionResponse const& response) const {
    std::cout << "CloseConnectionResponse" << std::endl;
  }
  void operator()(ConnectionDetailsResponse const& response) const {
    std::cout << "ConnectionDetailsResponse" << std::endl;
  }
  void operator()(MonitoringConnectionDetailsResponse const& response) const {
    std::cout << "MonitoringConnectionDetailsResponse" << std::endl;
  }
};
/**
 * @brief Wraps the response send to a client
 *
 * A response can essentially indicate two states:
 * - The request was successful and (most likely) some data has been sent
 * based on the request ( the playoad )
 * - The request caused an error.
 *
 * If the request was successfull, indicated by `success()` the received payload
 * can be accessed by `payload()`.
 * If the request was *not* successfull, any attempt to acces the payload is a
 * programming error and will trigger an `std::runtime_error`
 */
class Response {
 public:
  /**
   * Constructor for a "successfull" response.
   *
   * The given payload will be stored, the `errorCode()` is set to
   * `ErrorCode::NO_ERROR`
   */
  Response(const ResponsePayloadVariant& payload)
      : payload_(payload), ec_(ErrorCode::NO_ERROR) {}

  /**
   * Constructor for a "failure" response
   *
   * The errorCode and the optional error description will be set.
   * The `payload` is undefined.
   *
   * Using `ErrorCode::NO_ERROR` as a value for `ec` will lead to undefined
   * behaviour.
   */
  Response(ErrorCode ec, const std::string& errorDescription = "")
      : ec_(ec), errorDescription_(errorDescription) {}

  /**
   * @brief Access the stored payload
   *
   * This will throw an exception if `success() == false`
   */
  const ResponsePayloadVariant& payload() const {
    if (payload_) {
      return payload_.get();
    } else {
      throw std::runtime_error("access to payload with error condition");
    }
  }

  /**
   * @brief Access stored payload of a given type
   *
   * Convenience short-hand for `boost::get<T>(payload());`
   */
  template <typename T>
  const T& payloadAs() const {
    return boost::get<T>(payload_.get());
  }

  /**
   * The code of the error or `ErrorCode::NO_ERROR` to indicate a response to a
   * successfull request
   */
  ErrorCode errorCode() const { return ec_; }
  /**
   * @brief Check if request/response was succesfull
   *
   * Syntactic short-hand for `errorCode() == ErrorCode::NO_ERROR`
   */
  bool success() const { return ec_ == ErrorCode::NO_ERROR; }

  /**
   * @brief (Optional) textual description of the error
   *
   * Undefined if `success() == true`.
   */
  std::string errorDescription() const { return errorDescription_; }

 private:
  boost::optional<ResponsePayloadVariant> payload_;
  ErrorCode ec_;
  std::string errorDescription_;
};

MessageBuffer serialize(const NewConnectionMessage& msg);
MessageBuffer serialize(const NewConnectionResponse& msg);
MessageBuffer serialize(const CloseConnectionMessage& msg);
MessageBuffer serialize(const CloseConnectionResponse& msg);
MessageBuffer serialize(const ObjectDetailsMessage& msg);
MessageBuffer serialize(const ConnectionDetailsResponse& msg);
MessageBuffer serialize(const ItemPropertiesChangedMessage& msg);
MessageBuffer serialize(const MonitoringConnectionDetailsMessage& msg);
MessageBuffer serialize(const MonitoringConnectionDetailsResponse& msg);
MessageBuffer serializeErrorResponse(ErrorCode errorCode,
                                     const std::string& message);

MessageBuffer serialize(const Response& response);

RequestVariant parseRequest(const void* data, int size);
Response parseResponse(const void* data, int size);

template <typename ConstBuffer>
RequestVariant parseRequest(const ConstBuffer& buffer) {
  return parseRequest(buffer.data(), buffer.size());
}

template <typename ConstBuffer>
Response parseResponse(const ConstBuffer& buffer) {
  return parseResponse(buffer.data(), buffer.size());
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
