#pragma once

#include <string>
#include <chrono>

#include "metadata_sender.hpp"
#include "data_wrapper.hpp"
#include "log.hpp"

namespace ear {
namespace plugin {
namespace communication {

class HoaMetadataSender {
 public:
  HoaMetadataSender(std::shared_ptr<spdlog::logger> logger = nullptr);
  void logger(std::shared_ptr<spdlog::logger> logger);
  void connect(const std::string& endpoint, ConnectionId connectionId);
  void disconnect();
  void triggerSend();

  void routing(int32_t value);
  void name(const std::string& value);
  void inputInstanceId(int value);
  void colour(int value);
  void packFormatIdValue(int value);

  ConnectionId getConnectionId() { return sender_.connectionId(); }

 private:
   template<typename FunctionT>
   void setData(FunctionT&& set) {
     data_.writeAccess([set](auto data) { std::invoke(set, data); });
   }
  std::shared_ptr<spdlog::logger> logger_;
  DataWrapper data_;
  MetadataSender sender_;
};

}  // namespace communication
}  // namespace plugin
}  // namespace ear
