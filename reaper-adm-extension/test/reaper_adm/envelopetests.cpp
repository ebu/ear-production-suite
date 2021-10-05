#include <catch2/catch_all.hpp>
#include <chrono>
#include <gmock/gmock.h>
#include "blockbuilders.h"
#include "mocks/reaperapi.h"
#include <automationenvelope.h>
#include "fakeptr.h"

using namespace admplug;
using ::testing::Return;
using ::testing::DoubleEq;
using ::testing::DoubleNear;
using ::testing::Gt;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using ::testing::_;
using ::testing::NiceMock;

using namespace admplug::testing;
using namespace std::chrono_literals;

// Defined start envelope
TEST_CASE("Test creating an envelope with no points does nothing", "[envelope]") {
    FakePtrFactory fake;
    auto fakeEnvelope = fake.get<TrackEnvelope>();

    NiceMock<MockReaperAPI> api;
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, _, _, _, _, _, _)).Times(0);
    EXPECT_CALL(api, DeleteEnvelopePointRange(fakeEnvelope,  _, _)).Times(0);

    DefinedStartEnvelope env{fakeEnvelope, api};
    env.createPoints(0.0);
}

TEST_CASE("Test creating an envelope with a single point at time 0 inserts & sorts", "[envelope]") {
    FakePtrFactory fake;
    auto fakeEnvelope = fake.get<TrackEnvelope>();
    NiceMock<MockReaperAPI> api;
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, _, _, _, _, _, _)).Times(1);
    EXPECT_CALL(api, DeleteEnvelopePointRange(fakeEnvelope,  _, _)).Times(0);

    DefinedStartEnvelope env{fakeEnvelope, api};
    env.addPoint(AutomationPoint{0ns, 0ns, 1.0});
    env.createPoints(0.0);
}

TEST_CASE("Test creating an envelope with a single point at time >0 deletes preceding points", "[envelope]") {
    FakePtrFactory fake;
    auto fakeEnvelope = fake.get<TrackEnvelope>();
    NiceMock<MockReaperAPI> api;
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, _, _, _, _, _, _)).Times(1);
    auto startTime = 1000ns;
    EXPECT_CALL(api, DeleteEnvelopePointRange(fakeEnvelope, DoubleEq(0.0) , DoubleEq(startTime.count() / 1000000000.0))).Times(1);

    DefinedStartEnvelope env{fakeEnvelope, api};

    env.addPoint(AutomationPoint{startTime, 0ns, 1.0});
    env.createPoints(0.0);
}

TEST_CASE("When initial point has 0 duration, one envelope point is created for each automation point", "[envelope]") {
    FakePtrFactory fake;
    auto fakeEnvelope = fake.get<TrackEnvelope>();
    NiceMock<MockReaperAPI> api;
    auto startTime = 0ns;
    auto increment = 1000ns;
    auto currentTime = startTime;
    auto const pointCount = 22;

    DefinedStartEnvelope env{fakeEnvelope, api};
    env.addPoint(AutomationPoint{currentTime, 0ns, 0.1});
    for(auto pointNumber = 1; pointNumber != pointCount; ++pointNumber) {
      env.addPoint(AutomationPoint{currentTime, increment, 0.1});
      currentTime += increment;
    }
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, _, _, _, _, _, _)).Times(pointCount);
    env.createPoints(0.0);
}

TEST_CASE("When initial point has non 0 duration, extra point added to start and end of block", "[envelope]") {
    FakePtrFactory fake;
    auto fakeEnvelope = fake.get<TrackEnvelope>();
    NiceMock<MockReaperAPI> api;

    DefinedStartEnvelope env{fakeEnvelope, api};
    auto duration = 1000ns;
    env.addPoint(AutomationPoint{0ns, 1000ns, 0.1});
    auto initialTime = 0.0;
    auto endTime = duration.count() / 1000000000.0;
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, DoubleEq(initialTime), _, _, _, _, _)).Times(1);
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, DoubleEq(endTime), _, _, _, _, _)).Times(1);
    env.createPoints(0.0);
}

TEST_CASE("When subsequent points have non 0 duration, time of added point is at the end of a block", "[envelope]") {
    FakePtrFactory fake;
    auto fakeEnvelope = fake.get<TrackEnvelope>();
    NiceMock<MockReaperAPI> api;

    DefinedStartEnvelope env{fakeEnvelope, api};
    auto duration = 2000ns;
    env.addPoint(AutomationPoint{0ns, 0ns, 0.1});
    env.addPoint(AutomationPoint{0ns, 2000ns, 0.1});
    auto endTime = duration.count() / 1000000000.0;
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, DoubleEq(0.0), _, _, _, _, _)).Times(1);
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, DoubleEq(endTime), _, _, _, _, _)).Times(1);
    env.createPoints(0.0);
}

