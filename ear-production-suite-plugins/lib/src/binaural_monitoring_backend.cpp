#include "binaural_monitoring_backend.hpp"
#include "communication/monitoring_metadata_receiver.hpp"
#include "detail/constants.hpp"
#include "helper/eps_to_ear_metadata_converter.hpp"
#include <functional>
#include <algorithm>

using std::placeholders::_1;
using std::placeholders::_2;

namespace {
  template<typename T>
  bool vectorContains(std::vector<T> const& v, T k) {
    return std::find(v.begin(), v.end(), k) != v.end();
  }

  bool isValidId(std::string const& id) {
    return !(id.empty() || id == "00000000-0000-0000-0000-000000000000");
  }
}

namespace ear {
namespace plugin {

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

  activeDirectSpeakersIds.reserve(inputChannelCount);
  activeObjectIds.reserve(inputChannelCount);
  activeHoaIds.reserve(inputChannelCount);

  commonDefinitionHelper
      .getElementRelationships();  // Save doing it later in time-critical calls

  allActiveIds.reserve(inputChannelCount);

  controlConnection_.logger(logger_);
  controlConnection_.onConnectionEstablished(
      std::bind(&BinauralMonitoringBackend::onConnection, this, _1, _2));
  controlConnection_.onConnectionLost(
      std::bind(&BinauralMonitoringBackend::onConnectionLost, this));
  controlConnection_.start(detail::SCENE_MASTER_CONTROL_ENDPOINT);

