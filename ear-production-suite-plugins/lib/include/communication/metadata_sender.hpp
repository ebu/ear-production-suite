#pragma once
#include <chrono>
#include <mutex>
#include <atomic>
#include <functional>
#include <string>
#include <system_error>
#include "common_types.hpp"
#include "log.hpp"
#include "message_buffer.hpp"
#include "data_wrapper.hpp"
#include "nng-cpp/nng.hpp"

namespace ear::plugin::communication {

class MetadataSender {
 public:
  explicit MetadataSender(DataWrapper& data,
                          std::shared_ptr<spdlog::logger> logger = nullptr);
  ~MetadataSender();
  void connect(const std::string& endpoint,
               ConnectionId id);
  ConnectionId connectionId();
  void disconnect();
  void triggerSend();
  void logger(std::shared_ptr<spdlog::logger> logger);
 private:
  void startTimer();
  void handleTimeout(std::error_code ec);
  DataWrapper& data_;
  std::shared_ptr<spdlog::logger> logger_;
  nng::PushSocket socket_;
  nng::AsyncIO timer_;
  nng::Dialer dialer_;
  std::mutex timeoutMutex_;
  std::mutex sendMutex_;
  ConnectionId connectionId_;
  std::chrono::system_clock::time_point lastSendTimestamp_;
  std::chrono::milliseconds maxSendInterval_;
  std::atomic<bool> timerRunning{false};
};
}
