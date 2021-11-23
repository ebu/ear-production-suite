#include <catch2/catch_all.hpp>
#include "communication/scene_connection_registry.hpp"
#include "communication/scene_connection_manager.hpp"
#include <tuple>

TEST_CASE("new connection") {
  ear::plugin::communication::SceneConnectionRegistry registry;
  using ear::plugin::communication::ConnectionType;

  auto id1 = registry.add(ConnectionType::METADATA_INPUT);
  REQUIRE(id1.isValid());
  REQUIRE(registry.size() == 1);
  REQUIRE(registry.has(id1));

  SECTION("reclaim a currently used connection id") {
    auto id2 = registry.add(ConnectionType::METADATA_INPUT, id1);
    REQUIRE(registry.size() == 2);
    REQUIRE(id1 != id2);
    REQUIRE(registry.has(id2));
    REQUIRE(id1.isValid());
    REQUIRE(id2.isValid());
  }
}

TEST_CASE("remove connection") {
  ear::plugin::communication::SceneConnectionRegistry registry;
  using ear::plugin::communication::ConnectionType;

  auto connection1 = registry.add(ConnectionType::METADATA_INPUT);
  auto connection2 = registry.add(ConnectionType::MONITORING);
  auto connection3 = registry.add(ConnectionType::METADATA_INPUT);

  REQUIRE(registry.size() == 3);

  SECTION("remove connection") {
    registry.remove(connection1);
    REQUIRE(registry.size() == 2);
    REQUIRE(registry.has(connection1) == false);
    REQUIRE(registry.has(connection2) == true);
    REQUIRE(registry.has(connection3) == true);
  }
  SECTION("remove non-existing id") {
    registry.remove(connection2);
    REQUIRE(registry.size() == 2);
    REQUIRE(registry.has(connection2) == false);

    registry.remove(connection2);
    REQUIRE(registry.size() == 2);
    REQUIRE(registry.has(connection1) == true);
    REQUIRE(registry.has(connection2) == false);
    REQUIRE(registry.has(connection3) == true);
  }
}

TEST_CASE("Scene Master Connection Manager - Input plugin") {
  using namespace ear::plugin;
  using namespace ear::plugin::communication;
  SceneConnectionManager manager;

  using Notification = std::tuple<SceneConnectionManager::Event, ConnectionId>;
  std::vector<Notification> notifications;

  manager.setEventHandler(
      [&notifications](SceneConnectionManager::Event event, ConnectionId id) {
        notifications.push_back({event, id});
      });

  NewConnectionMessage helloRequest{ConnectionType::METADATA_INPUT,
                                    ConnectionId{}};
  auto helloResponse =
      manager.handle(helloRequest).payloadAs<NewConnectionResponse>();
  REQUIRE(helloResponse.connectionId().isValid());

  REQUIRE(notifications.size() == 0);

  ObjectDetailsMessage detailsRequest{helloResponse.connectionId()};
  auto detailsResponse =
      manager.handle(detailsRequest).payloadAs<ConnectionDetailsResponse>();

  REQUIRE(notifications.size() == 1);
  REQUIRE(std::get<0>(notifications[0]) ==
          SceneConnectionManager::Event::INPUT_ADDED);
  REQUIRE(std::get<1>(notifications[0]) == helloResponse.connectionId());

  CloseConnectionMessage byeRequest{helloResponse.connectionId()};
  auto byeResponse =
      manager.handle(byeRequest).payloadAs<CloseConnectionResponse>();
  REQUIRE(byeResponse.connectionId() == helloResponse.connectionId());
  REQUIRE(notifications.size() == 2);
  REQUIRE(std::get<0>(notifications[1]) ==
          SceneConnectionManager::Event::INPUT_REMOVED);
  REQUIRE(std::get<1>(notifications[1]) == helloResponse.connectionId());
}

