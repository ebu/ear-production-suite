#include <catch2/catch_all.hpp>
#include <gmock/gmock.h>
#include <adm/adm.hpp>
#include <adm/utilities/object_creation.hpp>
#include <algorithm>
#include "mediatrackelement.h"
#include "mocks/pluginsuite.h"
#include "mocks/reaperapi.h"

using namespace admplug;
using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Ref;
using ::testing::Return;
using ::testing::StrEq;

class MediaTrack;

TEST_CASE("Given a MediaTrackElement") {
    std::string audioObjectName = "Test";
    std::shared_ptr<const adm::AudioObject> audioObject = adm::createSimpleObject(audioObjectName).audioObject;
    int fakeTrack{1};
    auto track = reinterpret_cast<MediaTrack*>(&fakeTrack);
    NiceMock<MockReaperAPI> api;
    ON_CALL(api, createTrackAtIndex(_, _)).WillByDefault(Return(track));
    GUID guid{ 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
    ON_CALL(api, GetTrackGUID(_)).WillByDefault(Return(&guid));
    ON_CALL(api, ValidatePtr(_, _)).WillByDefault(Return(true));

    SECTION("that represents a group") {
        auto groupId = 4;
        auto groupElement = std::make_unique<GroupTrack>(std::vector<adm::ElementConstVariant>(1, audioObject), nullptr, std::make_unique<TrackGroup>(groupId));
        NiceMock<MockPluginSuite> fakePluginSuite{};
        SECTION("It will notify the supplied PluginSuite of group creation when project elements created") {
            EXPECT_CALL(fakePluginSuite, onCreateObjectTrack(_, _)).Times(0);
            EXPECT_CALL(fakePluginSuite, onCreateGroup(_, _)).Times(1);
            groupElement->createProjectElements(fakePluginSuite, api);
        }
        SECTION("It contains the groupId of the supplied group") {
            REQUIRE(groupElement->slaveOfGroups().size() == 0);
            REQUIRE(groupElement->masterOfGroup().id() == groupId);
        }
    }

    SECTION("that represents a nested group") {
        auto parentGroup = 2;
        auto childGroup = 4;
        auto parentElement = std::make_shared<GroupTrack>(std::vector<adm::ElementConstVariant>(1, audioObject), nullptr, std::make_unique<TrackGroup>(parentGroup));
        auto childElement = std::make_shared<GroupTrack>(std::vector<adm::ElementConstVariant>(1, audioObject), parentElement, std::make_unique<TrackGroup>(childGroup));

        SECTION("It contains the group ids of the supplied group and that supplied to the parent element") {
            auto parentSlaveOfGroups = parentElement->slaveOfGroups();
            auto childSlaveOfGroups = childElement->slaveOfGroups();

            auto containsGroup = [](int num) {
                return [num](TrackGroup const& group) {
                    if (group.id() == num) {
                        return true;
                    }
                    return false;
                };
            };

            REQUIRE(parentElement->masterOfGroup().id() == parentGroup);
            REQUIRE(childElement->masterOfGroup().id() == childGroup);

            REQUIRE(parentSlaveOfGroups.size() == 0);
            REQUIRE(childSlaveOfGroups.size() == 1);
            auto findRes = std::find_if(childSlaveOfGroups.begin(), childSlaveOfGroups.end(), containsGroup(parentGroup));
            REQUIRE(findRes != childSlaveOfGroups.end());
        }
    }

    SECTION("that represents a leaf track") {
        auto mediaTrackElement = std::make_unique<ObjectTrack>(std::vector<adm::ElementConstVariant>(1, audioObject), nullptr);
        NiceMock<MockPluginSuite> fakePluginSuite{};

        SECTION("after createProjectElements()") {
            int numTracks = 3;
            EXPECT_CALL(api, CountTracks(_)).Times(AnyNumber()).WillRepeatedly(Return(numTracks));
            int trackVal{ 1 };
            auto trackPtr = reinterpret_cast<MediaTrack*>(&trackVal);
            ON_CALL(api, createTrackAtIndex(_, _)).WillByDefault(Return(trackPtr));
            EXPECT_CALL(api, GetSetMediaTrackInfo_String(_, _, _, _)).Times(AnyNumber());

            SECTION("A new track is added to start of project") {
                EXPECT_CALL(api, createTrackAtIndex(0, _)).Times(1);
                mediaTrackElement->createProjectElements(fakePluginSuite, api);
            }

            SECTION("The track name is set from audio object name") {
                EXPECT_CALL(api, setTrackName(trackPtr, StrEq(audioObjectName))).Times(1);
                mediaTrackElement->createProjectElements(fakePluginSuite, api);
            }

            SECTION("The track name is truncated to 32 characters") {
                auto longObject = adm::createSimpleObject("012345678901234567890123456789012").audioObject;
                auto truncatedName = std::string{ "01234567890123456789012345678901" };
                auto truncatedTrackElement = std::make_unique<ObjectTrack>(std::vector<adm::ElementConstVariant>(1, longObject), nullptr);
                EXPECT_CALL(api, setTrackName(trackPtr, StrEq(truncatedName))).Times(1);
                truncatedTrackElement->createProjectElements(fakePluginSuite, api);
            }

            SECTION("The supplied PluginSuite is notified of track creation") {
                EXPECT_CALL(fakePluginSuite, onCreateObjectTrack(Ref(*mediaTrackElement), Ref(api))).Times(1);
                mediaTrackElement->createProjectElements(fakePluginSuite, api);
            }
        }
    }
}
