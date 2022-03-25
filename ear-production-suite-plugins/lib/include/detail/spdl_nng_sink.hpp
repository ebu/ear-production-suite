#include "nng-cpp/protocols/push.hpp"
#include "log_config.hpp"
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/version.h>
#include <mutex>
#include <boost/circular_buffer.hpp>
#include <string>

namespace ear {
namespace plugin {
namespace detail {

template <class Mutex>
class NNGSink : public spdlog::sinks::base_sink<Mutex> {
  using base_sink = spdlog::sinks::base_sink<Mutex>;

 public:
  NNGSink() : messageQueue_(512), base_sink(), socket_() {
#ifdef EPS_ENABLE_LOGGING
    socket_.dial(ear::plugin::detail::DEFAULT_LOG_ENDPOINT,
                 nng::Flags::nonblock);
#endif
  }
  virtual ~NNGSink() {}

 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override {
#ifdef EPS_ENABLE_LOGGING
#if SPDLOG_VER_MAJOR <= 1 && SPDLOG_VER_MINOR <= 3
    fmt::memory_buffer formatted;
#else
    spdlog::memory_buf_t formatted;
#endif
    base_sink::formatter_->format(msg, formatted);
    auto message = fmt::to_string(formatted);
    bool sendAlreadyInProgress = !messageQueue_.empty();
    messageQueue_.push_back(message);
    if (!sendAlreadyInProgress) {
      // send the next message, but leave in the queue until sent to indicate
      // work-in-progress´
      sendMessage(message);
    }
#endif
  }
  void flush_() override {}

 private:
  void sendMessage(std::string message) {
#ifdef EPS_ENABLE_LOGGING
    socket_.asyncSend(message,
                      [this](std::error_code ec, const nng::Message& m) {
                        this->handleSend(ec, m);
                      });
#endif
  }

  void handleSend(std::error_code ec, const nng::Message& /*ignored*/) {
    // No matter if sending was successfull or not, just discard message and
    // stop. What else could we do? Write to stderr?
    if (ec) {
      return;
    }
    std::lock_guard<Mutex> lock(base_sink::mutex_);
    messageQueue_.pop_front();
    if (!messageQueue_.empty()) {
      sendMessage(messageQueue_.front());
    }
  }

 private:
  boost::circular_buffer<std::string> messageQueue_;
  nng::PushSocket socket_;
};

using NNGSink_st = NNGSink<spdlog::details::null_mutex>;
using NNGSink_mt = NNGSink<std::mutex>;
}  // namespace detail
}  // namespace plugin
}  // namespace ear
