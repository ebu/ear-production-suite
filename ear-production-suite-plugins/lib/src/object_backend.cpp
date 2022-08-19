
#include "object_backend.hpp"

#include "ui/object_frontend_backend_connector.hpp"
#include "detail/constants.hpp"

using std::placeholders::_1;
using std::placeholders::_2;

namespace ear {
namespace plugin {

ObjectBackend::ObjectBackend(ui::ObjectsFrontendBackendConnector* connector)
    : logger_(createLogger(fmt::format("Object Input@{}", (const void*)this))),
    connector_(connector),
    controlConnection_(logger_) {

#ifdef EPS_ENABLE_LOGGING
  logger_->set_level(spdlog::level::trace);
#else
  logger_->set_level(spdlog::level::off);
#endif

  metadataSender_.logger(logger_);

  if (connector_) {
    connector_->setStatusBarText("Failure: No connection to Scene");
    connector_->onParameterChanged(
        std::bind(&ObjectBackend::onParameterChanged, this, _1, _2));
  }
  controlConnection_.onConnectionEstablished(
      std::bind(&ObjectBackend::onConnection, this, _1, _2));
  controlConnection_.onConnectionLost(
      std::bind(&ObjectBackend::onConnectionLost, this));
  controlConnection_.start(detail::SCENE_MASTER_CONTROL_ENDPOINT);
}

ObjectBackend::~ObjectBackend() {
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

void ObjectBackend::setConnectionId(communication::ConnectionId id) {
    EAR_LOGGER_TRACE(logger_, "Restoring connection id {}", id.string());
  controlConnection_.setConnectionId(id);
}

void ObjectBackend::triggerMetadataSend() { metadataSender_.triggerSend(); }

EAR_PLUGIN_BASE_EXPORT void ObjectBackend::setImportedId(uint32_t id)
{
  metadataSender_.importedId(id);
}

void ObjectBackend::onConnection(communication::ConnectionId connectionId,
                                 const std::string& streamEndpoint) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (connector_) {
    connector_->setStatusBarText("Ready: Connected to Scene");
  }
  metadataSender_.connect(streamEndpoint,
                          communication::ConnectionId{connectionId});
}
void ObjectBackend::onConnectionLost() {
  std::lock_guard<std::mutex> lock(mutex_);
  metadataSender_.disconnect();
  if (connector_) {
    connector_->setStatusBarText("Failure: No connection to Scene");
  }
}

void ObjectBackend::onParameterChanged(
    ui::ObjectsFrontendBackendConnector::ParameterId parameter,
    ui::ObjectsFrontendBackendConnector::ParameterValue value) {
  using ParameterId = ui::ObjectsFrontendBackendConnector::ParameterId;
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
  } else if (parameter == ParameterId::GAIN) {
    auto extractedValue = boost::get<float>(value);
    EAR_LOGGER_DEBUG(logger_, "Gain -> {}", extractedValue);
    metadataSender_.gain(extractedValue);
  } else if (parameter == ParameterId::AZIMUTH) {
    auto extractedValue = boost::get<float>(value);
    EAR_LOGGER_DEBUG(logger_, "Azimuth -> {}", extractedValue);
    metadataSender_.azimuth(extractedValue);
  } else if (parameter == ParameterId::ELEVATION) {
    auto extractedValue = boost::get<float>(value);
    EAR_LOGGER_DEBUG(logger_, "Elevation -> {}", extractedValue);
    metadataSender_.elevation(extractedValue);
  } else if (parameter == ParameterId::DISTANCE) {
    auto extractedValue = boost::get<float>(value);
    EAR_LOGGER_DEBUG(logger_, "Distance -> {}", extractedValue);
    metadataSender_.distance(extractedValue);
  } else if (parameter == ParameterId::WIDTH) {
    auto extractedValue = boost::get<float>(value);
    EAR_LOGGER_DEBUG(logger_, "Width -> {}", extractedValue);
    metadataSender_.width(extractedValue);
  } else if (parameter == ParameterId::HEIGHT) {
    auto extractedValue = boost::get<float>(value);
    EAR_LOGGER_DEBUG(logger_, "Height -> {}", extractedValue);
    metadataSender_.height(extractedValue);
  } else if (parameter == ParameterId::DEPTH) {
    auto extractedValue = boost::get<float>(value);
    EAR_LOGGER_DEBUG(logger_, "Depth -> {}", extractedValue);
    metadataSender_.depth(extractedValue);
  } else if (parameter == ParameterId::DIFFUSE) {
    auto extractedValue = boost::get<float>(value);
    EAR_LOGGER_DEBUG(logger_, "Diffuse -> {}", extractedValue);
    metadataSender_.diffuse(extractedValue);
  } else if (parameter == ParameterId::FACTOR) {
    auto extractedValue = boost::get<float>(value);
    EAR_LOGGER_DEBUG(logger_, "Factor -> {}", extractedValue);
    metadataSender_.factor(extractedValue);
  } else if (parameter == ParameterId::RANGE) {
    auto extractedValue = boost::get<float>(value);
    EAR_LOGGER_DEBUG(logger_, "Range -> {}", extractedValue);
    metadataSender_.range(extractedValue);
  }
}

}  // namespace plugin
}  // namespace ear
