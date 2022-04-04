#include "communication/hoa_metadata_sender.hpp"
#include "helper/protobuf_utilities.hpp"

namespace ear::plugin::communication {

HoaMetadataSender::HoaMetadataSender(std::shared_ptr<spdlog::logger> logger)
    : logger_(std::move(logger)),
      data_{[](auto data) {
        data->set_allocated_hoa_metadata(new proto::HoaTypeMetadata{});
      }},
      sender_(data_, logger_) {}

void HoaMetadataSender::logger(std::shared_ptr<spdlog::logger> logger) {
  sender_.logger(std::move(logger));
}

void HoaMetadataSender::connect(const std::string& endpoint, ConnectionId id) {
  sender_.connect(endpoint, id);
}

void HoaMetadataSender::disconnect() { sender_.disconnect(); }

void HoaMetadataSender::triggerSend() { sender_.triggerSend(); }

void HoaMetadataSender::routing(int32_t value) {
  setData([value](auto data) { data->set_routing(value); });
}
void HoaMetadataSender::name(const std::string& value) {
  setData([&value](auto data) { data->set_name(value); });
}
void HoaMetadataSender::colour(int value) {
  setData([value](auto data) { data->set_colour(value); });
}
void HoaMetadataSender::packFormatIdValue(int value) {
  setData([value](auto data) {
    proto::HoaTypeMetadata* hoa_metadata = new proto::HoaTypeMetadata();
    hoa_metadata->set_packformatidvalue(value);
    data->set_allocated_hoa_metadata(hoa_metadata);
  });
}

}  // namespace ear::plugin::communication
