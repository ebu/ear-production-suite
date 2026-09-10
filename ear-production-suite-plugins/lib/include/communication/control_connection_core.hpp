#pragma once

#include "communication/commands.hpp"
#include "log.hpp"
#include "nng-cpp/nng.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <system_error>

namespace ear {
namespace plugin {
namespace communication {

class EAR_PLUGIN_BASE_EXPORT ControlConnectionCore {
 public:
  using ConnectionEstablishedHandler =
      std::function<void(ConnectionId, std::string)>;
  using ConnectionLostHandler = std::function<void()>;
  using AsyncResponseHandler =
      std::function<void(std::error_code, nng::Message)>;
  using AsyncRequest =
      std::function<bool(const MessageBuffer&, AsyncResponseHandler)>;
  using CallbackDispatcher = std::function<bool(std::function<void()>)>;
  using StartHandler = std::function<void(const std::string&)>;
  using StopHandler = std::function<void()>;
  using DetailsRequestFactory = std::function<MessageBuffer(ConnectionId)>;
  using DetailsResponseParser = std::function<std::string(const Response&)>;
  using CloseConnectionHandler = std::function<bool(ConnectionId)>;

  struct Operations {
    StartHandler start;
    StopHandler stop;
    StopHandler cancel;
    AsyncRequest request;
    // Connection callbacks must run outside the transport AIO callback.
    CallbackDispatcher dispatch;
  };

  struct LoggingOptions {
    bool warnOnDisconnect{false};
    bool warnWhenNoConnectionEstablishedCallback{false};
    bool warnWhenNoConnectionLostCallback{false};
  };

  ControlConnectionCore(Operations operations, ConnectionType connectionType,
                        std::string detailsType,
                        DetailsRequestFactory detailsRequestFactory,
                        DetailsResponseParser detailsResponseParser,
                        std::shared_ptr<spdlog::logger> logger,
                        LoggingOptions loggingOptions);
  ~ControlConnectionCore();

  ControlConnectionCore(const ControlConnectionCore&) = delete;
  ControlConnectionCore& operator=(const ControlConnectionCore&) = delete;
  ControlConnectionCore(ControlConnectionCore&&) = delete;
  ControlConnectionCore& operator=(ControlConnectionCore&&) = delete;

  void start(const std::string& endpoint);
  void stop();

  void connected();
  void disconnected();

  void setConnectionId(ConnectionId id, CloseConnectionHandler closeConnection);
  ConnectionId getConnectionId() const;
  bool isConnected() const;

  void logger(std::shared_ptr<spdlog::logger> logger);
  std::shared_ptr<spdlog::logger> logger() const;

  void onConnectionEstablished(ConnectionEstablishedHandler callback);
  void onConnectionLost(ConnectionLostHandler callback);

 private:
  void handshake(std::uint64_t generation);
  void handleNewConnectionResponse(std::error_code ec, nng::Message message,
                                   std::uint64_t generation);
  void handleDetailsResponse(std::error_code ec, nng::Message message,
                             std::uint64_t generation);
  bool retryHandshake(std::uint64_t completedGeneration);
  void notifyConnectionLost(ConnectionId connectionId);
  void dispatchCallback(std::function<void()> callback);
  std::shared_ptr<spdlog::logger> loggerSnapshot() const;

  Operations operations_;
  ConnectionType connectionType_;
  std::string detailsType_;
  DetailsRequestFactory detailsRequestFactory_;
  DetailsResponseParser detailsResponseParser_;
  LoggingOptions loggingOptions_;

  mutable std::mutex stateMutex_;
  std::mutex negotiationMutex_;
  std::shared_ptr<spdlog::logger> logger_;
  ConnectionId connectionId_;
  bool connected_{false};
  bool pipeConnected_{false};
  bool reconfiguring_{false};
  bool stopped_{false};
  std::uint64_t negotiationGeneration_{0};
  ConnectionEstablishedHandler connectedCallback_;
  ConnectionLostHandler disconnectedCallback_;
};

}  // namespace communication
}  // namespace plugin
}  // namespace ear
