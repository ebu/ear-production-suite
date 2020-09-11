#pragma once
#include "error_handling.hpp"
#include "message.hpp"
#include "placeholders.hpp"
#include <nng/nng.h>
#include <memory>
#include <functional>
#include <chrono>

namespace nng {

class AsyncIO;

namespace detail {
inline void aio_dispatch_callback(void* arg);

class AsyncSendAction {
  public:
  using Handler = std::function<void(std::error_code ec, Message message)>;
  AsyncSendAction(AsyncIO* aio, const Handler& handler) : aio_(aio), callback_(handler) {}
  void operator()();

  private:
  AsyncIO* aio_;
  Handler callback_;
};

class AsyncReadAction {
  public:
    using Handler = std::function<void(std::error_code ec, Message message)>;
  AsyncReadAction(AsyncIO* aio, const Handler& handler) : aio_(aio), callback_(handler) {}
  void operator()();

  private:
  AsyncIO* aio_;
  Handler callback_;
};

class AsyncSleepAction {
  
  public:
  using Handler = std::function<void(std::error_code ec)>;
  AsyncSleepAction(AsyncIO* aio, const Handler& handler) : aio_(aio), callback_(handler) {}
  void operator()();

  private:
  AsyncIO* aio_;
  Handler callback_;
};
}

/**
 * @brief
 * Helper and wrapper for `nng_aio` for asynchronous operations.
 *
 * An `AsyncIO` object can be used to start asynchronous operations, for example
 * read or write from a socket or a simple sleep operation.
 *
 * On completion of an async operation, a user-provided `CompletionHandler` will
 * be called. A `CompletionHandler` can be any C++ callable (function, function
 * object, lambda, etc.) that that can be called using the signature
 * `void(std::error_code, nng::Message)`.
 *
 * For convenience / improved readability, the placeholders
 * `nng::placeholders::ErrorCode` and `nng::placeholders::Message` that can be
 * used with `std::bind` are provided.
 *
 * `AsyncIO` instances cannot be copied nor moved. The reason for this is that
 * it's close to impossible (at least very complicated) to ensure that the
 * internal `nng_aio` instance has not any running operations or is bound to the
 * correct object instance. The main reason for the latter is that the `nng_aio`
 * API does not allow to change a once registered callback.
 *
 * @warning
 * Only a single operation can be active at a time per `AsyncIO` instance.
 * The `AsyncIO` instance must stay valid until any asynchronous operation
 * completes.
 *
 * @note
 * For simple async read/write use cases it's usually not necessary to use an
 * `AsyncIO` instance, as `sockets` already have an internal instance that can
 * be used with the `asyncRead(...)`, `asyncWrite(...)` family of methods.
 *
 * @see `nng_aio`: https://nanomsg.github.io/nng/man/v1.1.0/nng_aio.5
 */
class AsyncIO {
 public:
  /// Constructor
  AsyncIO() : aio_(nullptr) {
    auto ret = nng_aio_alloc(&aio_, detail::aio_dispatch_callback, this);
    handleError(ret);
  }

  /**
   * @brief
   * Destructor
   *
   * Stops any currently running asynchronous operation.
   *
   * @see AsyncIO::stop()
   */
  ~AsyncIO() {
    nng_aio_stop(aio_);
    nng_aio_free(aio_);
  }

  AsyncIO(const AsyncIO&) = delete;
  AsyncIO& operator=(const AsyncIO&) = delete;
  AsyncIO(AsyncIO&& other) = delete;
  AsyncIO& operator=(AsyncIO&& other) = delete;

  /**
   * @brief
   * Send the message `message` asynchronously.
   *
   * The `CompletionHandler` @a handler will be called when
   * the send operation completes, either successfully or due to a failure.
   *
   * In case of an error, the `CompletionHandler` will receive message to be
   * send as an argument, which might be usefull to re-send it at a later point
   * in time.
   *
   * @see `nng_send_aio` https://nanomsg.github.io/nng/man/v1.1.0/nng_send_aio.3
   */
  template <typename CompletionHandler>
  void send(nng_socket socket, Message&& message, CompletionHandler handler) {
    action_ = detail::AsyncSendAction(this, handler);
    setMessage(std::move(message));
    nng_send_aio(socket, aio_);
  }

  /**
   * @brief
   * Receive a message asynchronously.
   *
   * The `CompletionHandler` @a handler will be called when
   * the read operation completes, either successfully or due to a failure.
   *
   * On success, the `CompletionHandler` is passed the received messageas an
   * argument
   *
   * @see `nng_recv_aio` https://nanomsg.github.io/nng/man/v1.1.0/nng_send_aio.3
   */
  template <typename CompletionHandler>
  void read(nng_socket socket, CompletionHandler handler) {
    action_ = detail::AsyncReadAction(this, handler);
    nng_recv_aio(socket, aio_);
  }

