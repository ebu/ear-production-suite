#pragma once
#include "protocol_traits.hpp"
#include "../socket_base.hpp"
#include <nng/protocol/pubsub0/pub.h>
#include <nng/nng.h>

namespace nng {
namespace protocols {
struct Pub0 {};
using Pub = Pub0;
}  // namespace protocols

template <>
struct ProtocolTraits<protocols::Pub0> {
  using can_receive = std::false_type;
  using can_send = std::true_type;
  using Options = detail::CommonSendOptions;

  static void open(nng_socket* socket) { nng_pub0_open(socket); }
};

/**
 * @brief Socket using `pub` protocol
 * @sa https://nanomsg.github.io/nng/man/v1.1.0/nng_pub.7
 */
using PubSocket = SocketBase<protocols::Pub>;
/**
 * @brief Socket using `pub` protocol (version 0)
 * @sa https://nanomsg.github.io/nng/man/v1.1.0/nng_pub.7
 */
using Pub0Socket = SocketBase<protocols::Pub0>;
}  // namespace nng
