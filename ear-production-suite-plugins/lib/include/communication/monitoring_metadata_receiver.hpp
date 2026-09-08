#pragma once

#include "log.hpp"
#include "nng-cpp/nng.hpp"
#include <memory>

namespace ear {
namespace plugin {

namespace proto {
class SceneStore;
}

namespace communication {
class MonitoringMetadataReceiver {
 public:
  using RequestHandler = std::function<void(const proto::SceneStore& store)>;
  MonitoringMetadataReceiver(std::shared_ptr<spdlog::logger> logger = nullptr);
  ~MonitoringMetadataReceiver();
  MonitoringMetadataReceiver(const MonitoringMetadataReceiver&) = delete;
  MonitoringMetadataReceiver& operator=(const MonitoringMetadataReceiver&) =
      delete;
  MonitoringMetadataReceiver(MonitoringMetadataReceiver&&) = delete;
  MonitoringMetadataReceiver& operator=(MonitoringMetadataReceiver&&) = delete;

  void logger(std::shared_ptr<spdlog::logger> logger);

  void start(const std::string& endpoint, const RequestHandler& handler);

 private:
  void waitForMetadata();
  void handleReceive(std::error_code ec, nng::Message message);

  std::shared_ptr<spdlog::logger> logger_;
  RequestHandler handler_;
  nng::SubSocket socket_;
};
}  // namespace communication
}  // namespace plugin
}  // namespace ear
