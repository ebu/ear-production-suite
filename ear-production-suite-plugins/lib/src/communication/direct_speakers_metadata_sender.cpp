#include "communication/direct_speakers_metadata_sender.hpp"
#include "helper/protobuf_utilities.hpp"

using namespace std::chrono_literals;

namespace ear {
namespace plugin {

namespace communication {

DirectSpeakersMetadataSender::DirectSpeakersMetadataSender(
    std::shared_ptr<spdlog::logger> logger)
    : logger_(logger), maxSendInterval_(100ms) {
  lastSendTimestamp_ = std::chrono::system_clock::now();
  data_.set_allocated_ds_metadata(new proto::DirectSpeakersTypeMetadata{});
}

DirectSpeakersMetadataSender::~DirectSpeakersMetadataSender() {
  timer_.stop();
  timer_.wait();
}

void DirectSpeakersMetadataSender::logger(
    std::shared_ptr<spdlog::logger> logger) {
  logger_ = logger;
}

void DirectSpeakersMetadataSender::connect(const std::string& endpoint,
                                           ConnectionId id) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  connectionId_ = id;
  data_.set_connection_id(connectionId_.string());
  // set data changed flag to trigger/ sending metadata
  // to the scene master when the connection has been established,
  // even if the data hasn't ""changed"" from the object input point of view.
  data_.set_changed(true);
  EAR_LOGGER_DEBUG(logger_, "Connecting metadata stream to {}", endpoint);
  dialer_ = socket_.createDialer(endpoint.c_str());
  dialer_.start();
  EAR_LOGGER_DEBUG(logger_, "Metadata stream connected", endpoint);
  startTimer();
}

MessageBuffer DirectSpeakersMetadataSender::prepareMessage() {
  std::lock_guard<std::mutex> lock(dataMutex_);
  MessageBuffer buffer = allocBuffer(data_.ByteSizeLong());
  data_.SerializeToArray(buffer.data(), buffer.size());
  data_.set_changed(false);
  return buffer;
}

void DirectSpeakersMetadataSender::disconnect() {
  timer_.cancel();
  timer_.wait();
  socket_.asyncCancel();
  socket_.asyncWait();
  dialer_.close();
  connectionId_ = ConnectionId{};
  socket_ = nng::PushSocket{};
}

void DirectSpeakersMetadataSender::triggerSend() { sendMetadata(); }

void DirectSpeakersMetadataSender::sendMetadata() {
  std::lock_guard<std::mutex> lock(sendMutex_);
  socket_.asyncWait();
  if (!connectionId_.isValid()) {
    return;
  }
  auto msg = prepareMessage();
  socket_.asyncSend(
      msg, [this](std::error_code ec, const nng::Message& ignored) {
        if (!ec) {
          std::lock_guard<std::mutex> lock(timeoutMutex_);
          lastSendTimestamp_ = std::chrono::system_clock::now();
        } else {
          {
            std::lock_guard<std::mutex> dataLock(dataMutex_);
            data_.set_changed(true);
          }
          EAR_LOGGER_WARN(logger_, "Metadata sending failed: {}", ec.message());
        }
      });
}

void DirectSpeakersMetadataSender::startTimer() {
  if (maxSendInterval_ > 0ms) {
    bool expected{false};
    if(timerRunning.compare_exchange_strong(expected, true)) {
      timer_.sleep(maxSendInterval_,
                   std::bind(&DirectSpeakersMetadataSender::handleTimeout, this,
                             nng::placeholders::ErrorCode));
    }
  }
}

void DirectSpeakersMetadataSender::handleTimeout(std::error_code ec) {
  timerRunning.store(false);
  if (!ec) {
    auto now = std::chrono::system_clock::now();
    std::chrono::system_clock::duration deltaT{0};
    {
      std::lock_guard<std::mutex> lock(timeoutMutex_);
      deltaT = now - lastSendTimestamp_;
    }
    if (deltaT > maxSendInterval_) {
      sendMetadata();
    }
    startTimer();
  }
}

void DirectSpeakersMetadataSender::routing(int32_t value) {
  setData([value](auto data) { data->set_routing(value); });
}
void DirectSpeakersMetadataSender::name(const std::string& value) {
  setData([&value](auto data) { data->set_name(value); });
}
void DirectSpeakersMetadataSender::colour(int value) {
  setData([value](auto data) { data->set_colour(value); });
}
void DirectSpeakersMetadataSender::speakerSetupIndex(int value) {
  setData([value](auto data) {
    data->set_allocated_ds_metadata(
        proto::convertSpeakerSetupToEpsMetadata(value));
  });
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
