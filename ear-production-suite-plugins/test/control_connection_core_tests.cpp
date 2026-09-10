#include <catch2/catch_all.hpp>

#include "communication/commands.hpp"
#include "communication/control_connection_core.hpp"

#include <functional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace {

using ear::plugin::communication::ConnectionId;
using ear::plugin::communication::ConnectionType;
using ear::plugin::communication::ControlConnectionCore;
using ear::plugin::communication::ErrorCode;
using ear::plugin::communication::MonitoringConnectionDetailsMessage;
using ear::plugin::communication::MonitoringConnectionDetailsResponse;
using ear::plugin::communication::NewConnectionMessage;
using ear::plugin::communication::NewConnectionResponse;
using ear::plugin::communication::RequestVariant;
using ear::plugin::communication::Response;
using ear::plugin::communication::ResponsePayloadVariant;

struct EventListener {
  void signal() { signaled = true; }

  bool signaled{false};
};

class FakeTransport {
 public:
  using Handler = ControlConnectionCore::AsyncResponseHandler;

  bool request(const ear::plugin::communication::MessageBuffer& buffer,
               Handler handler) {
    if (activeHandler) {
      return false;
    }
    requests.push_back(ear::plugin::communication::parseRequest(buffer));
    activeHandler = std::move(handler);
    return true;
  }

  void cancel() {
    if (activeHandler) {
      cancelledHandlers.push_back(std::move(activeHandler));
      activeHandler = {};
    }
  }

  void respond(Response response) {
    REQUIRE(activeHandler);
    auto handler = std::move(activeHandler);
    activeHandler = {};
    auto buffer = ear::plugin::communication::serialize(response);
    handler({}, nng::createMessageFrom(buffer));
  }

  void failCancelled() {
    REQUIRE_FALSE(cancelledHandlers.empty());
    auto handler = std::move(cancelledHandlers.front());
    cancelledHandlers.erase(cancelledHandlers.begin());
    handler(std::make_error_code(std::errc::operation_canceled),
            nng::Message{});
  }

  std::vector<RequestVariant> requests;
  std::vector<Handler> cancelledHandlers;
  Handler activeHandler;
  unsigned int stopCount{0};
};

ControlConnectionCore::Operations makeOperations(FakeTransport& transport,
                                                 std::string& endpoint) {
  return ControlConnectionCore::Operations{
      [&endpoint](const std::string& value) { endpoint = value; },
      [&transport] {
        ++transport.stopCount;
        transport.cancel();
      },
      [&transport] { transport.cancel(); },
      [&transport](const ear::plugin::communication::MessageBuffer& buffer,
                   ControlConnectionCore::AsyncResponseHandler handler) {
        return transport.request(buffer, std::move(handler));
      }};
}

ControlConnectionCore makeCore(FakeTransport& transport,
                               std::string& startedEndpoint) {
  return ControlConnectionCore{
      makeOperations(transport, startedEndpoint),
      ConnectionType::MONITORING,
      "monitoring",
      [](ConnectionId id) {
        return ear::plugin::communication::serialize(
            MonitoringConnectionDetailsMessage{id});
      },
      [](const Response& response) {
        return response.payloadAs<MonitoringConnectionDetailsResponse>()
            .metadataEndpoint();
      },
      nullptr,
      ControlConnectionCore::LoggingOptions{}};
}

TEST_CASE("control connection core completes a negotiation") {
  FakeTransport transport;
  std::string startedEndpoint;
  auto core = makeCore(transport, startedEndpoint);
  const auto assignedId = ConnectionId::generate();
  ConnectionId establishedId;
  std::string metadataEndpoint;
  bool callbackSawConnectedState = false;

  core.onConnectionEstablished(
      [&core, &establishedId, &metadataEndpoint, &callbackSawConnectedState](
          ConnectionId id, std::string endpoint) {
        establishedId = id;
        metadataEndpoint = std::move(endpoint);
        callbackSawConnectedState = core.isConnected();
      });
  core.start("inproc://core-test");
  REQUIRE(startedEndpoint == "inproc://core-test");

  core.connected();
  REQUIRE(transport.requests.size() == 1);
  const auto& newConnectionRequest =
      boost::get<NewConnectionMessage>(transport.requests.front());
  REQUIRE(newConnectionRequest.type() == ConnectionType::MONITORING);

  transport.respond(
      Response{ResponsePayloadVariant{NewConnectionResponse{assignedId}}});
  REQUIRE(transport.requests.size() == 2);
  REQUIRE(
      boost::get<MonitoringConnectionDetailsMessage>(transport.requests.back())
          .connectionId() == assignedId);

  transport.respond(Response{ResponsePayloadVariant{
      MonitoringConnectionDetailsResponse{assignedId, "inproc://metadata"}}});

  REQUIRE(core.isConnected());
  REQUIRE(callbackSawConnectedState);
  REQUIRE(establishedId == assignedId);
  REQUIRE(metadataEndpoint == "inproc://metadata");
  REQUIRE(core.getConnectionId() == assignedId);
}

