#pragma once
#include "error_handling.hpp"
#include "ear-plugin-base/config.h"
#include <nng/nng.h>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

namespace nng {
/**
 * Types of events for pipe notifications.
 *
 * @see https://nanomsg.github.io/nng/man/v1.1.0/nng_pipe_notify.3
 */
enum class PipeEvent {
  preAdd = NNG_PIPE_EV_ADD_PRE,  ///< A new pipe is about to be added
  postAdd = NNG_PIPE_EV_ADD_POST,  ///< A new pipe has just been added
  postRemove = NNG_PIPE_EV_REM_POST  ///< A pipe has been removed
};

/**
 *  @brief `nng_pipe` handle
 *
 * Holds a `nng_pipe`, but does neither create, destroy or
 * close it automatically, i.e. behaves exactly like `nng_pipe`.
 *
 * @see `nng_pipe` https://nanomsg.github.io/nng/man/v1.1.0/nng_pipe.5
 */
class Pipe {
 public:
  Pipe() : handle_(NNG_PIPE_INITIALIZER) {}
  explicit Pipe(nng_pipe pipe) : handle_(pipe) {}

  nng_pipe handle() { return handle_; };

  /**
   * Close the pipe, equivalent to `nng_pipe_close`
   */
  void close() {
    auto ret = nng_pipe_close(handle_);
    handleError(ret);
  }

 private:
  nng_pipe handle_;
};

namespace detail {
// two things happened:
// nng suddenly changed the callback signature (from int to nng_pipe_ev).
// This is currently not yet released, but in the master. (2019/05/10)
// To make things even more fun, vcpkg decided to use this master version
// instead of the latest release, which is still used by homebrew, for example.
#ifdef NEW_NNG_PIPE_NOTIFY_CALLBACK_SIGNATURE
using pipe_event_t = nng_pipe_ev;
#else
using pipe_event_t = int;
#endif
inline void pipe_notify_dispatch(nng_pipe pipe, pipe_event_t ev, void*);

class PipeEventState : public std::enable_shared_from_this<PipeEventState> {
 public:
  PipeEventState() = default;
  ~PipeEventState() { stop(); }

  PipeEventState(const PipeEventState&) = delete;
  PipeEventState& operator=(const PipeEventState&) = delete;

  void setHandler(PipeEvent event,
                  std::function<void(Pipe, PipeEvent)> handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    handlerFor(event) = std::move(handler);
    if (event != PipeEvent::preAdd && !worker_.joinable()) {
      auto self = shared_from_this();
      worker_ = std::thread([self] { self->run(); });
    }
  }

  void dispatchPreAdd(Pipe pipe) {
    std::function<void(Pipe, PipeEvent)> handler;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (stopping_) {
        return;
      }
      handler = pipeAddPreHandler_;
    }
    if (handler) {
      handler(pipe, PipeEvent::preAdd);
    }
  }

  void post(Pipe pipe, PipeEvent event) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (stopping_) {
        return;
      }
      events_.emplace_back(pipe, event);
    }
    condition_.notify_one();
  }

  void stop() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (!stopping_) {
        stopping_ = true;
        events_.clear();
        pipeAddPreHandler_ = nullptr;
        pipeAddPostHandler_ = nullptr;
        pipeRemPostHandler_ = nullptr;
      }
    }
    condition_.notify_one();
    std::lock_guard<std::mutex> joinLock(joinMutex_);
    if (worker_.joinable()) {
      if (worker_.get_id() == std::this_thread::get_id()) {
        worker_.detach();
      } else {
        worker_.join();
      }
    }
  }

 private:
  std::function<void(Pipe, PipeEvent)>& handlerFor(PipeEvent event) {
    switch (event) {
      case PipeEvent::preAdd:
        return pipeAddPreHandler_;
      case PipeEvent::postAdd:
        return pipeAddPostHandler_;
      case PipeEvent::postRemove:
        return pipeRemPostHandler_;
    }
    return pipeRemPostHandler_;
  }

  void run() {
    while (true) {
      std::function<void(Pipe, PipeEvent)> handler;
      std::pair<Pipe, PipeEvent> event;
      {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return stopping_ || !events_.empty(); });
        if (stopping_) {
          return;
        }
        event = events_.front();
        events_.pop_front();
        handler = handlerFor(event.second);
      }
      if (handler) {
        handler(event.first, event.second);
      }
    }
  }

  std::mutex mutex_;
  std::mutex joinMutex_;
  std::condition_variable condition_;
  std::deque<std::pair<Pipe, PipeEvent>> events_;
  std::function<void(Pipe, PipeEvent)> pipeAddPreHandler_;
  std::function<void(Pipe, PipeEvent)> pipeAddPostHandler_;
  std::function<void(Pipe, PipeEvent)> pipeRemPostHandler_;
  bool stopping_{false};
  std::thread worker_;
};