  listenerOrientation = std::make_shared<ListenerOrientation>();
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

std::vector<ConnId> BinauralMonitoringBackend::getActiveDirectSpeakersIds() {
  std::lock_guard<std::mutex> lock(activeDirectSpeakersIdsMutex_);
  return activeDirectSpeakersIds;
}

size_t BinauralMonitoringBackend::getTotalDirectSpeakersChannels() {
  std::lock_guard<std::mutex> lock(activeDirectSpeakersIdsMutex_);
  return directSpeakersChannelCount;
}

std::vector<ConnId> BinauralMonitoringBackend::getActiveHoaIds() {
  std::lock_guard<std::mutex> lock(activeHoaIdsMutex_);
  return activeHoaIds;
}

size_t BinauralMonitoringBackend::getTotalHoaChannels() {
  std::lock_guard<std::mutex> lock(activeHoaIdsMutex_);
  return hoaChannelCount;
}

std::vector<ConnId> BinauralMonitoringBackend::getActiveObjectIds() {
  std::lock_guard<std::mutex> lock(activeObjectIdsMutex_);
  return activeObjectIds;
}

size_t BinauralMonitoringBackend::getTotalObjectChannels() {
  std::lock_guard<std::mutex> lock(activeObjectIdsMutex_);
  return objectChannelCount;
}

std::optional<BinauralMonitoringBackend::DirectSpeakersEarMetadataAndRouting>
BinauralMonitoringBackend::getLatestDirectSpeakersTypeMetadata(ConnId id) {
  std::lock_guard<std::mutex> lockA(latestDirectSpeakersTypeMetadataMutex_);

  auto earMD =
      getValuePointerFromMap<ConnId, DirectSpeakersEarMetadataAndRouting>(
          latestDirectSpeakersTypeMetadata, id);
  if (earMD) {
    return std::optional<DirectSpeakersEarMetadataAndRouting>(*earMD);
  }

  std::lock_guard<std::mutex> lockB(latestMonitoringItemMetadataMutex_);

  auto epsMD =
      getValuePointerFromMap<ConnId,
                             ear::plugin::proto::MonitoringItemMetadata>(
          latestMonitoringItemMetadata, id);
  if (!epsMD || !epsMD->has_ds_metadata()) {
    return std::optional<DirectSpeakersEarMetadataAndRouting>();
  }

  setInMap<ConnId, DirectSpeakersEarMetadataAndRouting>(
      latestDirectSpeakersTypeMetadata, id,
      DirectSpeakersEarMetadataAndRouting{
          epsMD->has_routing() ? epsMD->routing() : -1,
          EpsToEarMetadataConverter::convert(epsMD->ds_metadata())});

  earMD = getValuePointerFromMap<ConnId, DirectSpeakersEarMetadataAndRouting>(
      latestDirectSpeakersTypeMetadata, id);
  if (earMD) {
    return std::optional<DirectSpeakersEarMetadataAndRouting>(*earMD);
  }

  return std::optional<DirectSpeakersEarMetadataAndRouting>();
}

std::optional<BinauralMonitoringBackend::HoaEarMetadataAndRouting>

BinauralMonitoringBackend::getLatestHoaTypeMetadata(ConnId id) {
  std::lock_guard<std::mutex> lockA(latestHoaTypeMetadataMutex_);
  auto earMD = getValuePointerFromMap<ConnId, HoaEarMetadataAndRouting>(
      latestHoaTypeMetadata, id);
  if (earMD) {
    return std::optional<HoaEarMetadataAndRouting>(*earMD);
  }

  std::lock_guard<std::mutex> lockB(latestMonitoringItemMetadataMutex_);

  auto epsMD =
      getValuePointerFromMap<ConnId,
                             ear::plugin::proto::MonitoringItemMetadata>(
          latestMonitoringItemMetadata, id);
  if (!epsMD || !epsMD->has_hoa_metadata()) {
    return std::optional<HoaEarMetadataAndRouting>();
  }
  {
    std::lock_guard<std::mutex> lock(commonDefinitionHelperMutex_);
    setInMap<ConnId, HoaEarMetadataAndRouting>(
        latestHoaTypeMetadata, id,
        HoaEarMetadataAndRouting{
            epsMD->has_routing() ? epsMD->routing() : -1,
            EpsToEarMetadataConverter::convert(epsMD->hoa_metadata(),
                                               commonDefinitionHelper)});
  }

  earMD = getValuePointerFromMap<ConnId, HoaEarMetadataAndRouting>(
      latestHoaTypeMetadata, id);
  if (earMD) {
    return std::optional<HoaEarMetadataAndRouting>(*earMD);
  }
  return std::optional<HoaEarMetadataAndRouting>();
}

std::optional<BinauralMonitoringBackend::ObjectsEarMetadataAndRouting>
BinauralMonitoringBackend::getLatestObjectsTypeMetadata(ConnId id) {
  std::lock_guard<std::mutex> lockA(latestObjectsTypeMetadataMutex_);

  auto earMD = getValuePointerFromMap<ConnId, ObjectsEarMetadataAndRouting>(
      latestObjectsTypeMetadata, id);
  if (earMD) {
    return std::optional<ObjectsEarMetadataAndRouting>(*earMD);
  }

  std::lock_guard<std::mutex> lockB(latestMonitoringItemMetadataMutex_);

  auto epsMD =
      getValuePointerFromMap<ConnId,
                             ear::plugin::proto::MonitoringItemMetadata>(
          latestMonitoringItemMetadata, id);
  if (!epsMD || !epsMD->has_obj_metadata()) {
    return std::optional<ObjectsEarMetadataAndRouting>();
  }

  setInMap<ConnId, ObjectsEarMetadataAndRouting>(
      latestObjectsTypeMetadata, id,
      ObjectsEarMetadataAndRouting{
          epsMD->has_routing() ? epsMD->routing() : -1,
          EpsToEarMetadataConverter::convert(epsMD->obj_metadata())});

  earMD = getValuePointerFromMap<ConnId, ObjectsEarMetadataAndRouting>(
      latestObjectsTypeMetadata, id);
  if (earMD) {
    return std::optional<ObjectsEarMetadataAndRouting>(*earMD);
  }

  return std::optional<ObjectsEarMetadataAndRouting>();
}

void BinauralMonitoringBackend::onSceneReceived(proto::SceneStore store) {
  isExporting_ = store.has_is_exporting() && store.is_exporting();

  size_t totalDsChannels = 0;
  size_t totalObjChannels = 0;
  size_t totalHoaChannels = 0;

  std::vector<ConnId> availableItemIds;
  availableItemIds.reserve(store.all_available_items_size());

  for (const auto& item : store.all_available_items()) {
    if(item.has_connection_id() &&
       isValidId(item.connection_id())) {
      availableItemIds.push_back(item.connection_id());
    }
    if (item.has_ds_metadata()) {
      totalDsChannels += item.ds_metadata().speakers_size();
    }
    if (item.has_obj_metadata()) {
      totalObjChannels++;
    }
    if (item.has_hoa_metadata()) {
      auto hoaId = item.hoa_metadata().packformatidvalue();
      {
        std::lock_guard<std::mutex> lock(commonDefinitionHelperMutex_);
        // TODO: May need to revisit later -
        //       this is a high-frequency call but may be slow to to execute
        auto pfData = commonDefinitionHelper.getPackFormatData(4, hoaId);
        if (pfData) {
          auto cfCount = pfData->relatedChannelFormats.size();
          totalHoaChannels += cfCount;
        }
      }
    }
  }

  std::lock_guard<std::mutex> lockDsIds(activeDirectSpeakersIdsMutex_);
  std::lock_guard<std::mutex> lockObjIds(activeObjectIdsMutex_);
  std::lock_guard<std::mutex> lockHoaIds(activeHoaIdsMutex_);
  activeDirectSpeakersIds.clear();
  activeObjectIds.clear();
  activeHoaIds.clear();

  for (const auto& item : store.monitoring_items()) {
    if (item.has_connection_id() &&
        isValidId(item.connection_id()) &&
        vectorContains(availableItemIds, item.connection_id())) {

      bool newItem = !vectorContains(allActiveIds, item.connection_id());

      // clang-format off
      if (item.has_hoa_metadata()) {
        if(newItem || item.changed()) {
          {
            std::lock_guard<std::mutex> lock(latestHoaTypeMetadataMutex_);
            removeFromMap<ConnId, HoaEarMetadataAndRouting>(
                latestHoaTypeMetadata, item.connection_id());
          }
          {
            std::lock_guard<std::mutex> lock(
                latestMonitoringItemMetadataMutex_);
            setInMap<ConnId, ear::plugin::proto::MonitoringItemMetadata>(
                latestMonitoringItemMetadata, item.connection_id(), item);
          }
        }
        activeHoaIds.push_back(item.connection_id());
      }

      if (item.has_ds_metadata()) {
        if (newItem || item.changed()) {
          {
            std::lock_guard<std::mutex> lock(
                latestDirectSpeakersTypeMetadataMutex_);
            removeFromMap<ConnId, DirectSpeakersEarMetadataAndRouting>(
                latestDirectSpeakersTypeMetadata, item.connection_id());
          }
          {
            std::lock_guard<std::mutex> lock(
                latestMonitoringItemMetadataMutex_);
            setInMap<ConnId, ear::plugin::proto::MonitoringItemMetadata>(
                latestMonitoringItemMetadata, item.connection_id(), item);
          }
        }
        activeDirectSpeakersIds.push_back(item.connection_id());
      }

      if (item.has_obj_metadata()) {
        if (newItem || item.changed()) {
          {
            std::lock_guard<std::mutex> lock(latestObjectsTypeMetadataMutex_);
            removeFromMap<ConnId, ObjectsEarMetadataAndRouting>(
                latestObjectsTypeMetadata, item.connection_id());
          }
          {
            std::lock_guard<std::mutex> lock(
                latestMonitoringItemMetadataMutex_);
            setInMap<ConnId, ear::plugin::proto::MonitoringItemMetadata>(
                latestMonitoringItemMetadata, item.connection_id(), item);
          }
        }
        activeObjectIds.push_back(item.connection_id());
      }
    }
  }

  allActiveIds.clear();
  for(const auto& item : store.monitoring_items()) {
    allActiveIds.push_back(item.connection_id());
  }

  objectChannelCount = totalObjChannels;
  directSpeakersChannelCount = totalDsChannels;
  hoaChannelCount = totalHoaChannels;
}

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
  {
    std::lock_guard<std::mutex> lock(activeDirectSpeakersIdsMutex_);
    activeDirectSpeakersIds.clear();
  }
  {
    std::lock_guard<std::mutex> lock(activeObjectIdsMutex_);
    activeObjectIds.clear();
  }
}

}  // namespace plugin
}  // namespace ear
