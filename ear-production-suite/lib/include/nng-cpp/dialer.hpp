#pragma once

#include "error_handling.hpp"
#include <nng/nng.h>

namespace nng {

/**
 * Wraps a nng dialer object.
 *
 * As dialers are tied to a socket, `SocketBase::createDialer()`
 * should be used to create instances of this class.
 *
 * This class will close the wrapped dialer on destruction.
 */
class Dialer {
 public:
  Dialer() {}
  /** Constructor, nng_dialer_create
   * Use SocketBase.createDialer()
   */
  Dialer(nng_socket s, const char* endpoint) {
    auto ret = nng_dialer_create(&dialer_, s, endpoint);
    handleError(ret);
    ;
  }
  ~Dialer() { close(); }

  Dialer(const Dialer&) = delete;
  Dialer& operator=(const Dialer&) = delete;
  Dialer(Dialer&& other) {
    dialer_ = other.dialer_;
    other.dialer_ = NNG_DIALER_INITIALIZER;
  }
  Dialer& operator=(Dialer&& other) {
    close();
    dialer_ = other.dialer_;
    other.dialer_ = NNG_DIALER_INITIALIZER;
    return *this;
  }

  /// nng_dialer_start
  void start() {
    auto ret = nng_dialer_start(dialer_, 0);
    handleError(ret);
  }
  /// nng_dialer_close
  void close() {
    if (id() > 0) {
      auto ret = nng_dialer_close(dialer_);
      handleError(ret);
      dialer_ = NNG_DIALER_INITIALIZER;
    }
  };
  // nng_dialer_id
  int id() { return nng_dialer_id(dialer_); }

 private:
  nng_dialer dialer_ = NNG_DIALER_INITIALIZER;
};
}  // namespace nng
