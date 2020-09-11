#pragma once
#include "protocols/protocol_traits.hpp"
#include "error_handling.hpp"
#include "asyncio.hpp"
#include "dialer.hpp"
#include "pipe.hpp"
#include "enum_bitmask.hpp"
#include "buffer.hpp"
#include <nng/nng.h>
#include <boost/mp11/algorithm.hpp>
#include <cstddef>
#include <type_traits>
#include <chrono>
#include <memory>

namespace nng {

enum class Flags : int {
  none = 0x0,
  nonblock = NNG_FLAG_NONBLOCK,
};
NNG_ENABLE_ENUM_BITMASK_OPERATORS(Flags);

/**
 * @brief Socket template for all available protocols
 *
 * This class implements a `socket` that uses
 * the given `Protocol`.
 *
 * The socket class supports both synchronous an asynchronous operations.
 * Async operations will be handled by an *internal* default `AsyncIO` instance.
 *
 * Only a single operation can be active at a time.
 *
 * On destruction, the socket will be closed and any async operation will be
 * stopped.
 *
 * @note
 * Not all operations will be available for all protocols.
 * I.e. send-only protocols cannot initiate read operations.
 * Trying to do so will result in an compile time error.
 *
 * nng::PullSocket
 *
 * @see https://nanomsg.github.io/nng/man/v1.1.0/nng_socket.5
 * @see nng::PushSocket
 * @see nng::PullSocket
 * @see nng::ReqSocket
 * @see nng::RepSocket
 */
template <typename Protocol>
class SocketBase {
  using ProtocolTraits = ProtocolTraits<Protocol>;

 public:
  using Options = typename ProtocolTraits::Options;
  SocketBase() {
    aio_.reset(new AsyncIO);
    ProtocolTraits::open(&socket_);
    eventDispatcher_.attach(socket_);
  }
  ~SocketBase() { nng_close(socket_); }

  SocketBase(const SocketBase&) = delete;
  SocketBase& operator=(const SocketBase&) = delete;
  SocketBase(SocketBase&& rhs) {
    socket_ = rhs.socket_;
    rhs.socket_ = NNG_SOCKET_INITIALIZER;
    aio_ = std::move(rhs.aio_);
    eventDispatcher_ = std::move(eventDispatcher_);
  }
  SocketBase& operator=(SocketBase&& rhs) {
    socket_ = rhs.socket_;
    rhs.socket_ = NNG_SOCKET_INITIALIZER;
    aio_ = std::move(rhs.aio_);
    eventDispatcher_ = std::move(eventDispatcher_);
    return *this;
  }

  void close() { nng_close(socket_); }

  void dial(const char* endpoint, Flags flags = Flags::none) {
    auto ret = nng_dial(socket_, endpoint, NULL, static_cast<int>(flags));
    handleError(ret);
  }

  /**
   * @brief Create a new dialoger for this socket.
   *
   * The returned dialer is not yet started, thus provides
   * the ability to set additional options beforehand etc.
   *
   * This essentially corresponds to a call to `nng_dialer_create()`.
   *
   * @sa dial() for a convience call-and-forget version of this functionality.`
   */
  Dialer createDialer(const char* endpoint) {
    return Dialer(socket_, endpoint);
  }

  void checkEndpoint(const char* endpoint) {
    nng_listener m_listener = {0};
    auto ret = nng_listener_create(&m_listener, socket_, endpoint);
    handleError(ret);
    ret = nng_listener_start(m_listener, 0);
    if (ret > 0) {
      nng_listener_close(m_listener);
    }
    handleError(ret);
    nng_listener_close(m_listener);
  }

  void listen(const char* endpoint) {
    auto ret = nng_listen(socket_, endpoint, NULL, 0);
    handleError(ret);
  }

  std::size_t read(char* buffer, std::size_t size) {
    static_assert(ProtocolTraits::can_receive::value,
                  "This socket cannot read because the used protocol doesn't "
                  "support this.");
    auto ret = nng_recv(socket_, buffer, &size, 0);
    handleError(ret);
    return size;
  }

  Buffer read() {
    static_assert(ProtocolTraits::can_receive::value,
                  "This socket cannot read because the used protocol doesn't "
                  "support this.");
    void* data = nullptr;
    std::size_t size = 0;
    auto ret = nng_recv(socket_, &data, &size, NNG_FLAG_ALLOC);
    Buffer buffer{data, size};
    handleError(ret);
    return buffer;
  }

