#pragma once
#include "protocol_traits.hpp"
#include "../socket_base.hpp"
#include <nng/protocol/pipeline0/push.h>
#include <nng/nng.h>

namespace nng {
namespace protocols {
struct Push0 {};
using Push = Push0;
}  // namespace protocols

template <>
struct ProtocolTraits<protocols::Push0> {
  using can_receive = std::false_type;
  using can_send = std::true_type;
  using Options = detail::CommonSendOptions;

  static void open(nng_socket* socket) { nng_push0_open(socket); }
};

/**
 * @brief Socket using `push` protocol
 * @sa https://nanomsg.github.io/nng/man/v1.1.0/nng_push.7
 */
using PushSocket = SocketBase<protocols::Push>;
/**
 * @brief Socket using `push` protocol (version 0)
 * @sa https://nanomsg.github.io/nng/man/v1.1.0/nng_push.7
 */
using Push0Socket = SocketBase<protocols::Push0>;
}  // namespace nng
