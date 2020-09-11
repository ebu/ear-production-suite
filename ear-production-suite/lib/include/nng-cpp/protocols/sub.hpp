#pragma once
#include "protocol_traits.hpp"
#include "../socket_base.hpp"
#include "../error_handling.hpp"
#include <nng/protocol/pubsub0/sub.h>
#include <nng/nng.h>

namespace nng {
namespace protocols {
struct Sub0 {};
using Sub = Sub0;
}  // namespace protocols

namespace options {
struct SubSubscribe_t {
  constexpr const char* name() { return NNG_OPT_SUB_SUBSCRIBE; }
};
static SubSubscribe_t SubSubscribe;

struct SubUnsubscribe_t {
  constexpr const char* name() { return NNG_OPT_SUB_UNSUBSCRIBE; }
};
static SubUnsubscribe_t SubUnsubscribe;

}  // namespace options

namespace detail {
using SubOptions = boost::mp11::mp_list<options::SubSubscribe_t, options::SubUnsubscribe_t>;
}

template <>
struct ProtocolTraits<protocols::Sub0> {
  using can_receive = std::true_type;
  using can_send = std::false_type;
  using Options = boost::mp11::mp_append<detail::CommonReceiveOptions, detail::SubOptions>;


  static void open(nng_socket* socket) {
    auto ret = nng_sub0_open(socket);
    handleError(ret);
  }
};

/**
 * @brief Socket using `sub` protocol
 * @sa https://nanomsg.github.io/nng/man/v1.1.0/nng_sub.7
 */
using SubSocket = SocketBase<protocols::Sub>;
/**
 * @brief Socket using `sub` protocol (version 0)
 * @sa https://nanomsg.github.io/nng/man/v1.1.0/nng_sub.7
 */
using Sub0Socket = SocketBase<protocols::Sub0>;

}  // namespace nng