  template <typename ConstBuffer>
  bool send(const ConstBuffer& buffer, Flags flags = Flags::none) {
    static_assert(ProtocolTraits::can_send::value,
                  "This socket cannot send because the used protocol doesn't "
                  "support this.");
    auto ret = nng_send(socket_, (void*)(buffer.data()), buffer.size(),
                        static_cast<int>(flags));
    if (ret == NNG_EAGAIN) {
      return false;
    }
    handleError(ret);
    return true;
  }

  bool send(char* buffer, std::size_t size, Flags flags = Flags::none) {
    static_assert(ProtocolTraits::can_send::value,
                  "This socket cannot send because the used protocol doesn't "
                  "support this.");
    auto ret = nng_send(socket_, buffer, size, static_cast<int>(flags));
    if (ret == NNG_EAGAIN) {
      return false;
    }
    handleError(ret);
    return true;
  }

  bool send(Buffer&& buffer, Flags flags = Flags::none) {
    static_assert(ProtocolTraits::can_send::value,
                  "This socket cannot send because the used protocol doesn't "
                  "support this.");

    int nng_flags = static_cast<int>(flags) | NNG_FLAG_ALLOC;
    auto ret = nng_send(socket_, buffer.data(), buffer.size(), nng_flags);
    if (ret == NNG_EAGAIN) {
      return false;
    }
    handleError(ret);
    // docs says that nng_send get ownership of data on _success_,
    // so we only release it after we checked for errors.
    buffer.release();
    return true;
  }

  template <typename ConstBuffer, typename CompletionHandler>
  void asyncSend(const ConstBuffer& buffer, CompletionHandler handler) {
    static_assert(ProtocolTraits::can_send::value,
                  "This socket cannot send because the used protocol doesn't "
                  "support this.");

    aio_->send(socket_, createMessageFrom(buffer), handler);
  }

  template <typename CompletionHandler>
  void asyncRead(CompletionHandler handler) {
    static_assert(ProtocolTraits::can_receive::value,
                  "This socket cannot read because the used protocol doesn't "
                  "support this.");
    aio_->read(socket_, handler);
  }

  /**
   * Wait for an async operation to finish.
   * Returns immediately if no async operation is currently running.
   *
   * @see AsyncIO::wait()
   */
  void asyncWait() { aio_->wait(); }

  /**
   * Stops an async operation.
   *
   * If an operation is currently in progress and aborted by this function, the
   * operation is cancelled and any associated callback is **not** called.
   *
   * If no operation is currently in progress, this function has no effect.
   *
   * @sa https://nanomsg.github.io/nng/man/tip/nng_aio_stop.3
   */
  void asyncStop() { aio_->stop(); }

  /**
   * Cancels an async operation and waits for it to complete or to be completely
   * aborted.
   *
   * If an operation is currently in progress and aborted by this function, the
   * operation is cancelled and the callback will receive an error
   * `NNG_ECANCELED`.
   *
   * If no operation is currently in progress, this function has no effect.
   *
   * @sa https://nanomsg.github.io/nng/man/tip/nng_aio_cancel.3
   */
  void asyncCancel() { aio_->cancel(); }

  /**
   * Register an `PipeEventHandler` @a handler that will be called
   * when a `PipeEvent` of type @a event occurs.
   *
   * A different handler can be registered for each type.
   * Any previously registered handler for this event type will be discarded.
   *
   * The same event handler can be registered for multiple events as well.
   *
   * @see https://nanomsg.github.io/nng/man/v1.1.0/nng_pipe_notify.3
   */
  template <typename PipeEventHandler>
  void onPipeEvent(PipeEvent event, PipeEventHandler handler) {
    eventDispatcher_.onPipeEvent(event, handler);
  }

  template <typename Option>
  void setOpt(Option option, const void* value, std::size_t size) {
    using ValidOptions = typename ProtocolTraits::Options;
    static_assert(boost::mp11::mp_contains<ValidOptions, Option>::value,
                  "Not a valid option for this socket type");
    auto ret = nng_setopt(socket_, option.name(), value, size);
    handleError(ret);
  }

  template <typename Option>
  void setOpt(Option option, std::chrono::milliseconds duration) {
    using ValidOptions = typename ProtocolTraits::Options;
    static_assert(boost::mp11::mp_contains<ValidOptions, Option>::value,
                  "Not a valid option for this socket type");
    auto ret = nng_setopt_ms(socket_, option.name(), duration.count());
    handleError(ret);
  }
  nng_socket handle() { return socket_; }

 private:
  std::unique_ptr<AsyncIO> aio_;
  detail::PipeEventDispatcher eventDispatcher_;
  nng_socket socket_ = NNG_SOCKET_INITIALIZER;
};
}  // namespace nng
