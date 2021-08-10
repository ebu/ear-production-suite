#pragma once
#include "socket_options.hpp"
#include <type_traits>
namespace nng {

template <typename Protocol>
struct ProtocolTraits {};

namespace detail {
    using CommonSendOptions = boost::mp11::mp_list<options::SendTimeout_t, options::ReconnectMaxTime_t, options::ReconnectMinTime_t>;
    using CommonReceiveOptions = boost::mp11::mp_list<options::RecvTimeout_t, options::ReconnectMaxTime_t, options::ReconnectMinTime_t>;
}

}  // namespace nng