TEST_CASE("Scene Master Connection Manager - Monitoring plugin") {
  using namespace ear::plugin;
  using namespace ear::plugin::communication;
  SceneConnectionManager manager;

  using Notification = std::tuple<SceneConnectionManager::Event, ConnectionId>;
  std::vector<Notification> notifications;

  manager.setEventHandler(
      [&notifications](SceneConnectionManager::Event event, ConnectionId id) {
        notifications.push_back({event, id});
      });

  NewConnectionMessage helloRequest{ConnectionType::MONITORING, ConnectionId{}};
  auto helloResponse =
      manager.handle(helloRequest).payloadAs<NewConnectionResponse>();
  REQUIRE(helloResponse.connectionId().isValid());

  REQUIRE(notifications.size() == 0);

  MonitoringConnectionDetailsMessage detailsRequest{
      helloResponse.connectionId()};
  auto resp = manager.handle(detailsRequest);
  auto detailsResponse = resp.payloadAs<MonitoringConnectionDetailsResponse>();

  REQUIRE(notifications.size() == 1);
  REQUIRE(std::get<0>(notifications[0]) ==
          SceneConnectionManager::Event::MONITORING_ADDED);
  REQUIRE(std::get<1>(notifications[0]) == helloResponse.connectionId());

  CloseConnectionMessage byeRequest{helloResponse.connectionId()};
  auto byeResponse =
      manager.handle(byeRequest).payloadAs<CloseConnectionResponse>();
  REQUIRE(byeResponse.connectionId() == helloResponse.connectionId());
  REQUIRE(notifications.size() == 2);
  REQUIRE(std::get<0>(notifications[1]) ==
          SceneConnectionManager::Event::MONITORING_REMOVED);
  REQUIRE(std::get<1>(notifications[1]) == helloResponse.connectionId());
}

TEST_CASE("Scene Master Connection Manager - mismatch plugin") {
  using namespace ear::plugin;
  using namespace ear::plugin::communication;
  SceneConnectionManager manager;

  using Notification = std::tuple<SceneConnectionManager::Event, ConnectionId>;
  std::vector<Notification> notifications;

  manager.setEventHandler(
      [&notifications](SceneConnectionManager::Event event, ConnectionId id) {
        notifications.push_back({event, id});
      });

  NewConnectionMessage helloRequest{ConnectionType::METADATA_INPUT,
                                    ConnectionId{}};
  auto helloResponse =
      manager.handle(helloRequest).payloadAs<NewConnectionResponse>();
  REQUIRE(helloResponse.connectionId().isValid());

  REQUIRE(notifications.size() == 0);

  MonitoringConnectionDetailsMessage detailsRequest{
      helloResponse.connectionId()};
  auto resp = manager.handle(detailsRequest);
  REQUIRE(resp.errorCode() != ErrorCode::NO_ERROR);

  REQUIRE(notifications.size() == 0);
}

TEST_CASE("Scene Master Connection Manager - Item Properties ") {
  using namespace ear::plugin;
  using namespace ear::plugin::communication;
  SceneConnectionManager manager;

  NewConnectionMessage helloRequest{ConnectionType::METADATA_INPUT,
                                    ConnectionId{}};
  auto helloResponse =
      manager.handle(helloRequest).payloadAs<NewConnectionResponse>();
  ConnectionId connectionId = helloResponse.connectionId();

  ObjectDetailsMessage detailsRequest{connectionId};
  manager.handle(detailsRequest);

  using Notification = std::tuple<SceneConnectionManager::Event, ConnectionId>;
  std::vector<Notification> notifications;

  manager.setEventHandler(
      [&notifications](SceneConnectionManager::Event event, ConnectionId id) {
        notifications.push_back({event, id});
      });
}

TEST_CASE("ItemColour") {
  // not the best place to put this test, but there's currently no other obvious
  // place and the class and its test should be minimal and trivial anyway
  using namespace ear::plugin::ui;
  {
    ItemColour colour(12, 42, 64, 255);

    REQUIRE(colour.red() == 12);
    REQUIRE(colour.green() == 42);
    REQUIRE(colour.blue() == 64);
    REQUIRE(colour.alpha() == 255);
    REQUIRE(colour.argbValue() == 4278987328);
  }

  {
    ItemColour colour(1234567890);

    REQUIRE(colour.red() == 150);
    REQUIRE(colour.green() == 2);
    REQUIRE(colour.blue() == 210);
    REQUIRE(colour.alpha() == 73);
    REQUIRE(colour.argbValue() == 1234567890);
  }
}
