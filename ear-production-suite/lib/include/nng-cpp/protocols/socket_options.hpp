#pragma once
#include <nng/nng.h>
#include <boost/mp11/list.hpp>

namespace nng {
namespace options {

struct RecvTimeout_t {
  constexpr const char* name() { return NNG_OPT_RECVTIMEO; }
};

static RecvTimeout_t RecvTimeout;

struct SendTimeout_t {
  constexpr const char* name() { return NNG_OPT_SENDTIMEO; }
};

static SendTimeout_t SendTimeout;

struct ReconnectMinTime_t {
  constexpr const char* name() { return NNG_OPT_RECONNMINT; }
};

static ReconnectMinTime_t ReconnectMinTime;

struct ReconnectMaxTime_t {
  constexpr const char* name() { return NNG_OPT_RECONNMAXT; }
};

static ReconnectMaxTime_t ReconnectMaxTime;
}  // namespace options


}  // namespace nng
