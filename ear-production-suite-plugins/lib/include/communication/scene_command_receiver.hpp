#pragma once
#include "nng-cpp/nng.hpp"
#include "log.hpp"
#include <functional>

namespace ear {
namespace plugin {
namespace communication {

enum class ErrorCode;
class Response;
class Request;

/**
 * @brief Control connection endpoint and command receiver
 *
 * The command receiver listens for incoming connections from other plugins
 * and handles the control line communication in a client/server
 * (request/response). Control line, in this context, refers to nearly any
 * communication that does not involve passing the actual metadata.
 *
 * The command receiver itself does only handle transmission of bytes on the
 * wire and serializing/parsing the outgoing/incoming data.
 *
 * Users of this class register a `RequestHandler` that will be called
 * for every received and parsed `Request`. The request handler is expected
 * to return an appropriate `Response` object that will be send as a reply.
 *
 * @sa ConnectionManager
 */
class SceneCommandReceiver {
 public:
  /// Signature/handle to a function/callable that will handle requests
  using RequestHandler = std::function<Response(const Request&)>;

  /**
   * @param logger logger instance for logging, can be a nullptr to disable
   * logging.
   */
  SceneCommandReceiver() = default;
  SceneCommandReceiver(std::shared_ptr<spdlog::logger> logger);
  SceneCommandReceiver(const SceneCommandReceiver&) = delete;
  SceneCommandReceiver& operator=(const SceneCommandReceiver&) = delete;
  SceneCommandReceiver(SceneCommandReceiver&&) = delete;
  SceneCommandReceiver& operator=(SceneCommandReceiver&&) = delete;
  ~SceneCommandReceiver() {socket_.asyncStop();}

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
  void waitForCommand();
  void handleReceive(std::error_code ec, const nng::Message& message);
  void sendResponse(const Response& response);
  void sendResponse(ErrorCode ec, const std::string& description);
  std::shared_ptr<spdlog::logger> logger_;
  RequestHandler handler_;
  nng::RepSocket socket_;
};
}  // namespace communication
}  // namespace plugin
}  // namespace ear