TEST_CASE("creating point with an offset offsets all points", "[envelope]") {
    FakePtrFactory fake;
    auto fakeEnvelope = fake.get<TrackEnvelope>();
    NiceMock<MockReaperAPI> api;

    DefinedStartEnvelope env{fakeEnvelope, api};
    auto duration = 2000ns;
    env.addPoint(AutomationPoint{0ns, 0ns, 0.1});
    env.addPoint(AutomationPoint{0ns, 2000ns, 0.1});
    auto endTime = duration.count() / 1000000000.0;
    auto offset = 2.5;
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, DoubleEq(0.0 + offset), _, _, _, _, _)).Times(1);
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, DoubleEq(endTime + offset), _, _, _, _, _)).Times(1);
    env.createPoints(offset);
}

TEST_CASE("Wrapped envelope does not insert extra points when shortest path is positive and direct", "[envelope]") {

    FakePtrFactory fake;
    auto fakeEnvelope = fake.get<TrackEnvelope>();
    NiceMock<MockReaperAPI> api;

    WrappingEnvelope env{fakeEnvelope, api};
    env.addPoint(AutomationPoint{0ns, 0ns, 0.1});
    env.addPoint(AutomationPoint{0ns, 2000ns, 0.59});
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, _, _, _, _, _, _)).Times(2);
    env.createPoints(0);
}

TEST_CASE("Wrapped envelope does not insert extra points when shortest path is negative and direct", "[envelope]") {

    FakePtrFactory fake;
    auto fakeEnvelope = fake.get<TrackEnvelope>();
    NiceMock<MockReaperAPI> api;

    WrappingEnvelope env{fakeEnvelope, api};
    env.addPoint(AutomationPoint{0ns, 0ns, 0.59});
    env.addPoint(AutomationPoint{0ns, 2000ns, 0.1});
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, _, _, _, _, _, _)).Times(2);
    env.createPoints(0);
}

TEST_CASE("Wrapped envelope inserts extra points at wrap position when shortest path is wrapped bottom to top", "[envelope]") {

    FakePtrFactory fake;
    auto fakeEnvelope = fake.get<TrackEnvelope>();
    NiceMock<MockReaperAPI> api;

    WrappingEnvelope env{fakeEnvelope, api};
    auto start = 0ns;
    auto end = 2000ns;
    env.addPoint(AutomationPoint{start, 0ns, 0.2});
    env.addPoint(AutomationPoint{start, end, 0.8});
    auto startTime = start.count() / 1000000000.0;
    auto endTime = end.count() / 1000000000.0;
    auto wrapTime = 0.5 * (end - start).count() / 1000000000.0;

    ::testing::InSequence seq{};
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, DoubleEq(startTime), DoubleEq(0.2), _, _, _, _)).Times(1);
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, DoubleEq(wrapTime), DoubleEq(0.0), _, _, _, _)).Times(1);
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, DoubleEq(wrapTime), DoubleEq(1.0), _, _, _, _)).Times(1);
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, DoubleEq(endTime), DoubleEq(0.8), _, _, _, _)).Times(1);
    env.createPoints(0);
}

TEST_CASE("Wrapped envelope inserts extra points at wrap position when shortest path is wrapped top to bottom", "[envelope]") {

    FakePtrFactory fake;
    auto fakeEnvelope = fake.get<TrackEnvelope>();
    NiceMock<MockReaperAPI> api;

    WrappingEnvelope env{fakeEnvelope, api};
    auto start = 0ns;
    auto end = 2000ns;
    env.addPoint(AutomationPoint{start, 0ns, 0.8});
    env.addPoint(AutomationPoint{start, end, 0.2});
    auto startTime = start.count() / 1000000000.0;
    auto endTime = end.count() / 1000000000.0;
    auto wrapTime = 0.5 * (end - start).count() / 1000000000.0;

    ::testing::InSequence seq{};
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, DoubleEq(startTime), DoubleEq(0.8), _, _, _, _)).Times(1);
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, DoubleEq(wrapTime), DoubleEq(1.0), _, _, _, _)).Times(1);
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, DoubleEq(wrapTime), DoubleEq(0.0), _, _, _, _)).Times(1);
    EXPECT_CALL(api, InsertEnvelopePoint(fakeEnvelope, DoubleEq(endTime), DoubleEq(0.2), _, _, _, _)).Times(1);
    env.createPoints(0);
}
