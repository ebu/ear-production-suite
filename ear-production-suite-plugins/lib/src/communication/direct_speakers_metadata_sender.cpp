#include "communication/direct_speakers_metadata_sender.hpp"
#include "helper/protobuf_utilities.hpp"

namespace ear::plugin::communication {

DirectSpeakersMetadataSender::DirectSpeakersMetadataSender(
    std::shared_ptr<spdlog::logger> logger)
    : logger_(std::move(logger)),
      data_{[](auto data) {
        data->set_allocated_ds_metadata(
            new proto::DirectSpeakersTypeMetadata{});
      }},
      sender_(data_, logger_) {}

void DirectSpeakersMetadataSender::logger(
    std::shared_ptr<spdlog::logger> logger) {
  sender_.logger(std::move(logger));
}

void DirectSpeakersMetadataSender::connect(const std::string& endpoint,
                                           ConnectionId id) {
  sender_.connect(endpoint, id);
}

void DirectSpeakersMetadataSender::disconnect() { sender_.disconnect(); }

void DirectSpeakersMetadataSender::triggerSend() { sender_.triggerSend(); }

void DirectSpeakersMetadataSender::routing(int32_t value) {
  setData([value](auto data) { data->set_routing(value); });
}
void DirectSpeakersMetadataSender::name(const std::string& value) {
  setData([&value](auto data) { data->set_name(value); });
}
void DirectSpeakersMetadataSender::importedAudioObjectId(int value)
{
  setData([&value](auto data) { data->set_imported_ao_id(value); });
}
void DirectSpeakersMetadataSender::importedAudioTrackUidId(int value)
{
  setData([&value](auto data) { data->set_imported_atu_id(value); });
}
void DirectSpeakersMetadataSender::inputInstanceId(int value)
{
  setData([&value](auto data) { data->set_input_instance_id(value); });
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

}  // namespace ear::plugin::communication
