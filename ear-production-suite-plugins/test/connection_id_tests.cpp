#include <catch2/catch.hpp>
#include "communication/common_types.hpp"

TEST_CASE("connection id") {
  using namespace ear::plugin::communication;
  ConnectionId id;
  REQUIRE(id.string() == "00000000-0000-0000-0000-000000000000");
  ConnectionId idFromString("00000000-0000-0000-0000-000000000000");
}
