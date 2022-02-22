/// @file named_type.hpp
#pragma once

#include "../detail/named_type.hpp"
#include <stdint.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/functional/hash.hpp>

#ifdef WIN32
#include <winerror.h>
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#endif

namespace ear {
namespace plugin {
namespace communication {

class ConnectionId {
 public:
  ConnectionId() : id_(boost::uuids::nil_uuid()) {}
  ConnectionId(boost::uuids::uuid id) : id_(id) {}
  ConnectionId(const std::string& str) {
    boost::uuids::string_generator gen;
    id_ = gen(str);
  }
  ConnectionId(const ConnectionId&) = default;
  ConnectionId& operator=(const ConnectionId&) = default;
  ConnectionId(ConnectionId&&) = default;
  ConnectionId& operator=(ConnectionId&&) = default;

  bool isValid() const { return !(id_.is_nil()); }

  boost::uuids::uuid getUuid() const { return id_; }

  static ConnectionId generate() {
    return ConnectionId(boost::uuids::random_generator()());
  }

  std::string string() const { return boost::uuids::to_string(id_); }

  bool operator==(const ConnectionId& other) const {
    return this->getUuid() == other.getUuid();
  }

  template <typename U>
  bool operator==(const U& other) const {
    return this->getUuid() == other;
  }

  bool operator!=(const ConnectionId& other) const {
    return this->getUuid() != other.getUuid();
  }

  bool operator<(const ConnectionId& other) const {
    return this->getUuid() < other.getUuid();
  }

 private:
  boost::uuids::uuid id_;
};

inline std::ostream& operator<<(std::ostream& stream, const ConnectionId& rhs) {
  stream << rhs.getUuid();
  return stream;
}

const uint32_t CURRENT_PROTOCOL_VERSION = 0;

enum class ErrorCode {
  NO_ERROR,
  UNKOWN_ERROR,
  MALFORMED_RESPONSE,
  PROTOCOL_VERSION_MISMATCH
};

enum class ConnectionType { METADATA_INPUT, MONITORING };

}  // namespace communication
}  // namespace plugin
}  // namespace ear
