#pragma once

#include "nng-cpp/nng.hpp"
#include "log.hpp"
#include "communication/common_types.hpp"
#include "input_item_metadata.pb.h"
#include <boost/variant.hpp>
#include <functional>

namespace ear {
namespace plugin {
namespace communication {

class SceneMetadataReceiver {
 public:
  /// Signature/handle to a function/callable that will handle requests
  using RequestHandler = std::function<void(communication::ConnectionId,
                                            proto::InputItemMetadata)>;
  /**
   * @param logger logger instance for logging, can be a nullptr to disable
   * logging.
   */
  SceneMetadataReceiver() = default;
  SceneMetadataReceiver(std::shared_ptr<spdlog::logger> logger);
  ~SceneMetadataReceiver();
  SceneMetadataReceiver(const SceneMetadataReceiver&) = delete;
  SceneMetadataReceiver& operator=(const SceneMetadataReceiver&) = delete;
  SceneMetadataReceiver(SceneMetadataReceiver&&) = delete;
  SceneMetadataReceiver& operator=(SceneMetadataReceiver&&) = delete;

  void setLogger(std::shared_ptr<spdlog::logger> logger) { logger_ = logger; }

  /**
   * @brief Start listening and handling incoming connections
   *
   * The command receiver will continue to handle requests until destructed.
   *
   * @param endpoint A `nng` URL to listen for connections
   * @param handler The request handler that will be called for every incoming
   * request.
   */
  void run(const std::string& endpoint, const RequestHandler& handler);
  void checkEndpoint(const std::string& endpoint);

 private:
  void waitForMetadata();
  void handleReceive(std::error_code ec, nng::Message message);
  RequestHandler handler_;
  nng::PullSocket socket_;
  std::shared_ptr<spdlog::logger> logger_;
};

}  // namespace communication
}  // namespace plugin
}  // namespace ear
