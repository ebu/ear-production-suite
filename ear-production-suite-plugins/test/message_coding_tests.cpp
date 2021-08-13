#include <catch2/catch.hpp>
#include "communication/commands.hpp"

TEST_CASE("NewConnectionMessage encoding/decoding") {
  SECTION("with default/invalid connection id") {
    ear::plugin::communication::ConnectionId id;
    ear::plugin::communication::NewConnectionMessage msg{
        ear::plugin::communication::ConnectionType::METADATA_INPUT, id};
    auto buffer = ear::plugin::communication::serialize(msg);

    auto req = ear::plugin::communication::parseRequest(buffer);
    auto received_msg =
        boost::get<ear::plugin::communication::NewConnectionMessage>(req);
    REQUIRE(!received_msg.connectionId().isValid());
  }

  SECTION("with explicit connection id") {
    auto id = ear::plugin::communication::ConnectionId::generate();
    ear::plugin::communication::NewConnectionMessage msg{
        ear::plugin::communication::ConnectionType::METADATA_INPUT, id};
    auto buffer = ear::plugin::communication::serialize(msg);

    auto req = ear::plugin::communication::parseRequest(buffer);
    auto received_msg =
        boost::get<ear::plugin::communication::NewConnectionMessage>(req);
    REQUIRE(received_msg.connectionId() == id);
  }
}

TEST_CASE("NewConnectionResponse encoding/decoding") {
  auto id = ear::plugin::communication::ConnectionId::generate();
  ear::plugin::communication::NewConnectionResponse msg{id};
  auto buffer = ear::plugin::communication::serialize(msg);

  auto resp = ear::plugin::communication::parseResponse(buffer);
  REQUIRE(resp.errorCode() == ear::plugin::communication::ErrorCode::NO_ERROR);
  auto received_msg =
      resp.payloadAs<ear::plugin::communication::NewConnectionResponse>();
  REQUIRE(received_msg.connectionId() == id);
}

TEST_CASE("CloseConnectionMessage encoding/decoding") {
  auto id = ear::plugin::communication::ConnectionId::generate();
  ear::plugin::communication::CloseConnectionMessage msg{id};
  auto buffer = ear::plugin::communication::serialize(msg);

  auto req = ear::plugin::communication::parseRequest(buffer);
  auto received_msg =
      boost::get<ear::plugin::communication::CloseConnectionMessage>(req);
  REQUIRE(received_msg.connectionId() == id);
}

TEST_CASE("CloseConnectionResponse encoding/decoding") {
  auto id = ear::plugin::communication::ConnectionId::generate();
  ear::plugin::communication::CloseConnectionResponse msg{id};
  auto buffer = ear::plugin::communication::serialize(msg);

  auto resp = ear::plugin::communication::parseResponse(buffer);
  REQUIRE(resp.errorCode() == ear::plugin::communication::ErrorCode::NO_ERROR);
  auto received_msg =
      resp.payloadAs<ear::plugin::communication::CloseConnectionResponse>();
  REQUIRE(received_msg.connectionId() == id);
}

TEST_CASE("ObjectDetails encoding/decoding") {
  auto id = ear::plugin::communication::ConnectionId::generate();
  ear::plugin::communication::ObjectDetailsMessage msg{id};
  auto buffer = ear::plugin::communication::serialize(msg);

  auto req = ear::plugin::communication::parseRequest(buffer);
  auto received_msg =
      boost::get<ear::plugin::communication::ObjectDetailsMessage>(req);
  REQUIRE(received_msg.connectionId() == id);
}

TEST_CASE("ConnectionDetailsResponse encoding/decoding") {
  auto id = ear::plugin::communication::ConnectionId::generate();
  ear::plugin::communication::ConnectionDetailsResponse msg{
      id, "SomeString that describes an endpoint"};
  auto buffer = ear::plugin::communication::serialize(msg);

  auto resp = ear::plugin::communication::parseResponse(buffer);
  REQUIRE(resp.errorCode() == ear::plugin::communication::ErrorCode::NO_ERROR);
  auto received_msg =
      resp.payloadAs<ear::plugin::communication::ConnectionDetailsResponse>();
  REQUIRE(received_msg.metadataEndpoint() ==
          "SomeString that describes an endpoint");
}

TEST_CASE("MonitoringConnectionDetails encoding/decoding") {
  auto id = ear::plugin::communication::ConnectionId::generate();
  ear::plugin::communication::MonitoringConnectionDetailsMessage msg{id};
  auto buffer = ear::plugin::communication::serialize(msg);

  auto req = ear::plugin::communication::parseRequest(buffer);
  auto received_msg = boost::get<
      ear::plugin::communication::MonitoringConnectionDetailsMessage>(req);
  REQUIRE(received_msg.connectionId() == id);
}

TEST_CASE("MonitoringConnectionDetailsResponse encoding/decoding") {
  auto id = ear::plugin::communication::ConnectionId::generate();
  ear::plugin::communication::MonitoringConnectionDetailsResponse msg{
      id, "SomeString that describes an nng endpoint"};

  auto buffer = ear::plugin::communication::serialize(msg);

  auto resp = ear::plugin::communication::parseResponse(buffer);
  REQUIRE(resp.errorCode() == ear::plugin::communication::ErrorCode::NO_ERROR);
  auto received_msg = resp.payloadAs<
      ear::plugin::communication::MonitoringConnectionDetailsResponse>();

  REQUIRE(received_msg.metadataEndpoint() ==
          "SomeString that describes an nng endpoint");
}

TEST_CASE("ErrorResponse encoding/decoding") {
  using namespace ear::plugin::communication;
  auto buffer = serializeErrorResponse(ErrorCode::UNKOWN_ERROR, "some message");
  auto resp = parseResponse(buffer);
  REQUIRE(resp.errorCode() == ErrorCode::UNKOWN_ERROR);
  REQUIRE(resp.errorDescription() == "some message");
  REQUIRE_THROWS(resp.payload());
}
