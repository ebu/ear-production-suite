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
  using RequestHandler = std::function<void(proto::SceneStore)>;
  MonitoringMetadataReceiver(std::shared_ptr<spdlog::logger> logger = nullptr);
  ~MonitoringMetadataReceiver();
  MonitoringMetadataReceiver(const MonitoringMetadataReceiver&) = delete;
  MonitoringMetadataReceiver& operator=(const MonitoringMetadataReceiver&) =
      delete;
  MonitoringMetadataReceiver(MonitoringMetadataReceiver&&) = delete;
  MonitoringMetadataReceiver& operator=(MonitoringMetadataReceiver&&) = delete;

  void logger(std::shared_ptr<spdlog::logger> logger);

  void start(const std::string& endpoint, const RequestHandler& handler);

  /**
   * Stop receiving metadata and shutdown the receiver.
   *
   * This will effectively cancel pending async receiving operations,
   * which will in turn cause `handleReceive` to shutdown the stream receiver.
   *
   * Once the stream receiver has been shut down, it cannot be restarted.
   *
   * @note
   * This is not the best design choice, but it's the easiest way to work
   * with the fact that we cannot re-open an nng socket after closing.
   * Another option would be to replace the socket during start or
   * don't close the socket at all, but implent access to the nng dialer
   * and stop this to cancel automatic reconnection.
   */
  void shutdown();

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
