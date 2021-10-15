#pragma once

#include "common_types.hpp"
#include "data_wrapper.hpp"
#include "metadata_sender.hpp"
#include "message_buffer.hpp"
#include "log.hpp"
#include "input_item_metadata.pb.h"
#include <string>

namespace ear {
namespace plugin {
namespace communication {

class ObjectMetadataSender {
 public:
  ObjectMetadataSender(std::shared_ptr<spdlog::logger> logger = nullptr);

  void logger(std::shared_ptr<spdlog::logger> logger);
  void connect(const std::string& endpoint, ConnectionId connectionId);
  void disconnect();
  void triggerSend();

  void routing(int32_t value);
  void name(const std::string& value);
  void colour(int value);

  void gain(float);
  void azimuth(float);
  void elevation(float);
  void distance(float);

  void width(float);
  void height(float);
  void depth(float);
  void diffuse(float);
  void factor(float);
  void range(float);

  ConnectionId getConnectionId() { return sender_.connectionId(); }

 private:
  template<typename FunctionT>
  void setData(FunctionT&& set) {
    data_.writeAccess([set](auto data) {
      std::invoke(set, data);
    });
  }
  template<typename FunctionT>
  void setObjectData(FunctionT&& set) {
    data_.writeAccess([set](auto data) {
      std::invoke(set, data->mutable_obj_metadata());
    });
  }
  DataWrapper data_;
  MetadataSender sender_;
};

}  // namespace communication
}  // namespace plugin
}  // namespace ear
