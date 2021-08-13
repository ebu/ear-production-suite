#pragma once
#include "protocol_traits.hpp"
#include "../socket_base.hpp"
#include "../error_handling.hpp"
#include <nng/protocol/pipeline0/pull.h>
#include <nng/nng.h>

namespace nng {
namespace protocols {
struct Pull0 {};
using Pull = Pull0;
}  // namespace protocols

template <>
struct ProtocolTraits<protocols::Pull0> {
  using can_receive = std::true_type;
  using can_send = std::false_type;
  using Options = detail::CommonReceiveOptions;

  static void open(nng_socket* socket) {
    auto ret = nng_pull0_open(socket);
    handleError(ret);
  }
};

/**
 * @brief Socket using `pull` protocol
 * @sa https://nanomsg.github.io/nng/man/v1.1.0/nng_pull.7
 */
using PullSocket = SocketBase<protocols::Pull>;
/**
 * @brief Socket using `pull` protocol (version 0)
 * @sa https://nanomsg.github.io/nng/man/v1.1.0/nng_pull.7
 */
using Pull0Socket = SocketBase<protocols::Pull0>;

}  // namespace nng
