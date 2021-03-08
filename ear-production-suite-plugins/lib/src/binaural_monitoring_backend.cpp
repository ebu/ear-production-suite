#include "binaural_monitoring_backend.hpp"
#include "communication/monitoring_metadata_receiver.hpp"
#include "detail/constants.hpp"
#include "helper/eps_to_ear_metadata_converter.hpp"
#include <functional>

using std::placeholders::_1;
using std::placeholders::_2;

namespace ear {
namespace plugin {
/*
BinauralMonitoringBackend::BinauralMonitoringBackend(
  ui::MonitoringFrontendBackendConnector* connector,
  const Layout& targetLayout, int inputChannelCount)
  :  gainsCalculator_(targetLayout, inputChannelCount),
    frontendConnector_(connector),
    controlConnection_() {
    */
BinauralMonitoringBackend::BinauralMonitoringBackend(
    ui::MonitoringFrontendBackendConnector* connector, int inputChannelCount)
    : frontendConnector_(connector), controlConnection_() {
  logger_ =
      createLogger(fmt::format("BinauralMonitoring@{}", (const void*)this));

#ifndef NDEBUG
  logger_->set_level(spdlog::level::trace);
#else
  logger_->set_level(spdlog::level::off);
#endif

  //channelAllocations.resize(inputChannelCount, blankRoutingInformation);
  activeDirectSpeakersIds.reserve(inputChannelCount);
  activeObjectIds.reserve(inputChannelCount);

  // gains_.direct = gainsCalculator_.directGains();
  // gains_.diffuse = gainsCalculator_.diffuseGains();
  controlConnection_.logger(logger_);
  controlConnection_.onConnectionEstablished(
      std::bind(&BinauralMonitoringBackend::onConnection, this, _1, _2));
  controlConnection_.onConnectionLost(
      std::bind(&BinauralMonitoringBackend::onConnectionLost, this));
  controlConnection_.start(detail::SCENE_MASTER_CONTROL_ENDPOINT);
}

BinauralMonitoringBackend::~BinauralMonitoringBackend() {
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

std::vector<std::string>* BinauralMonitoringBackend::getActiveDirectSpeakersIds()
{
  // TODO: Needs locks
  return &activeDirectSpeakersIds;
}

std::vector<std::string>* BinauralMonitoringBackend::getActiveObjectIds()
{
  // TODO: Needs locks
  return &activeObjectIds;
}

BinauralMonitoringBackend::DirectSpeakersEarMetadataAndRouting* BinauralMonitoringBackend::getLatestDirectSpeakersTypeMetadata(ConnId id)
{
  // TODO: Needs locks (need to do copies?)

  auto earMD = getValuePointerFromMap<ConnId, DirectSpeakersEarMetadataAndRouting>
    (latestDirectSpeakersTypeMetadata, id);
  if(earMD) return earMD;

  auto epsMD = getValuePointerFromMap<ConnId, ear::plugin::proto::MonitoringItemMetadata>(
      latestMonitoringItemMetadata, id);
  if(!epsMD || !epsMD->has_ds_metadata()) return nullptr;

  setInMap<ConnId, DirectSpeakersEarMetadataAndRouting>(latestDirectSpeakersTypeMetadata, id,
                                                        DirectSpeakersEarMetadataAndRouting{
                                                          epsMD->has_routing() ? epsMD->routing() : -1,
                                                          EpsToEarMetadataConverter::convert(epsMD->ds_metadata())
                                                        });

  return getValuePointerFromMap<ConnId, DirectSpeakersEarMetadataAndRouting>
    (latestDirectSpeakersTypeMetadata, id);
}

BinauralMonitoringBackend::ObjectsEarMetadataAndRouting* BinauralMonitoringBackend::getLatestObjectsTypeMetadata(ConnId id)
{
  // TODO: Needs locks (need to do copies?)

  auto earMD = getValuePointerFromMap<ConnId, ObjectsEarMetadataAndRouting>
    (latestObjectsTypeMetadata, id);
  if(earMD) return earMD;

  auto epsMD = getValuePointerFromMap<ConnId, ear::plugin::proto::MonitoringItemMetadata>(
    latestMonitoringItemMetadata, id);
  if(!epsMD || !epsMD->has_obj_metadata()) return nullptr;

  setInMap<ConnId, ObjectsEarMetadataAndRouting>(latestObjectsTypeMetadata, id,
                                                 ObjectsEarMetadataAndRouting{
                                                   epsMD->has_routing() ? epsMD->routing() : -1,
                                                   EpsToEarMetadataConverter::convert(epsMD->obj_metadata())
                                                 });

  return getValuePointerFromMap<ConnId, ObjectsEarMetadataAndRouting>
    (latestObjectsTypeMetadata, id);
}

void BinauralMonitoringBackend::onSceneReceived(proto::SceneStore store) {
  // updateActiveGains(std::move(store));

  // TODO: Needs locks
  //std::fill(channelAllocations.begin(), channelAllocations.end(), blankRoutingInformation);
  activeDirectSpeakersIds.clear();
  activeObjectIds.clear();

  for(const auto& item : store.items()) {
    if(item.connection_id() != "00000000-0000-0000-0000-000000000000") {
      if(item.has_ds_metadata()) {
        /*
        if(item.routing() >= 0) {
          for(int ch = 0; ch < item.ds_metadata().speakers_size(); ch++) {
            channelAllocations[item.routing() + ch].id = item.connection_id();
            channelAllocations[item.routing() + ch].vectorIndex = ch;
          }
        }
        */
        activeDirectSpeakersIds.push_back(item.connection_id());
        if(item.changed()) {
          removeFromMap<ConnId,
            DirectSpeakersEarMetadataAndRouting>(
              latestDirectSpeakersTypeMetadata, item.connection_id());
          setInMap<ConnId, ear::plugin::proto::MonitoringItemMetadata>(
            latestMonitoringItemMetadata, item.connection_id(), item);
        }
      }
      if(item.has_obj_metadata()) {
        /*
        if(item.routing() >= 0) {
          channelAllocations[item.routing()].id = item.connection_id();
          channelAllocations[item.routing()].vectorIndex = 0;
        }
        */
        activeObjectIds.push_back(item.connection_id());
        if(item.changed()) {
          removeFromMap<ConnId, ObjectsEarMetadataAndRouting>(
            latestObjectsTypeMetadata, item.connection_id());
          setInMap<ConnId, ear::plugin::proto::MonitoringItemMetadata>(
            latestMonitoringItemMetadata, item.connection_id(), item);
        }
      }
    }
  }
}

/*
GainHolder BinauralMonitoringBackend::currentGains() {
  std::lock_guard<std::mutex> lock(gainsMutex_);
  return gains_;
}

void BinauralMonitoringBackend::updateActiveGains(proto::SceneStore store) {
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
*/

void BinauralMonitoringBackend::onConnection(
    communication::ConnectionId id, const std::string& streamEndpoint) {
  try {
    logger_->info(
        "Connected to Scene ({}), will now listening for metadata from "
        "{}",
        id.string(), streamEndpoint);
    metadataReceiver_ =
        std::make_unique<communication::MonitoringMetadataReceiver>(logger_);
    metadataReceiver_->start(
        streamEndpoint,
        std::bind(&BinauralMonitoringBackend::onSceneReceived, this, _1));
  } catch (const std::runtime_error& e) {
    logger_->error("Failed to start stream receiver: {}", e.what());
  }
}

void BinauralMonitoringBackend::onConnectionLost() {
  logger_->info("Lost connection to Scene");
  metadataReceiver_->shutdown();
  // force update with an "empty" store to generate silence
  // updateActiveGains(proto::SceneStore{});
  activeDirectSpeakersIds.clear();
  activeObjectIds.clear();
}

}  // namespace plugin
}  // namespace ear
