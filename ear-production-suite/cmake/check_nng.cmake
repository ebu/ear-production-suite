# two things happened:
# nng suddenly changed the callback signature (from int to nng_pipe_ev).
# This is currently not yet released, but in the master. (2019/05/10)
# To make things even more fun, vcpkg decided to use this master version
# instead of the latest release, which is still used by homebrew, for example.

function(check_nng_pipe_callback_signature)

set(CMAKE_REQUIRED_LIBRARIES nng::nng)
include(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES(
  "#include <nng/nng.h>
  void cb(nng_pipe, nng_pipe_ev, void*) {}

  int main() {
    nng_socket s;
    nng_pipe_notify(s, NNG_PIPE_EV_ADD_PRE, cb, NULL);
  }
  " NEW_NNG_PIPE_NOTIFY_CALLBACK_SIGNATURE
)
set(NEW_NNG_PIPE_NOTIFY_CALLBACK_SIGNATURE ${NNEW_NNG_PIPE_NOTIFY_CALLBACK_SIGNATURE} PARENT_SCOPE)
endfunction()
