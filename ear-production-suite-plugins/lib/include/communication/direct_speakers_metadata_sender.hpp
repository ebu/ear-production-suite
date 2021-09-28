#pragma once

#include "common_types.hpp"
#include "message_buffer.hpp"
#include "nng-cpp/nng.hpp"
#include "log.hpp"
#include "input_item_metadata.pb.h"
#include <boost/optional.hpp>
#include <string>
#include <chrono>
#include <mutex>
#include <atomic>

namespace ear {
namespace plugin {
namespace communication {

class DirectSpeakersMetadataSender {
 public:
  DirectSpeakersMetadataSender(
      std::shared_ptr<spdlog::logger> logger = nullptr);
  ~DirectSpeakersMetadataSender();

  void logger(std::shared_ptr<spdlog::logger> logger);

  void connect(const std::string& endpoint, ConnectionId connectionId);
  void disconnect();

  void triggerSend();

  void routing(int32_t value);
  void name(const std::string& value);
  void colour(int value);
  void speakerSetupIndex(int value);

  ConnectionId getConnectionId() { return connectionId_; }

 private:
  void sendMetadata();
  void startTimer();
  MessageBuffer prepareMessage();
  void handleTimeout(std::error_code ec);
  template<typename FunctionT>
  void setData(FunctionT&& set) {
    std::lock_guard<std::mutex> dataLock{dataMutex_};
    data_.set_changed(true);
    set(&data_);
  }
  std::shared_ptr<spdlog::logger> logger_;
  nng::PushSocket socket_;
  nng::AsyncIO timer_;
  nng::Dialer dialer_;
  std::mutex dataMutex_;
  std::mutex timeoutMutex_;
  std::mutex sendMutex_;
  proto::InputItemMetadata data_;
  ConnectionId connectionId_;
  std::chrono::system_clock::time_point lastSendTimestamp_;
  std::chrono::milliseconds maxSendInterval_;
  std::atomic<bool> timerRunning{false};
};

}  // namespace communication
}  // namespace plugin
}  // namespace ear
