#include <catch2/catch_all.hpp>
#include <pluginsuite.h>
#include "mocks/reaperapi.h"
#include "mocks/parametised.h"

using namespace admplug;
using testing::NiceMock;
using ::testing::_;
using ::testing::Return;
using ::testing::An;

std::vector<int> initiallyNoUsedValues(PluginInstance&) {
    return std::vector<int>{};
}

std::vector<int> usingValues1To4(PluginInstance&) {
    return std::vector<int>{1,2,3,4};
}
std::vector<int> usingValues5To6(PluginInstance&) {
    return std::vector<int>{5,6};
}
std::vector<int> usingValue7(PluginInstance&) {
    return std::vector<int>{7};
}

class MediaTrack;
int fakeTrack{1};
auto track = reinterpret_cast<MediaTrack*>(&fakeTrack);
GUID guid{ 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};

void setApiDefaults(MockReaperAPI& api) {
    ON_CALL(api, CountTracks(_)).WillByDefault(Return(1));
    ON_CALL(api, GetTrack(_, _)).WillByDefault(Return(track));
    ON_CALL(api, GetTrackGUID(_)).WillByDefault(Return(&guid));
    ON_CALL(api, TrackFX_GetFXGUID(_, _)).WillByDefault(Return(&guid));
    ON_CALL(api, ValidatePtr(_, _)).WillByDefault(Return(true));
    ON_CALL(api, TrackFX_AddByName(_, _, false, TrackFXAddMode::QueryPresence)).WillByDefault(Return(1));
}


TEST_CASE("Ensure only values in range are returned") {
    NiceMock<MockReaperAPI> api{};
    setApiDefaults(api);

    UniqueValueAssigner::SearchCandidate sc{ "testplug", initiallyNoUsedValues };
    UniqueValueAssigner assigner{sc, 0, 1, api};

    SECTION("First returns 0") {
        auto val = assigner.getNextAvailableValue();
        REQUIRE(val.has_value());
        REQUIRE(*val == 0);
        SECTION("Second returns 1") {
            auto val = assigner.getNextAvailableValue();
            REQUIRE(val.has_value());
            REQUIRE(*val == 1);
            SECTION("Third returns no value") {
                auto val = assigner.getNextAvailableValue();
                REQUIRE(!val.has_value());
            }
        }
    }
}

TEST_CASE("Ensure only unused values are returned") {
    NiceMock<MockReaperAPI> api{};
    setApiDefaults(api);

    std::vector<UniqueValueAssigner::SearchCandidate> scs{
        UniqueValueAssigner::SearchCandidate{ "testplug", usingValues1To4 },
        UniqueValueAssigner::SearchCandidate{ "testplug", usingValue7 }
    };
    UniqueValueAssigner assigner{scs, 0, 8, api};

    SECTION("First returns 0") {
        auto val = assigner.getNextAvailableValue();
        REQUIRE(val.has_value());
        REQUIRE(*val == 0);
        SECTION("Second returns 5") {
            auto val = assigner.getNextAvailableValue();
            REQUIRE(val.has_value());
            REQUIRE(*val == 5);
            SECTION("Third returns 6") {
                auto val = assigner.getNextAvailableValue();
                REQUIRE(val.has_value());
                REQUIRE(*val == 6);
            }
        }
    }
}

TEST_CASE("Ensure consecutive free values are found") {
    NiceMock<MockReaperAPI> api{};
    setApiDefaults(api);

    std::vector<UniqueValueAssigner::SearchCandidate> scs{
        UniqueValueAssigner::SearchCandidate{ "testplug", usingValues1To4 },
        UniqueValueAssigner::SearchCandidate{ "testplug", usingValue7 }
    };
    UniqueValueAssigner assigner{scs, 0, 10, api};

    SECTION("First (requesting 2 consecutives) returns 5") {
        auto val = assigner.getNextAvailableValue(2);
        REQUIRE(val.has_value());
        REQUIRE(*val == 5);
        SECTION("Second (requesting 2 consecutives) returns 8") {
            auto val = assigner.getNextAvailableValue(2);
            REQUIRE(val.has_value());
            REQUIRE(*val == 8);
            SECTION("Third (requesting 2 consecutives) returns no value") {
                auto val = assigner.getNextAvailableValue(2);
                REQUIRE(!val.has_value());
            }
        }
    }
}

TEST_CASE("Ensure negative values are handled") {
    NiceMock<MockReaperAPI> api{};
    setApiDefaults(api);

    std::vector<UniqueValueAssigner::SearchCandidate> scs{
        UniqueValueAssigner::SearchCandidate{ "testplug", usingValues1To4 },
        UniqueValueAssigner::SearchCandidate{ "testplug", usingValue7 }
    };
    UniqueValueAssigner assigner{scs, -2, 8, api};

    SECTION("First returns -2") {
        auto val = assigner.getNextAvailableValue();
        REQUIRE(val.has_value());
        REQUIRE(*val == -2);
        SECTION("Second (requesting 2 consecutives) returns -1") {
            auto val = assigner.getNextAvailableValue(2);
            REQUIRE(val.has_value());
            REQUIRE(*val == -1);
            SECTION("Third returns 5") {
                auto val = assigner.getNextAvailableValue();
                REQUIRE(val.has_value());
                REQUIRE(*val == 5);
            }
        }
    }
}
