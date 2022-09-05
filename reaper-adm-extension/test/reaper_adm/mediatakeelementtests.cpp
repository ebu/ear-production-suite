#include <catch2/catch_all.hpp>
#include <gmock/gmock.h>
#include <adm/adm.hpp>
#include <adm/utilities/object_creation.hpp>
#include "mediatakeelement.h"
#include "mocks/projectelements.h"
#include "mocks/pluginsuite.h"
#include "mocks/reaperapi.h"
#include "mocks/track.h"
#include "testhelpers.h"

using namespace admplug;
using ::testing::_;
using ::testing::AnyNumber;
using ::testing::DoubleEq;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::DoAll;

TEST_CASE("MediaTakeElement") {
    int lengthInSecs = 3;
    auto tempObject = adm::AudioObject::create(adm::AudioObjectName{ "Test Object" },
        adm::Duration{ std::chrono::seconds{lengthInSecs} });
    auto trackUid = adm::AudioTrackUid::create();
    tempObject->addReference(trackUid);
    std::shared_ptr<adm::AudioObject const> audioObjectWithDuration{ tempObject };

    auto mediaTrackElement = std::make_shared<NiceMock<MockTrackElement>>();
    auto track = std::make_shared<NiceMock<MockTrack>>();
    auto te = std::dynamic_pointer_cast<TrackElement>(mediaTrackElement);
    te->setTrack(track);

    NiceMock<MockPluginSuite> fakePluginSuite{};
    auto mediaTakeElement = std::make_shared<MediaTakeElement>(audioObjectWithDuration, mediaTrackElement);

    SECTION("has parent element from construction") {
        REQUIRE(mediaTakeElement->countParentElements() == 1);
    }

    SECTION("on createProjectElements()") {
        NiceMock<MockReaperAPI> api;
        int fakeItemVal{ 2 };
        int fakeTakeVal{ 3 };
        auto fakeTrack = std::make_shared<NiceMock<MockTrack>>();
        auto mediaItem = reinterpret_cast<MediaItem*>(&fakeItemVal);
        auto mediaTake = reinterpret_cast<MediaItem_Take*>(&fakeTakeVal);
        ON_CALL(*mediaTrackElement, getTrack()).WillByDefault(Return(fakeTrack));
        ON_CALL(api, TrackFX_AddByName(_, _, _, _)).WillByDefault(Return(0));
        EXPECT_CALL(*mediaTrackElement, addMediaItem(_)).Times(AnyNumber()).WillRepeatedly(Return(mediaItem));
        EXPECT_CALL(api, AddTakeToMediaItem(_)).Times(AnyNumber()).WillRepeatedly(Return(mediaTake));
        EXPECT_CALL(api, SetMediaItemTake_Source(_, _)).Times(AnyNumber()).WillRepeatedly(Return(true));
        SECTION("when source previously set") {
            int sourceVal{ 4 };
            auto pcmSource = reinterpret_cast<PCM_source*>(&sourceVal);
            mediaTakeElement->setSource(pcmSource);

            SECTION("mediaItem is added to parent track") {
                EXPECT_CALL(*mediaTrackElement, addMediaItem(_)).Times(1);
                mediaTakeElement->createProjectElements(fakePluginSuite, api);
            }

            SECTION("mediaItem length is set from object") {
                EXPECT_CALL(api, SetMediaItemLength(mediaItem, DoubleEq(lengthInSecs), _)).Times(1);
                mediaTakeElement->createProjectElements(fakePluginSuite, api);
            }

            SECTION("take is added to media item") {
                EXPECT_CALL(api, AddTakeToMediaItem(mediaItem)).Times(1);
                mediaTakeElement->createProjectElements(fakePluginSuite, api);
            }

            SECTION("source is added to take") {
                EXPECT_CALL(api, SetMediaItemTake_Source(mediaTake, pcmSource)).Times(1);
                mediaTakeElement->createProjectElements(fakePluginSuite, api);
            }
        }
    }
}

TEST_CASE("MediaTakeElement with durationless audioobject") {
    auto audioObject = testhelper::createAudioObject(1);
    auto mediaTrackElement = std::make_shared<NiceMock<MockTrackElement>>();
    auto track = std::make_shared<NiceMock<MockTrack>>();
    auto te = std::dynamic_pointer_cast<TrackElement>(mediaTrackElement);
    te->setTrack(track);
    auto mediaTakeElement = std::make_unique<MediaTakeElement>(audioObject, mediaTrackElement);
    NiceMock<MockReaperAPI> api;
    NiceMock<MockPluginSuite> fakePluginSuite{};
    auto fakeTrack = std::make_shared<NiceMock<MockTrack>>();
    int fakeItemVal{ 2 };
    int fakeTakeVal{ 3 };
    auto mediaItem = reinterpret_cast<MediaItem*>(&fakeItemVal);
    auto mediaTake = reinterpret_cast<MediaItem_Take*>(&fakeTakeVal);
    ON_CALL(*mediaTrackElement, getTrack()).WillByDefault(Return(fakeTrack));
    EXPECT_CALL(*mediaTrackElement, addMediaItem(_)).Times(AnyNumber()).WillRepeatedly(Return(mediaItem));
    EXPECT_CALL(api, AddTakeToMediaItem(_)).Times(AnyNumber()).WillRepeatedly(Return(mediaTake));
    EXPECT_CALL(api, SetMediaItemTake_Source(_, _)).Times(AnyNumber()).WillRepeatedly(Return(true));
    SECTION("When source previously set") {
        int sourceVal{ 4 };
        auto pcmSource = reinterpret_cast<PCM_source*>(&sourceVal);
        mediaTakeElement->setSource(pcmSource);
        SECTION("Sets take to full length of time-based source") {
            double length{ 5.0 };
            EXPECT_CALL(api, GetMediaSourceLength(pcmSource, _)).Times(1).WillOnce(DoAll(SetArgPointee<1>(false), Return(length)));
            EXPECT_CALL(api, SetMediaItemLength(mediaItem, DoubleEq(length), _)).Times(1);
            mediaTakeElement->createProjectElements(fakePluginSuite, api);
        }
    }
}

namespace {
auto channelFormatDirectSpeakers(std::string name) {
    return adm::AudioChannelFormat::create(adm::AudioChannelFormatName(name.c_str()), adm::TypeDefinition::DIRECT_SPEAKERS);
}
}

TEST_CASE("MediaTakeElement returns channels in order added") {
    auto audioObject = testhelper::createAudioObject(1);
    auto mediaTrackElement = std::make_shared<NiceMock<MockTrackElement>>();
    auto mediaTakeElement = std::make_unique<MediaTakeElement>(audioObject, mediaTrackElement);
    auto uid = adm::AudioTrackUid::create();
    auto uid2 = adm::AudioTrackUid::create();
    auto uid3 = adm::AudioTrackUid::create();
    auto chanFormat = channelFormatDirectSpeakers("Ch1");
    auto chanFormat2 = channelFormatDirectSpeakers("Ch2");
    auto chanFormat3 = channelFormatDirectSpeakers("Ch3");
    std::vector<std::shared_ptr<adm::AudioTrackUid const>> channels{uid, uid2, uid3};
    std::vector<std::shared_ptr<adm::AudioTrackUid const>> channelsWrongOrder{uid, uid3, uid2};
    for(auto channel : channels) {
        mediaTakeElement->addTrackUid(channel);
    }
    auto takeChannels = mediaTakeElement->trackUids();
    REQUIRE(channels == takeChannels);
    REQUIRE(channelsWrongOrder != takeChannels);
}