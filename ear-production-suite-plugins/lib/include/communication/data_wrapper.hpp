#pragma once
#include <functional>
#include <mutex>
#include "message_buffer.hpp"
#include "input_item_metadata.pb.h"

namespace ear::plugin::communication {

class DataWrapper {
 public:
  explicit DataWrapper(std::function<void(proto::InputItemMetadata*)> init) {
    writeAccess(init);
  }

  template <typename FunctionT>
  void writeAccess(FunctionT&& accessor) {
    std::lock_guard<std::mutex> lock{mutex_};
    data_.set_changed(true);
    std::invoke(accessor, &data_);
  }

  template <typename FunctionT>
  auto readAccess(FunctionT&& accessor) {
    std::lock_guard<std::mutex> lock{mutex_};
    auto const& data{data_};
    return std::invoke(accessor, data);
  }

  MessageBuffer prepareMessage() {
    std::lock_guard<std::mutex> lock{mutex_};
    MessageBuffer buffer = allocBuffer(data_.ByteSizeLong());
    data_.SerializeToArray(buffer.data(), buffer.size());
    data_.set_changed(false);
    return buffer;
  }

 private:
  proto::InputItemMetadata data_;
  std::mutex mutex_;
};
}