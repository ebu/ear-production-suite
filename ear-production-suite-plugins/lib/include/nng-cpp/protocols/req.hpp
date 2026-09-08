#pragma once
#include "protocol_traits.hpp"
#include "../socket_base.hpp"
#include "../error_handling.hpp"
#include <nng/protocol/reqrep0/req.h>
#include <nng/nng.h>
#include <functional>
#include <mutex>
#include <system_error>
#include <utility>

namespace nng {
namespace protocols {
struct Req0 {};
using Req = Req0;
}  // namespace protocols

template <>
struct ProtocolTraits<protocols::Req0> {
  using can_receive = std::true_type;
  using can_send = std::true_type;
  using Options = boost::mp11::mp_append<detail::CommonSendOptions,
                                         detail::CommonReceiveOptions>;

  static void open(nng_socket* socket) {
    auto ret = nng_req0_open(socket);
    handleError(ret);
  }
};

/**
 * @brief Socket using `req` protocol
 * @sa https://nanomsg.github.io/nng/man/v1.1.0/nng_req.7
 */
class ReqSocket : public SocketBase<protocols::Req> {
 public:
  using AsyncResponseHandler = std::function<void(std::error_code, Message)>;

  ~ReqSocket() { close(); }

  /**
   * Send a request and receive its response using the socket's single AIO.
   *
   * Returns false when another request is still pending or the AIO has been
   * stopped. The completion handler may start the next request.
   */
  template <typename ConstBuffer, typename CompletionHandler>
  bool asyncRequest(const ConstBuffer& request, CompletionHandler handler) {
    std::lock_guard<std::mutex> lock(requestMutex_);
    if (stopped_ || requestPending_) {
      return false;
    }

    requestPending_ = true;
    asyncSend(request, [this, handler = std::move(handler)](std::error_code ec,
                                                            Message) mutable {
      if (ec) {
        completeRequest(std::move(handler), ec, Message{});
        return;
      }

      {
        std::lock_guard<std::mutex> lock(requestMutex_);
        if (stopped_) {
          requestPending_ = false;
          return;
        }
        asyncRead([this, handler = std::move(handler)](
                      std::error_code ec, Message message) mutable {
          completeRequest(std::move(handler), ec, std::move(message));
        });
      }
    });
    return true;
  }

  void asyncCancel() {
    std::lock_guard<std::mutex> lock(requestMutex_);
    if (requestPending_) {
      SocketBase<protocols::Req>::asyncCancel();
    }
  }

  void asyncStop() {
    {
      std::lock_guard<std::mutex> lock(requestMutex_);
      stopped_ = true;
      requestPending_ = false;
    }
    SocketBase<protocols::Req>::asyncStop();
  }

  void close() {
    {
      std::lock_guard<std::mutex> lock(requestMutex_);
      stopped_ = true;
      requestPending_ = false;
    }
    SocketBase<protocols::Req>::close();
  }

 private:
  void completeRequest(AsyncResponseHandler handler, std::error_code ec,
                       Message message) {
    {
      std::lock_guard<std::mutex> lock(requestMutex_);
      requestPending_ = false;
    }
    handler(ec, std::move(message));
  }

  std::mutex requestMutex_;
  bool requestPending_{false};
  bool stopped_{false};
};
/**
 * @brief Socket using `req` protocol (version 0)
 * @sa https://nanomsg.github.io/nng/man/v1.1.0/nng_req.7
 */
using Req0Socket = ReqSocket;

}  // namespace nng
