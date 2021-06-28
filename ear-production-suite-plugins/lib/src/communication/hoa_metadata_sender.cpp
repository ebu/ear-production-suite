#include "communication/hoa_metadata_sender.hpp"
#include "helper/protobuf_utilities.hpp"

using namespace std::chrono_literals;

namespace ear {
namespace plugin {

namespace communication {

HoaMetadataSender::HoaMetadataSender(
    std::shared_ptr<spdlog::logger> logger)
    : logger_(logger), maxSendInterval_(100ms) {
  lastSendTimestamp_ = std::chrono::system_clock::now();
  // TODO - this is what defines the plugin as DirectSpeakers. We need to implement a set_allocated_hoa_metadata method and a proto::HoaTypeMetadata
  data_.set_allocated_ds_metadata(new proto::DirectSpeakersTypeMetadata{});
}

HoaMetadataSender::~HoaMetadataSender() {
  timer_.stop();
  timer_.wait();
}

void HoaMetadataSender::logger(
    std::shared_ptr<spdlog::logger> logger) {
  logger_ = logger;
}

void HoaMetadataSender::connect(const std::string& endpoint,
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

MessageBuffer HoaMetadataSender::getMessage() {
  std::lock_guard<std::mutex> lock(dataMutex_);
  MessageBuffer buffer = allocBuffer(data_.ByteSizeLong());
  data_.SerializeToArray(buffer.data(), buffer.size());
  return buffer;
}

void HoaMetadataSender::disconnect() {
  std::lock_guard<std::mutex> lock(dataMutex_);
  timer_.cancel();
  timer_.wait();
  socket_.asyncCancel();
  socket_.asyncWait();
  dialer_.close();
  connectionId_ = ConnectionId{};
  socket_ = nng::PushSocket{};
}

void HoaMetadataSender::triggerSend() { sendMetadata(); }

void HoaMetadataSender::sendMetadata() {
  std::lock_guard<std::mutex> lock(sendMutex_);
  socket_.asyncWait();
  if (!connectionId_.isValid()) {
    return;
  }
  auto msg = getMessage();
  socket_.asyncSend(
      msg, [this](std::error_code ec, const nng::Message& ignored) {
        if (!ec) {
          std::lock_guard<std::mutex> lock(timeoutMutex_);
          lastSendTimestamp_ = std::chrono::system_clock::now();
        } else {
          EAR_LOGGER_WARN(logger_, "Metadata sending failed: {}", ec.message());
        }
      });
}

void HoaMetadataSender::startTimer() {
  if (maxSendInterval_ > 0ms) {
    bool expected{false};
    if(timerRunning.compare_exchange_strong(expected, true)) {
      timer_.sleep(maxSendInterval_,
                   std::bind(&HoaMetadataSender::handleTimeout, this,
                             nng::placeholders::ErrorCode));
    }
  }
}

void HoaMetadataSender::handleTimeout(std::error_code ec) {
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

void HoaMetadataSender::routing(int32_t value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.set_changed(true);
  data_.set_routing(value);
}
void HoaMetadataSender::name(const std::string& value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.set_name(value);
}
void HoaMetadataSender::colour(int value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.set_colour(value);
}
//ME add

void HoaMetadataSender::hoa_type(int valueHOAtypeIndex) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  proto::HoaTypeMetadata* hoa_metadata;
  hoa_metadata->set_hoatypeindex(1);
  data_.set_allocated_hoa_metadata(hoa_metadata);
}
//ME end

/*
 Old DS Code
// Will need similar method for HOA type
void HoaMetadataSender::speakerSetupIndex(int value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.set_allocated_ds_metadata(
      proto::convertSpeakerSetupToEpsMetadata(value));
}
 */

}  // namespace communication
}  // namespace plugin
}  // namespace ear