  /**
   * @brief
   * Start an asynchronous timer.
   *
   * The timer will call the `CompletionHandler` after the given time has
   * passed, the timer was cancelled or an error occured.
   *
   * In any case, the `CompletionHandler` will only receive an invalid Message
   * as parameter, as there is no Message that can be passed.
   *
   * @see `nng_sleep_aio`
   * https://nanomsg.github.io/nng/man/v1.1.0/nng_send_aio.3
   */
  template <typename CompletionHandler>
  void sleep(std::chrono::milliseconds time, CompletionHandler handler) {
    action_ = detail::AsyncSleepAction(this, handler);;
    nng_sleep_aio(time.count(), aio_);
  }

  /**
   * @brief Wait for an asynchronous operation to complete.
   *
   * This method blocks until the current async operation has completed.
   * If no async operation is currently running or has already been completed
   * it will return immediatly.
   *
   * @see  `nng_aio_wait`
   * https://nanomsg.github.io/nng/man/v1.1.0/nng_aio_wait.3
   */
  void wait() { nng_aio_wait(aio_); }

  /**
   * @brief Cancel an asynchronous operation.
   *
   * Cancels the currently running async operation, if any.
   * If an operation is cancelled, the `CompletionHandler` will be invoked
   * with an error code of `NNG_ECANCELED`.
   * If no operation is currently running this function has no effect.
   *
   * This function does not wait until the operation has been cancelled, but
does return immediately.
   *
   * @see `nng_aio_cancel`
https://nanomsg.github.io/nng/man/v1.1.0/nng_aio_cancel.3
   */
  void cancel() { nng_aio_cancel(aio_); }

  /**
   * @brief Stop an asynchronous operation.
   *
   * Stops the currently running async operation, if any.
   * If an operation is stopped, the `CompletionHandler` will *not* be invoked.
   *
   * The the `CompletionHandler` already has been invoked is currently in
   * progress, this method will block until it has finished.
   *
   * @note The `AsyncIO` instance cannot be used to schedule any new async
   * opertions afterwards.
   *
   * @see `nng_aio_stop` https://nanomsg.github.io/nng/man/v1.1.0/nng_aio_stop.3
   */
  void stop() { nng_aio_stop(aio_); }

 private:
  friend void detail::aio_dispatch_callback(void*);
  friend class detail::AsyncSendAction;
  friend class detail::AsyncReadAction;
  friend class detail::AsyncSleepAction;
  int result() { return nng_aio_result(aio_); }
  void setMessage(Message&& message) {
    nng_aio_set_msg(aio_, message.release());
  }
  Message message() { return Message(nng_aio_get_msg(aio_)); }

  void callback() {
    auto cb = action_;
    action_ = nullptr;
    cb();
  }
  nng_aio* aio_;
  std::function<void()> action_;
};

namespace detail {
void aio_dispatch_callback(void* arg) {
  static_cast<AsyncIO*>(arg)->callback();
}

  inline void AsyncSendAction::operator()() {
    if(!callback_) { return;}
    auto ec = makeErrorCode(aio_->result());
    // In case of an error, we must retrieve the message
    // and dispose it ourselves (or try to resend it, etc.)
    // On success, nng will dispose the message for us
    if(!ec) {
      callback_(ec, Message());
    }
    else {
      callback_(ec, aio_->message());
    }
  }

 inline void AsyncReadAction::operator()() {
   if(!callback_) { return;}
    // On success, we can and must retrieve the message
    // and dispose it ourselves 
    // On error, nng will dispose the allocated message 
    // or no message will be present at all
    auto ec = makeErrorCode(aio_->result());
    if(!ec) {
      callback_(ec, aio_->message());
    }
    else {
      callback_(ec, Message());
    }
  }

   inline void AsyncSleepAction::operator()() {
     if(!callback_) { return;}
    auto ec = makeErrorCode(aio_->result());
    callback_(ec);
  }

}  // namespace detail

#ifdef DOXYGEN_DOC_GENERATION
/**
 * @interface CompletionHandler
 *
 * A `CompletionHandler` is just a *concept* for anything that can be passed as
 * callback for async operations, *not* a real class.
 *
 * A `CompletionHandler` can be any C++ callable
 * (function, function object, lambda, etc.) that can be called using the
 * signature `void(std::error_code, nng::Message)`.
 *
 * Here's an example of a lambda used as CompletionHandler:
 @code
 using namespace nng;
 AsyncIO aio;
 aio.sleep(std::chrono::seconds(1), [](std::error_code ec, Message) {
   if(ec) {
     std::cerr<<"Failed to sleep: "<<ec.message()<<std::endl;
    }
    else {
      std::cout<<"Well, that was refreshing"<<std::endl;
    }
 }));
 @endcode
 */
struct CompletionHandler {
  /**
   * Signature of completion handler callbacks for async operations.
   *
   * The error code @a ec can/must be used to check if an operation has been
   * succesfull.
   *
   * The @a message parameter might either represent a valid message or an
   * invalid message, depending on the async operation (read/send/sleep) and if
   * this operation has been completed successfully.
   *
   * @param ec Error code of the operation
   * @param message Message, if any
   */
  void operator()(std::error_code ec, Message message);
};
#endif
}  // namespace nng
