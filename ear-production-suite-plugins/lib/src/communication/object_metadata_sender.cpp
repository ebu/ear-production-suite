#include "communication/object_metadata_sender.hpp"

using namespace std::chrono_literals;

namespace ear {
namespace plugin {

namespace communication {

ObjectMetadataSender::ObjectMetadataSender(
    std::shared_ptr<spdlog::logger> logger)
    : data_([](auto data) {
        data->set_allocated_obj_metadata(new proto::ObjectsTypeMetadata{});
      }),
      sender_(data_, std::move(logger)) {
}

void ObjectMetadataSender::logger(std::shared_ptr<spdlog::logger> logger) {
  sender_.logger(std::move(logger));
}

void ObjectMetadataSender::connect(const std::string& endpoint,
                                   ConnectionId id) {
  sender_.connect(endpoint, std::move(id));
}

void ObjectMetadataSender::disconnect() {
  sender_.disconnect();
}

void ObjectMetadataSender::triggerSend() { sender_.triggerSend(); }

void ObjectMetadataSender::name(const std::string& value) {
  setData([value](auto data) { data->set_name(value); });
}
void ObjectMetadataSender::inputInstanceId(int value)
{
  setData([&value](auto data) { data->set_input_instance_id(value); });
}
void ObjectMetadataSender::colour(int value) {
  setData([value](auto data) { data->set_colour(value); });
}
void ObjectMetadataSender::routing(int32_t value) {
  setData([value](auto data) { data->set_routing(value); });
}
void ObjectMetadataSender::gain(float value) {
  setObjectData([value](auto data) { data->set_gain(value); });
}
void ObjectMetadataSender::azimuth(float value) {
  setObjectData(
      [value](auto data) { data->mutable_position()->set_azimuth(value); });
}
void ObjectMetadataSender::elevation(float value) {
  setObjectData(
      [value](auto data) { data->mutable_position()->set_elevation(value); });
}
void ObjectMetadataSender::distance(float value) {
  setObjectData(
      [value](auto data) { data->mutable_position()->set_distance(value); });
}
void ObjectMetadataSender::width(float value) {
  setObjectData([value](auto data) { data->set_width(value); });
}
void ObjectMetadataSender::height(float value) {
  setObjectData([value](auto data) { data->set_height(value); });
}
void ObjectMetadataSender::depth(float value) {
  setObjectData([value](auto data) { data->set_depth(value); });
}
void ObjectMetadataSender::diffuse(float value) {
  setObjectData([value](auto data) { data->set_diffuse(value); });
}
void ObjectMetadataSender::factor(float value) {
  setObjectData([value](auto data) { data->set_factor(value); });
}
void ObjectMetadataSender::range(float value) {
  setObjectData([value](auto data) { data->set_range(value); });
}

}  // namespace communication
}  // namespace plugin
}  // namespace ear