/**
 * Internal utility class to implement pipe notification callbacks for sockets.
 */
class PipeEventDispatcher {
 public:
  PipeEventDispatcher() : socket_(NNG_SOCKET_INITIALIZER) {}
  PipeEventDispatcher(const PipeEventDispatcher&) = delete;
  PipeEventDispatcher& operator=(const PipeEventDispatcher&) = delete;
  ~PipeEventDispatcher() {
    if (state_) {
      state_->stop();
    }
  }

  PipeEventDispatcher(PipeEventDispatcher&& other) noexcept
      : state_(std::move(other.state_)), socket_(other.socket_) {
    other.socket_ = NNG_SOCKET_INITIALIZER;
  }

  PipeEventDispatcher& operator=(PipeEventDispatcher&& other) noexcept {
    if (this != &other) {
      if (state_) {
        state_->stop();
      }
      state_ = std::move(other.state_);
      socket_ = other.socket_;
      other.socket_ = NNG_SOCKET_INITIALIZER;
    }
    return *this;
  }

  template <typename EventHandler>
  void onPipeEvent(PipeEvent event, EventHandler handler) {
    if (!state_) {
      throw std::runtime_error("Event dispatcher is not attached to a socket");
    }
    state_->setHandler(event, std::move(handler));
    switch (event) {
      case PipeEvent::preAdd:
        handleError(nng_pipe_notify(socket_, NNG_PIPE_EV_ADD_PRE,
                                    pipe_notify_dispatch, state_.get()));
        break;
      case PipeEvent::postAdd:
        handleError(nng_pipe_notify(socket_, NNG_PIPE_EV_ADD_POST,
                                    pipe_notify_dispatch, state_.get()));
        break;
      case PipeEvent::postRemove:
        handleError(nng_pipe_notify(socket_, NNG_PIPE_EV_REM_POST,
                                    pipe_notify_dispatch, state_.get()));
        break;
    }
  }

  void attach(nng_socket socket) {
    if (state_) {
      throw std::runtime_error("Event dispatcher already attached to a socket");
    }
    socket_ = socket;
    state_ = std::make_shared<PipeEventState>();
  }

  void quiesce() {
    if (!state_) {
      return;
    }
    if (nng_socket_id(socket_) > 0) {
      nng_pipe_notify(socket_, NNG_PIPE_EV_ADD_PRE, NULL, NULL);
      nng_pipe_notify(socket_, NNG_PIPE_EV_ADD_POST, NULL, NULL);
      nng_pipe_notify(socket_, NNG_PIPE_EV_REM_POST, NULL, NULL);
    }
    state_->stop();
  }

  void reset() {
    state_.reset();
    socket_ = NNG_SOCKET_INITIALIZER;
  }

 private:
  std::shared_ptr<PipeEventState> state_;
  nng_socket socket_;
};

void pipe_notify_dispatch(nng_pipe pipe, pipe_event_t event, void* arg) {
  auto state = static_cast<PipeEventState*>(arg);
  switch (event) {
    case NNG_PIPE_EV_ADD_PRE:
      state->dispatchPreAdd(Pipe(pipe));
      break;
    case NNG_PIPE_EV_ADD_POST:
      state->post(Pipe(pipe), PipeEvent::postAdd);
      break;
    case NNG_PIPE_EV_REM_POST:
      state->post(Pipe(pipe), PipeEvent::postRemove);
      break;
    case NNG_PIPE_EV_NUM:
      break;
  }
}
}  // namespace detail

#ifdef DOXYGEN_DOC_GENERATION
/**
 * @interface PipeEventHandler
 *
 * A `PipeEventHandler` is just a *concept* for anything that can be passed as
 * callback when listening for pipe events, *not* a real class.
 *
 * A `PipeEventHandler` can be any C++ callable
 * (function, function object, lambda, etc.) that can be called using the
 * signature `void(nng::Pipe, nng::PipeEvent event)`.
 *
 *
 * Here's an example of a lambda used as EventHandler:
 @code
 using namespace nng;
 RepSocket socket;
 socket.onPipeEvent(nng::PipeEvent::postAdd,
                    [](nng::Pipe pipe, nng::PipeEvent event) {
        assert(event == nng::PipeEvent::postAdd);
        std::cout<<"A new connection has been established."<<std::endl;
        }));
 @endcode
 * @see https://nanomsg.github.io/nng/man/v1.1.0/nng_pipe_notify.3
 */
struct PipeEventHandler {
  /**
   * Signature of callbacks for handling pipe events.
   *
   * In case if an `PipeEvent::preAdd` the `EventHandler` has a chance to
   * `Pipe::close` the pipe to and the socket will never actually "see" the
   * pipe.
   *
   * @param event The type of the event
   * @param pipe The pipe affected by the event
   */
  void operator()(nng::Pipe pipe, nng::PipeEvent event);
};
#endif
}  // namespace nng
