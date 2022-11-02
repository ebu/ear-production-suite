
#include "hoa_backend.hpp"

#include "ui/hoa_frontend_backend_connector.hpp"
#include "detail/constants.hpp"

using std::placeholders::_1;
using std::placeholders::_2;

namespace ear {
namespace plugin {

HoaBackend::HoaBackend(ui::HoaFrontendBackendConnector* connector)
    : logger_ {createLogger(fmt::format("HOA Input@{}", (const void*)this))},
    connector_(connector), controlConnection_(logger_) {

#ifdef EPS_ENABLE_LOGGING
  logger_->set_level(spdlog::level::trace);
#else
  logger_->set_level(spdlog::level::off);
#endif

  metadataSender_.logger(logger_);
  if (connector_) {
    connector_->setStatusBarText("Failure: No connection to Scene");
    connector_->onParameterChanged(
        std::bind(&HoaBackend::onParameterChanged, this, _1, _2));
  }
  controlConnection_.onConnectionEstablished(
      std::bind(&HoaBackend::onConnection, this, _1, _2));
  controlConnection_.onConnectionLost(
      std::bind(&HoaBackend::onConnectionLost, this));
  controlConnection_.start(detail::SCENE_MASTER_CONTROL_ENDPOINT);
}

HoaBackend::~HoaBackend() {
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

void HoaBackend::setConnectionId(communication::ConnectionId id) {
  controlConnection_.setConnectionId(id);
}

void HoaBackend::triggerMetadataSend() { metadataSender_.triggerSend(); }

void HoaBackend::setImportedAudioObjectId(uint32_t id)
{
  metadataSender_.importedAudioObjectId(id);
}

void HoaBackend::setImportedAudioTrackUidId(uint32_t id)
{
  metadataSender_.importedAudioTrackUidId(id);
}

void HoaBackend::onConnection(communication::ConnectionId connectionId,
                              const std::string& streamEndpoint) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (connector_) {
    connector_->setStatusBarText("Ready: Connected to Scene");
  }
  metadataSender_.connect(streamEndpoint,
                          communication::ConnectionId{connectionId});
}
void HoaBackend::onConnectionLost() {
  std::lock_guard<std::mutex> lock(mutex_);
  metadataSender_.disconnect();
  if (connector_) {
    connector_->setStatusBarText("Failure: No connection to Scene");
  }
}

void HoaBackend::onParameterChanged(
    ui::HoaFrontendBackendConnector::ParameterId parameter,
    ui::HoaFrontendBackendConnector::ParameterValue value) {
  using ParameterId = ui::HoaFrontendBackendConnector::ParameterId;
  if (parameter == ParameterId::ROUTING) {
    auto extractedValue = boost::get<int>(value);
    EAR_LOGGER_DEBUG(logger_, "Routing -> {}", extractedValue);
    metadataSender_.routing(extractedValue);
  } else if (parameter == ParameterId::NAME) {
    auto extractedValue = boost::get<std::string>(value);
    EAR_LOGGER_DEBUG(logger_, "Name -> {}", extractedValue);
    metadataSender_.name(extractedValue);
  } else if (parameter == ParameterId::COLOUR) {
    auto extractedValue = boost::get<unsigned int>(value);
    EAR_LOGGER_DEBUG(logger_, "Colour -> {}", extractedValue);
    metadataSender_.colour(extractedValue);
  } else if (parameter == ParameterId::PACKFORMAT_ID_FORMAT) {
    auto extractedValue = boost::get<int>(value);
    EAR_LOGGER_DEBUG(logger_, "PackFormat ID Value -> {}", extractedValue);
    metadataSender_.packFormatIdValue(extractedValue);
  }
}

}  // namespace plugin
}  // namespace ear
