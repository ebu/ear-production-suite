#pragma once
#include "protocol_traits.hpp"
#include "../socket_base.hpp"
#include "../error_handling.hpp"
#include <nng/protocol/reqrep0/rep.h>
#include <nng/nng.h>

namespace nng {
namespace protocols {
struct Rep0 {};
using Rep = Rep0;
}  // namespace protocols

template <>
struct ProtocolTraits<protocols::Rep0> {
  using can_receive = std::true_type;
  using can_send = std::true_type;
  using Options = boost::mp11::mp_append<detail::CommonSendOptions, detail::CommonReceiveOptions>;

  static void open(nng_socket* socket) {
    auto ret = nng_rep0_open(socket);
    handleError(ret);
  }
};

/**
 * @brief Socket using `rep` protocol
 * @sa https://nanomsg.github.io/nng/man/v1.1.0/nng_rep.7
 */
using RepSocket = SocketBase<protocols::Rep>;
/**
 * @brief Socket using `rep` protocol (version 0)
 * @sa https://nanomsg.github.io/nng/man/v1.1.0/nng_rep.7
 */
using Rep0Socket = SocketBase<protocols::Rep0>;

}  // namespace nng
