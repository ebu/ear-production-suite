#pragma once
#include "protocol_traits.hpp"
#include "../socket_base.hpp"
#include "../error_handling.hpp"
#include <nng/protocol/reqrep0/req.h>
#include <nng/nng.h>

namespace nng {
namespace protocols {
struct Req0 {};
using Req = Req0;
}  // namespace protocols

template <>
struct ProtocolTraits<protocols::Req0> {
  using can_receive = std::true_type;
  using can_send = std::true_type;
  using Options = boost::mp11::mp_append<detail::CommonSendOptions, detail::CommonReceiveOptions>;;

  static void open(nng_socket* socket) {
    auto ret = nng_req0_open(socket);
    handleError(ret);
  }
};

/**
 * @brief Socket using `req` protocol
 * @sa https://nanomsg.github.io/nng/man/v1.1.0/nng_req.7
 */
using ReqSocket = SocketBase<protocols::Req>;
/**
 * @brief Socket using `req` protocol (version 0)
 * @sa https://nanomsg.github.io/nng/man/v1.1.0/nng_req.7
 */
using Req0Socket = SocketBase<protocols::Req0>;

}  // namespace nng
