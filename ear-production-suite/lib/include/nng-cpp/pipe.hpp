#pragma once
#include "error_handling.hpp"
#include "ear-plugin-base/config.h"
#include <nng/nng.h>

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

/**
 * Internal utility class to implement pipe notification callbacks for sockets.
 */
class PipeEventDispatcher {
 public:
  PipeEventDispatcher() : socket_(NNG_SOCKET_INITIALIZER) {}
  PipeEventDispatcher(const PipeEventDispatcher&) = delete;
  PipeEventDispatcher& operator=(const PipeEventDispatcher&) = delete;
  ~PipeEventDispatcher() {
    if (nng_socket_id(socket_) != 0) {
      if (pipeAddPreHandler_) {
        nng_pipe_notify(socket_, NNG_PIPE_EV_ADD_PRE, NULL, NULL);
        pipeAddPreHandler_ = nullptr;
      }
      if (pipeAddPostHandler_) {
        nng_pipe_notify(socket_, NNG_PIPE_EV_ADD_POST, NULL, NULL);
        pipeAddPostHandler_ = nullptr;
      }
      if (pipeRemPostHandler_) {
        nng_pipe_notify(socket_, NNG_PIPE_EV_REM_POST, NULL, NULL);
        pipeRemPostHandler_ = nullptr;
      }
    }
  }
  PipeEventDispatcher(PipeEventDispatcher&& other) {
    socket_ = other.socket_;
    if (nng_socket_id(socket_) != -1) {
      onPipeEvent(PipeEvent::preAdd, other.pipeAddPreHandler_);
      onPipeEvent(PipeEvent::postAdd, other.pipeAddPostHandler_);
      onPipeEvent(PipeEvent::postRemove, other.pipeRemPostHandler_);
    }
    other.socket_ = NNG_SOCKET_INITIALIZER;
    other.pipeAddPreHandler_ = nullptr;
    other.pipeAddPostHandler_ = nullptr;
    other.pipeRemPostHandler_ = nullptr;
  }

  PipeEventDispatcher& operator=(PipeEventDispatcher&& other) {
    socket_ = other.socket_;
    if (nng_socket_id(socket_) != -1) {
      onPipeEvent(PipeEvent::preAdd, other.pipeAddPreHandler_);
      onPipeEvent(PipeEvent::postAdd, other.pipeAddPostHandler_);
      onPipeEvent(PipeEvent::postRemove, other.pipeRemPostHandler_);
    }
    other.socket_ = NNG_SOCKET_INITIALIZER;
    other.pipeAddPreHandler_ = nullptr;
    other.pipeAddPostHandler_ = nullptr;
    other.pipeRemPostHandler_ = nullptr;
    return *this;
  }
  template <typename EventHandler>
  void onPipeEvent(PipeEvent event, EventHandler handler) {
    bool eventAlreadyRegistered = false;
    switch (event) {
      case PipeEvent::preAdd:
        if (pipeAddPreHandler_) {
          eventAlreadyRegistered = true;
        }
        pipeAddPreHandler_ = handler;
        if (!eventAlreadyRegistered) {
          nng_pipe_notify(socket_, NNG_PIPE_EV_ADD_PRE, pipe_notify_dispatch,
                          this);
        }
        break;
      case PipeEvent::postAdd:
        if (pipeAddPostHandler_) {
          eventAlreadyRegistered = true;
        }
        pipeAddPostHandler_ = handler;
        if (!eventAlreadyRegistered) {
          nng_pipe_notify(socket_, NNG_PIPE_EV_ADD_POST, pipe_notify_dispatch,
                          this);
        }
        break;
      case PipeEvent::postRemove:
        if (pipeRemPostHandler_) {
          eventAlreadyRegistered = true;
        }
        pipeRemPostHandler_ = handler;
        if (!eventAlreadyRegistered) {
          nng_pipe_notify(socket_, NNG_PIPE_EV_REM_POST, pipe_notify_dispatch,
                          this);
        }
        break;
    }
  }

  void attach(nng_socket socket) {
    if (nng_socket_id(socket_) != -1) {
      throw std::runtime_error("Event dispatcher already attached to a socket");
    }
    socket_ = socket;
  }

 private:
  friend void pipe_notify_dispatch(nng_pipe pipe, pipe_event_t ev, void* arg);
  void dispatch(nng_pipe pipe, pipe_event_t event) {
    switch (event) {
      case NNG_PIPE_EV_ADD_PRE:
        if (pipeAddPreHandler_) {
          pipeAddPreHandler_(Pipe(pipe), PipeEvent::preAdd);
        }
        break;
      case NNG_PIPE_EV_ADD_POST:
        if (pipeAddPostHandler_) {
          pipeAddPostHandler_(Pipe(pipe), PipeEvent::postAdd);
        }
        break;
      case NNG_PIPE_EV_REM_POST:
        if (pipeRemPostHandler_) {
          pipeRemPostHandler_(Pipe(pipe), PipeEvent::postRemove);
        }
        break;
    }
  }
  std::function<void(Pipe, PipeEvent event)> pipeAddPreHandler_;
  std::function<void(Pipe, PipeEvent event)> pipeAddPostHandler_;
  std::function<void(Pipe, PipeEvent event)> pipeRemPostHandler_;
  nng_socket socket_;
};

void pipe_notify_dispatch(nng_pipe pipe, pipe_event_t ev, void* arg) {
  static_cast<PipeEventDispatcher*>(arg)->dispatch(pipe, ev);
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
