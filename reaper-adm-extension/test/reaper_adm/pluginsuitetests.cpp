#include <functional>
#include <catch2/catch_all.hpp>
#include "include_gmock.h"
#include <adm/elements.hpp>
#include "pluginsuite.h"
#include "mediatrackelement.h"
#include "projectnode.h"
#include "blockbuilders.h"
#include "mocks/reaperapi.h"
#include "mocks/automationenvelope.h"
#include "mocks/projectelements.h"
#include "pluginsuite_ear.h"
#include "mocks/track.h"

using namespace admplug;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::Ne;
using ::testing::AnyOf;
using ::testing::DoubleEq;
using ::testing::DoubleNear;
using ::testing::Gt;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::InSequence;

using namespace admplug::testing;

TEST_CASE("EAR plugin suite") {
  EARPluginSuite pluginSuite;
  NiceMock<MockReaperAPI> api;

  auto fakeRootElement = std::make_shared< NiceMock<MockRootElement>>();
  auto mockSceneTrackPtr = std::make_unique<NiceMock<MockTrack>>();
  auto& mockSceneTrack = *mockSceneTrackPtr;
  auto mockMonitorTrackPtr = std::make_unique<NiceMock<MockTrack>>();
  auto& mockMonitorTrack = *mockMonitorTrackPtr;

  SECTION("onProjectInit()") {
    ON_CALL(api, GetDawChannelCount()).WillByDefault(Return(128));

    EXPECT_CALL(api, createTrack()).Times(2).
      WillOnce(Return(ByMove(std::move(mockSceneTrackPtr)))).
      WillOnce(Return(ByMove(std::move(mockMonitorTrackPtr))));

    SECTION("EAR Scene Bus setup") {
      EXPECT_CALL(mockSceneTrack, setChannelCount(128));
      EXPECT_CALL(mockSceneTrack, setName(StrEq("EAR Scene Bus")));
      EXPECT_CALL(mockSceneTrack, createPlugin(StrEq("EAR Scene")));
    }

    SECTION("EAR Monitor Bus setup") {
      EXPECT_CALL(mockMonitorTrack, setChannelCount(128));
      EXPECT_CALL(mockMonitorTrack, setName(StrEq("EAR Monitor Bus")));
      EXPECT_CALL(mockMonitorTrack, createPlugin(StrEq("EAR Monitoring 0+2+0")));
    }

    SECTION("The EAR Scene Bus is routed to the EAR Monitor Bus") {
      EXPECT_CALL(mockSceneTrack, route(Ref(mockMonitorTrack), 128, 0, 0));
    }

    pluginSuite.onCreateProject(ProjectNode{ fakeRootElement }, api);
  }

  SECTION("onTrackCreate()") {
    auto fakeParentElement = std::make_shared<NiceMock<MockTrackElement>>();
    std::vector<adm::ElementConstVariant> admElements;
    auto trackElement = ObjectTrack(admElements, fakeParentElement);
    auto fakeTrackPtrVal{ 34 };
    auto fakeTrackPtr = reinterpret_cast<MediaTrack*>(&fakeTrackPtrVal);
    static GUID guid{ 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0} };
    ON_CALL(api, GetTrackGUID(_)).WillByDefault(Return(&guid));
    ON_CALL(api, TrackFX_GetFXGUID(_, _)).WillByDefault(Return(&guid));
    ON_CALL(api, ValidatePtr(_, _)).WillByDefault(Return(true));
    ON_CALL(api, createTrackAtIndex(_, _)).WillByDefault(Return(fakeTrackPtr));
    ON_CALL(*fakeParentElement, masterOfGroup).WillByDefault(Return(TrackGroup(0)));
    ON_CALL(api, GetDawChannelCount()).WillByDefault(Return(128));

    EXPECT_CALL(api, createTrack()).Times(2).
      WillOnce(Return(ByMove(std::move(mockSceneTrackPtr)))).
      WillOnce(Return(ByMove(std::move(mockMonitorTrackPtr))));

    pluginSuite.onCreateProject(ProjectNode{ fakeRootElement }, api);

    SECTION("suite creates a track") {
      EXPECT_CALL(api, createTrackAtIndex(_, _)).Times(1);
      SECTION("and track master send disabled") {
        EXPECT_CALL(api, disableTrackMasterSend(_));
      }
      pluginSuite.onCreateObjectTrack(trackElement, api);
      REQUIRE(trackElement.getTrack().get() != nullptr);
    }
  }
}