TEST_CASE("control connection core retries the current generation") {
  FakeTransport transport;
  std::string startedEndpoint;
  auto core = makeCore(transport, startedEndpoint);
  EventListener established;
  EventListener lost;
  core.onConnectionEstablished(
      [&established](ConnectionId, std::string) { established.signal(); });
  core.onConnectionLost([&lost] { lost.signal(); });

  core.connected();
  REQUIRE(transport.requests.size() == 1);
  core.disconnected();
  REQUIRE(lost.signaled);
  REQUIRE_FALSE(core.isConnected());

  core.connected();
  REQUIRE(transport.requests.size() == 2);
  transport.failCancelled();
  REQUIRE(transport.requests.size() == 2);

  const auto assignedId = ConnectionId::generate();
  transport.respond(
      Response{ResponsePayloadVariant{NewConnectionResponse{assignedId}}});
  transport.respond(Response{ResponsePayloadVariant{
      MonitoringConnectionDetailsResponse{assignedId, "inproc://metadata"}}});

  REQUIRE(established.signaled);
  REQUIRE(core.isConnected());
}

TEST_CASE("control connection core rejects failed negotiation responses") {
  FakeTransport transport;
  std::string startedEndpoint;
  auto core = makeCore(transport, startedEndpoint);
  EventListener established;
  core.onConnectionEstablished(
      [&established](ConnectionId, std::string) { established.signal(); });

  core.connected();
  transport.respond(Response{ErrorCode::UNKOWN_ERROR, "request failed"});
  REQUIRE(transport.requests.size() == 1);
  REQUIRE_FALSE(core.isConnected());
  REQUIRE_FALSE(established.signaled);

  core.connected();
  const auto assignedId = ConnectionId::generate();
  transport.respond(
      Response{ResponsePayloadVariant{NewConnectionResponse{assignedId}}});
  REQUIRE(transport.requests.size() == 3);
  transport.respond(Response{ErrorCode::UNKOWN_ERROR, "details failed"});
  REQUIRE_FALSE(core.isConnected());
  REQUIRE_FALSE(established.signaled);
}

TEST_CASE("control connection core stops without reporting a loss") {
  FakeTransport transport;
  std::string startedEndpoint;
  auto core = makeCore(transport, startedEndpoint);
  EventListener lost;
  core.onConnectionLost([&lost] { lost.signal(); });

  core.connected();
  core.stop();

  REQUIRE(transport.stopCount == 1);
  REQUIRE_FALSE(core.isConnected());
  REQUIRE_FALSE(lost.signaled);
}

TEST_CASE(
    "control connection core continues after a disconnect during reconfiguration") {
  FakeTransport transport;
  std::string startedEndpoint;
  auto core = makeCore(transport, startedEndpoint);
  EventListener established;
  EventListener lost;
  core.onConnectionEstablished(
      [&established](ConnectionId, std::string) { established.signal(); });
  core.onConnectionLost([&lost] { lost.signal(); });

  core.connected();
  const auto initialId = ConnectionId::generate();
  transport.respond(
      Response{ResponsePayloadVariant{NewConnectionResponse{initialId}}});
  transport.respond(Response{ResponsePayloadVariant{
      MonitoringConnectionDetailsResponse{initialId, "inproc://metadata"}}});
  REQUIRE(core.isConnected());
  established.signaled = false;

  const auto requestedId = ConnectionId::generate();
  REQUIRE_NOTHROW(core.setConnectionId(
      requestedId, [&core](ConnectionId) {
        core.disconnected();
        throw nng::NNGError(nng::makeErrorCode(NNG_ECLOSED));
        return false;
      }));

  REQUIRE(lost.signaled);
  REQUIRE_FALSE(core.isConnected());

  core.connected();
  REQUIRE(transport.requests.size() == 3);
  transport.respond(
      Response{ResponsePayloadVariant{NewConnectionResponse{requestedId}}});
  transport.respond(Response{ResponsePayloadVariant{
      MonitoringConnectionDetailsResponse{requestedId, "inproc://metadata"}}});
  REQUIRE(established.signaled);
  REQUIRE(core.isConnected());
}

TEST_CASE(
    "control connection core clears reconfiguration after an unexpected "
    "close failure") {
  FakeTransport transport;
  std::string startedEndpoint;
  auto core = makeCore(transport, startedEndpoint);

  core.connected();
  const auto initialId = ConnectionId::generate();
  transport.respond(
      Response{ResponsePayloadVariant{NewConnectionResponse{initialId}}});
  transport.respond(Response{ResponsePayloadVariant{
      MonitoringConnectionDetailsResponse{initialId, "inproc://metadata"}}});
  REQUIRE(core.isConnected());

  const auto requestedId = ConnectionId::generate();
  REQUIRE_THROWS_AS(
      core.setConnectionId(requestedId, [](ConnectionId) -> bool {
        throw std::runtime_error("close failed");
      }),
      std::runtime_error);

  core.connected();
  REQUIRE(transport.requests.size() == 3);
}

}  // namespace
