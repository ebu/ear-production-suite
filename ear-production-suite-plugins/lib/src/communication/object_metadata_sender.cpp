#include "communication/object_metadata_sender.hpp"

using namespace std::chrono_literals;

namespace ear {
namespace plugin {

namespace communication {

ObjectMetadataSender::ObjectMetadataSender(
    std::shared_ptr<spdlog::logger> logger)
    : logger_(logger), maxSendInterval_(100ms) {
  lastSendTimestamp_ = std::chrono::system_clock::now();
  data_.set_allocated_obj_metadata(new proto::ObjectsTypeMetadata{});
}

ObjectMetadataSender::~ObjectMetadataSender() {
  timer_.stop();
  timer_.wait();
}

void ObjectMetadataSender::logger(std::shared_ptr<spdlog::logger> logger) {
  logger_ = logger;
}

void ObjectMetadataSender::connect(const std::string& endpoint,
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

MessageBuffer ObjectMetadataSender::getMessage() {
  std::lock_guard<std::mutex> lock(dataMutex_);
  MessageBuffer buffer = allocBuffer(data_.ByteSizeLong());
  data_.SerializeToArray(buffer.data(), buffer.size());
  return buffer;
}

void ObjectMetadataSender::disconnect() {
  std::lock_guard<std::mutex> lock(dataMutex_);
  timer_.cancel();
  timer_.wait();
  socket_.asyncCancel();
  socket_.asyncWait();
  dialer_.close();
  connectionId_ = ConnectionId{};
  socket_ = nng::PushSocket{};
}

void ObjectMetadataSender::triggerSend() { sendMetadata(); }

void ObjectMetadataSender::sendMetadata() {
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
          data_.set_changed(false);
        } else {
          EAR_LOGGER_WARN(logger_, "Metadata sending failed: {}", ec.message());
        }
      });
}

void ObjectMetadataSender::startTimer() {
  if (maxSendInterval_ > 0ms) {
    bool expected{false};
    if(timerRunning.compare_exchange_strong(expected, true)) {
      timer_.sleep(maxSendInterval_,
                   std::bind(&ObjectMetadataSender::handleTimeout, this,
                             nng::placeholders::ErrorCode));
    }
  }
}

void ObjectMetadataSender::handleTimeout(std::error_code ec) {
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

void ObjectMetadataSender::name(const std::string& value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.set_name(value);
}
void ObjectMetadataSender::colour(int value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.set_colour(value);
}

void ObjectMetadataSender::routing(int32_t value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.set_changed(true);
  data_.set_routing(value);
}
void ObjectMetadataSender::gain(float value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.set_changed(true);
  data_.mutable_obj_metadata()->set_gain(value);
}

void ObjectMetadataSender::azimuth(float value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.set_changed(true);
  data_.mutable_obj_metadata()->mutable_position()->set_azimuth(value);
}
void ObjectMetadataSender::elevation(float value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.set_changed(true);
  data_.mutable_obj_metadata()->mutable_position()->set_elevation(value);
}
void ObjectMetadataSender::distance(float value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.mutable_obj_metadata()->mutable_position()->set_distance(value);
}

void ObjectMetadataSender::width(float value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.mutable_obj_metadata()->set_width(value);
}
void ObjectMetadataSender::height(float value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.mutable_obj_metadata()->set_height(value);
}
void ObjectMetadataSender::depth(float value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.mutable_obj_metadata()->set_depth(value);
}
void ObjectMetadataSender::diffuse(float value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.mutable_obj_metadata()->set_diffuse(value);
}
void ObjectMetadataSender::factor(float value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.mutable_obj_metadata()->set_factor(value);
}
void ObjectMetadataSender::range(float value) {
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_.mutable_obj_metadata()->set_range(value);
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
