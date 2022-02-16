#include "monitoring_backend.hpp"
#include "communication/monitoring_metadata_receiver.hpp"
#include "detail/constants.hpp"
#include <functional>

using std::placeholders::_1;
using std::placeholders::_2;

namespace ear {
namespace plugin {
MonitoringBackend::MonitoringBackend(
    ui::MonitoringFrontendBackendConnector* connector,
    const Layout& targetLayout, int inputChannelCount)
    : gainsCalculator_(targetLayout, inputChannelCount),
      frontendConnector_(connector),
      controlConnection_() {
  logger_ = createLogger(fmt::format("Monitoring@{}", (const void*)this));

#ifndef NDEBUG
  logger_->set_level(spdlog::level::trace);
#else
  logger_->set_level(spdlog::level::off);
#endif

  gains_.direct = gainsCalculator_.directGains();
  gains_.diffuse = gainsCalculator_.diffuseGains();
  controlConnection_.logger(logger_);
  controlConnection_.onConnectionEstablished(
      std::bind(&MonitoringBackend::onConnection, this, _1, _2));
  controlConnection_.onConnectionLost(
      std::bind(&MonitoringBackend::onConnectionLost, this));
  controlConnection_.start(detail::SCENE_MASTER_CONTROL_ENDPOINT);
}

MonitoringBackend::~MonitoringBackend() {
  if (metadataReceiver_) {
    metadataReceiver_->shutdown();
  }
  // remove connection signal handlers
  // this is required so the controlConnection_ does not try to invoke the
  // registered member function which might easily use already destructed
  // members. Another option might be to introduce a `stop()` method on the
  // InputControlConnection class, but probably
  // the expected behaviour of this would be to call any "disconnect" handlers
  // on `stop()` as well, so this wouldn't help here?
  controlConnection_.onConnectionLost(nullptr);
  controlConnection_.onConnectionEstablished(nullptr);
}

void MonitoringBackend::onSceneReceived(proto::SceneStore store) {
  isExporting_ = store.has_is_exporting() && store.is_exporting();
  updateActiveGains(std::move(store));
}

GainHolder MonitoringBackend::currentGains() {
  std::lock_guard<std::mutex> lock(gainsMutex_);
  return gains_;
}

void MonitoringBackend::updateActiveGains(proto::SceneStore store) {
  {
    std::lock_guard<std::mutex> lock(gainsCalculatorMutex_);
    gainsCalculator_.update(std::move(store));
  }
  {
    std::lock_guard<std::mutex> lock(gainsMutex_);
    gains_.direct = gainsCalculator_.directGains();
    gains_.diffuse = gainsCalculator_.diffuseGains();
  }
}

void MonitoringBackend::onConnection(communication::ConnectionId id,
                                     const std::string& streamEndpoint) {
  try {
    logger_->info(
        "Connected to Scene ({}), will now listening for metadata from "
        "{}",
        id.string(), streamEndpoint);
    metadataReceiver_ =
        std::make_unique<communication::MonitoringMetadataReceiver>(logger_);
    metadataReceiver_->start(
        streamEndpoint,
        std::bind(&MonitoringBackend::onSceneReceived, this, _1));
  } catch (const std::runtime_error& e) {
    logger_->error("Failed to start stream receiver: {}", e.what());
  }
}

void MonitoringBackend::onConnectionLost() {
  logger_->info("Lost connection to Scene");
  metadataReceiver_->shutdown();
  // force update with an "empty" store to generate silence
  updateActiveGains(proto::SceneStore{});
}

}  // namespace plugin
}  // namespace ear
