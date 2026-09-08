#include "communication/metadata_sender.hpp"
namespace ear {
namespace plugin {
namespace communication {

MetadataSender::MetadataSender(
    DataWrapper& data,
    std::shared_ptr<spdlog::logger> logger)
    : data_{data},
      logger_{std::move(logger)},
      timer_{std::make_unique<nng::AsyncIO>()},
      maxSendInterval_{std::chrono::milliseconds(250)},
      lastSendTimestamp_{std::chrono::system_clock::now()} {}

MetadataSender::~MetadataSender() {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  shuttingDown_ = true;
  stopTimerLocked();
  socket_.asyncStop();
}

ConnectionId MetadataSender::connectionId() {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  return connectionId_;
}

void MetadataSender::disconnect() {
  EAR_LOGGER_TRACE(logger_, "Disconnecting from metadata endpoint");
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  shuttingDown_ = true;
  stopTimerLocked();
  socket_.asyncCancel();
  socket_.asyncWait();
  sendInProgress_.store(false);
  dialer_.close();
  connectionId_ = ConnectionId{};
  socket_ = nng::PushSocket{};
}

void MetadataSender::triggerSend(bool force) {
  std::unique_lock<std::mutex> lock(lifecycleMutex_, std::try_to_lock);
  if (!lock.owns_lock() || shuttingDown_ || !connectionId_.isValid() ||
      sendInProgress_.load()) {
    return;
  }
  if (!force && !data_.readAccess([](auto const& item) {
        return item.changed();
      })) {
    return;
  }

  auto msg = data_.prepareMessage();
  sendInProgress_.store(true);
  try {
    socket_.asyncSend(
        msg, [this](std::error_code ec, const nng::Message& /*ignored*/) {
          sendInProgress_.store(false);
          if (!ec) {
            std::lock_guard<std::mutex> lock(timeoutMutex_);
            lastSendTimestamp_ = std::chrono::system_clock::now();
          } else {
            // this sets changed flag
            data_.writeAccess([](auto) {});
            EAR_LOGGER_WARN(logger_, "Metadata sending failed: {}",
                            ec.message());
          }
        });
  } catch (const std::exception&) {
    sendInProgress_.store(false);
    throw;
  }
}

void MetadataSender::stopTimerLocked() {
  timerRunning.store(false);
  if (timer_) {
    timer_->stop();
    timer_.reset();
  }
}

void MetadataSender::startTimerLocked() {
  using namespace std::chrono_literals;
  if (shuttingDown_ || !timer_ || maxSendInterval_ <= 0ms) {
    return;
  }

  bool expected{false};
  if (timerRunning.compare_exchange_strong(expected, true)) {
    timer_->sleep(maxSendInterval_ + 5ms,
                  std::bind(&MetadataSender::handleTimeout, this,
                            nng::placeholders::ErrorCode));
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
      triggerSend(true);
    }
    std::unique_lock<std::mutex> lock(lifecycleMutex_, std::try_to_lock);
    if (lock.owns_lock()) {
      startTimerLocked();
    }
  }
}
void MetadataSender::logger(std::shared_ptr<spdlog::logger> logger) {
  logger_ = std::move(logger);
}

void MetadataSender::connect(const std::string& endpoint, ConnectionId id) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  if (!timer_) {
    timer_ = std::make_unique<nng::AsyncIO>();
  }
  shuttingDown_ = false;
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
    startTimerLocked();
  });
}
}  // namespace communication
}  // namespace plugin
}  // namespace ear
