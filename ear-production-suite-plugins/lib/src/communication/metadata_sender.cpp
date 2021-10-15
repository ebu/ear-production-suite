#include "communication/metadata_sender.hpp"
namespace ear {
namespace plugin {
namespace communication {

MetadataSender::MetadataSender(
    DataWrapper& data,
    std::shared_ptr<spdlog::logger> logger)
    : data_{data},
      logger_{std::move(logger)},
      maxSendInterval_{std::chrono::milliseconds(100)},
      lastSendTimestamp_{std::chrono::system_clock::now()} {}

MetadataSender::~MetadataSender() {
  timer_.stop();
  timer_.wait();
}

ConnectionId MetadataSender::connectionId() { return connectionId_; }

void MetadataSender::disconnect() {
  timer_.cancel();
  timer_.wait();
  socket_.asyncCancel();
  socket_.asyncWait();
  dialer_.close();
  connectionId_ = ConnectionId{};
  socket_ = nng::PushSocket{};
}

void MetadataSender::triggerSend() {
  std::lock_guard<std::mutex> lock(sendMutex_);
  socket_.asyncWait();
  if (!connectionId_.isValid()) {
    return;
  }
  auto msg = data_.prepareMessage();
  socket_.asyncSend(
      msg, [this](std::error_code ec, const nng::Message& ignored) {
        if (!ec) {
          std::lock_guard<std::mutex> lock(timeoutMutex_);
          lastSendTimestamp_ = std::chrono::system_clock::now();
        } else {
          // this sets changed flag
          data_.writeAccess([](auto){});
          EAR_LOGGER_WARN(logger_, "Metadata sending failed: {}", ec.message());
        }
      });
}

void MetadataSender::startTimer() {
  using namespace std::chrono_literals;
  if (maxSendInterval_ > 0ms) {
    bool expected{false};
    if (timerRunning.compare_exchange_strong(expected, true)) {
      timer_.sleep(maxSendInterval_,
                   std::bind(&MetadataSender::handleTimeout, this,
                             nng::placeholders::ErrorCode));
    }
  }
}

void MetadataSender::handleTimeout(std::error_code ec) {
  timerRunning.store(false);
  if (!ec) {
    auto now = std::chrono::system_clock::now();
    std::chrono::system_clock::duration deltaT{0};
    {
      std::lock_guard<std::mutex> lock(timeoutMutex_);
      deltaT = now - lastSendTimestamp_;
    }
    if (deltaT > maxSendInterval_) {
      triggerSend();
    }
    startTimer();
  }
}
void MetadataSender::logger(std::shared_ptr<spdlog::logger> logger) {
  logger_ = std::move(logger);
}

void MetadataSender::connect(const std::string& endpoint, ConnectionId id) {
  data_.writeAccess([this, &id, &endpoint](auto data) {
    connectionId_ = id;
    data->set_connection_id(connectionId_.string());
    // set data changed flag to trigger/ sending metadata
    // to the scene master when the connection has been established,
    // even if the data hasn't ""changed"" from the object input point of view.
    EAR_LOGGER_DEBUG(logger_, "Connecting metadata stream to {}", endpoint);
    dialer_ = socket_.createDialer(endpoint.c_str());
    dialer_.start();
    EAR_LOGGER_DEBUG(logger_, "Metadata stream connected", endpoint);
    startTimer();
  });
}
}  // namespace communication
}  // namespace plugin
}  // namespace ear
