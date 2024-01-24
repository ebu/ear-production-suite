#include <catch2/catch_all.hpp>
#include "include_gmock.h"
#include <adm/adm.hpp>
#include "objectautomationelement.h"
#include "mocks/projectelements.h"
#include "mocks/pluginsuite.h"
#include "mocks/reaperapi.h"
#include "blockbuilders.h"
#include "parameter.h"
#include "mocks/parameter.h"
#include "mocks/parametised.h"
#include "mocks/automationenvelope.h"

using namespace admplug;
using namespace admplug::testing;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Ref;
using ::testing::Matcher;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ByMove;

namespace {
std::shared_ptr<adm::AudioChannelFormat> objectChannel() {
    return adm::AudioChannelFormat::create(adm::AudioChannelFormatName{"test"}, adm::TypeDefinition::OBJECTS);
}
std::shared_ptr<adm::AudioPackFormat> objectPack() {
    return adm::AudioPackFormat::create(adm::AudioPackFormatName{"test"}, adm::TypeDefinition::OBJECTS);
}

}

TEST_CASE("ObjectAutomationElement") {
    auto fakeParent = std::make_shared<NiceMock<MockTakeElement>>();
    auto fakeParentTrack = std::make_shared<NiceMock<MockTrackElement>>();
    auto audioTrackUid = adm::AudioTrackUid::create();
    auto channelFormatMutable = adm::AudioChannelFormat::create(adm::AudioChannelFormatName{ "Test channelFormat" }, adm::TypeDefinition::OBJECTS);
    std::shared_ptr<const adm::AudioChannelFormat> channelFormat{ channelFormatMutable };
    auto packFormatMutable = adm::AudioPackFormat::create(adm::AudioPackFormatName{ "Test packFormat" }, adm::TypeDefinition::OBJECTS);
    std::shared_ptr<const adm::AudioPackFormat> packFormat{ packFormatMutable };

    SECTION("parentTake returns element provided on construction") {
        auto automationElement = std::make_unique<ObjectAutomationElement>(ADMChannel{nullptr, channelFormat, packFormat, audioTrackUid, 0}, fakeParentTrack, fakeParent);
        auto aeParentTake = automationElement->parentTake();
        REQUIRE(aeParentTake == fakeParent);
    }

    SECTION("onCreateProjectElements()") {
        NiceMock<MockPluginSuite> pluginSet;
        NiceMock<MockReaperAPI> api;

        SECTION("Pluginset onObjectAutomation called with node ref") {
            auto automationElement = std::make_unique<ObjectAutomationElement>(ADMChannel{nullptr, channelFormat, packFormat, audioTrackUid, 0}, fakeParentTrack, fakeParent);
            EXPECT_CALL(pluginSet, onObjectAutomation(Ref(*automationElement), Ref(api)));
            automationElement->createProjectElements(pluginSet, api);
        }
        SECTION("blocks() returns blocks referenced by object") {
            auto block = adm::AudioBlockFormatObjects{ adm::SphericalPosition{adm::Azimuth{1.0}} };
            channelFormatMutable->add(block);
            auto automationElement = std::make_unique<ObjectAutomationElement>(ADMChannel{nullptr, channelFormatMutable, packFormatMutable, audioTrackUid, 0}, fakeParentTrack, fakeParent);
            REQUIRE(automationElement->blocks().size() == 1);
            REQUIRE(automationElement->blocks()[0].get<adm::SphericalPosition>().get<adm::Azimuth>() == block.get<adm::SphericalPosition>().get<adm::Azimuth>());
        }
    }
}

TEST_CASE("Applying automation") {
    auto channelFormat = objectChannel();
    auto packFormat = objectPack();
    auto api = NiceMock<MockReaperAPI>{};
    using ns = std::chrono::nanoseconds;
    auto parentTrack = std::make_shared<NiceMock<MockTrackElement>>();
    auto parentTake = std::make_shared<NiceMock<MockTakeElement>>();

    MockPlugin plugin;
    auto envelope = std::make_unique<MockAutomationEnvelope>();
    auto& envRef = *envelope;

    SECTION("Apply sets parameter directly rather than adding envelope when only one point") {
        MockPluginParameter parameter;
        ON_CALL(parameter, admParameter()).WillByDefault(Return(AdmParameter::OBJECT_AZIMUTH));
        EXPECT_CALL(parameter, admParameter()).Times(AnyNumber());
        AutomationPoint point{ns::zero(), ns::zero(), 1.0};
        EXPECT_CALL(parameter, forwardMap(Matcher<AutomationPoint>(_))).WillOnce(Return(point));
        ON_CALL(parameter, getEnvelope(_)).WillByDefault(Return(ByMove(std::move(envelope))));

        auto block = initialSphericalBlock();
        channelFormat->add(static_cast<adm::AudioBlockFormatObjects>(block));
        EXPECT_CALL(parameter, set(Ref(plugin), point.value()));
        EXPECT_CALL(parameter, getEnvelope(Ref(plugin))).Times(0);

        auto element = std::make_unique<ObjectAutomationElement>(ADMChannel{nullptr, channelFormat, packFormat, adm::AudioTrackUid::create(), 0}, parentTrack, parentTake);
        element->apply(parameter, plugin);
    }

    SECTION("When more than one point") {
        MockPluginParameter parameter;
        ON_CALL(parameter, admParameter()).WillByDefault(Return(AdmParameter::OBJECT_AZIMUTH));
        EXPECT_CALL(parameter, admParameter()).Times(AnyNumber());
        AutomationPoint point1{ns::zero(), ns(1000000000), 0.5};
        AutomationPoint point2{ns(1000000000), ns::zero(), 1.0};
        EXPECT_CALL(parameter, forwardMap(Matcher<AutomationPoint>(_))).WillOnce(Return(point1)).WillOnce(Return(point2));
        ON_CALL(parameter, getEnvelope(_)).WillByDefault(Return(ByMove(std::move(envelope))));

        auto blockRange = ObjectTypeBlockRange{}.with(initialSphericalBlock().withDuration(1.0)).followedBy(SphericalCoordBlock{}.withAzimuth(60.0));
        for(auto& block : blockRange.asConstRange()) {
            channelFormat->add(block);
        }
        auto element = std::make_unique<ObjectAutomationElement>(ADMChannel{ nullptr, channelFormat, packFormat, adm::AudioTrackUid::create(), 0 }, parentTrack, parentTake);

        EXPECT_CALL(parameter, set(_, point1.value())).Times(AnyNumber());
        EXPECT_CALL(parameter, set(_, point2.value())).Times(AnyNumber());
        EXPECT_CALL(parameter, getEnvelope(_)).Times(AnyNumber());
        EXPECT_CALL(envRef, createPoints(_)).Times(AnyNumber());
        EXPECT_CALL(envRef, addPoint(_)).Times(AnyNumber());

        SECTION("apply() sets parameter directly") {
            EXPECT_CALL(parameter, set(Ref(plugin), point1.value())).Times(1);
            element->apply(parameter, plugin);
        }
        SECTION("apply() creates envelope") {
            EXPECT_CALL(parameter, getEnvelope(Ref(plugin))).Times(1);
            element->apply(parameter, plugin);
        }
        SECTION("apply() adds 2 points") {
            EXPECT_CALL(envRef, addPoint(_)).Times(2);
            element->apply(parameter, plugin);
        }
        SECTION("apply() creates points") {
            EXPECT_CALL(envRef, createPoints(_)).Times(1);
            element->apply(parameter, plugin);
        }
    }

    SECTION("When more than one point which simplify in to 1") {
        MockPluginParameter parameter;
        ON_CALL(parameter, admParameter()).WillByDefault(Return(AdmParameter::OBJECT_AZIMUTH));
        EXPECT_CALL(parameter, admParameter()).Times(AnyNumber());
        AutomationPoint point1{ns::zero(), ns(1000000000), 1.0};
        AutomationPoint point2{ns(1000000000), ns::zero(), 1.0};
        EXPECT_CALL(parameter, forwardMap(Matcher<AutomationPoint>(_))).WillOnce(Return(point1)).WillOnce(Return(point2));
        ON_CALL(parameter, getEnvelope(_)).WillByDefault(Return(ByMove(std::move(envelope))));

        auto blockRange = ObjectTypeBlockRange{}.with(initialSphericalBlock().withDuration(1.0)).followedBy(SphericalCoordBlock{});
        for(auto& block : blockRange.asConstRange()) {
            channelFormat->add(block);
        }
        auto element = std::make_unique<ObjectAutomationElement>(ADMChannel{ nullptr, channelFormat, packFormat, adm::AudioTrackUid::create(), 0 }, parentTrack, parentTake);

        EXPECT_CALL(parameter, set(_, point1.value())).Times(AnyNumber());
        EXPECT_CALL(parameter, getEnvelope(_)).Times(AnyNumber());
        EXPECT_CALL(envRef, createPoints(_)).Times(AnyNumber());
        EXPECT_CALL(envRef, addPoint(_)).Times(AnyNumber());

        SECTION("apply() sets parameter directly") {
            EXPECT_CALL(parameter, set(Ref(plugin), point1.value())).Times(1);
            element->apply(parameter, plugin);
        }
        SECTION("apply() does not create envelope, because the automation points simplify to just one point") {
            EXPECT_CALL(parameter, getEnvelope(Ref(plugin))).Times(0);
            EXPECT_CALL(envRef, addPoint(_)).Times(0);
            EXPECT_CALL(envRef, createPoints(_)).Times(0);
            element->apply(parameter, plugin);
        }
    }
}

TEST_CASE("JumpPosition Insertion"){

    auto channelFormat = objectChannel();
    auto packFormat = objectPack();
    auto api = NiceMock<MockReaperAPI>{};
    using ns = std::chrono::nanoseconds;
    auto parentTrack = std::make_shared<NiceMock<MockTrackElement>>();
    auto parentTake = std::make_shared<NiceMock<MockTakeElement>>();

    MockPlugin plugin;
    auto envelope = std::make_unique<MockAutomationEnvelope>();
    auto& envRef = *envelope;

    MockPluginParameter parameter;
    ON_CALL(parameter, admParameter()).WillByDefault(Return(AdmParameter::OBJECT_AZIMUTH));
    EXPECT_CALL(parameter, admParameter()).Times(AnyNumber());
    ON_CALL(parameter, getEnvelope(_)).WillByDefault(Return(ByMove(std::move(envelope))));

    SECTION("Jump Position Scenario 1: No jump position"){
        AutomationPoint point1{ns::zero(), ns(1000000000), 0.1};
        AutomationPoint point2{ns(1000000000), ns(1000000000), 0.2};
        AutomationPoint point3{ns(2000000000), ns(1000000000), 0.3};
        auto blockRange = ObjectTypeBlockRange{}.with(initialSphericalBlock().withDistance(0.0).withJumpPosition(false).withDuration(1.0))
                .followedBy(SphericalCoordBlock{}.withDistance(55.0).withJumpPosition(false).withDuration(1.0))
                .followedBy(SphericalCoordBlock{}.withDistance(20.0).withJumpPosition(false).withDuration(1.0));
        for(auto& block : blockRange.asConstRange()) {
            channelFormat->add(block);
        }
        auto element = std::make_unique<ObjectAutomationElement>(ADMChannel{nullptr, channelFormat, packFormat, adm::AudioTrackUid::create(), 0}, parentTrack, parentTake);

        EXPECT_CALL(parameter, forwardMap(Matcher<AutomationPoint>(_))).WillOnce(Return(point1)).WillOnce(Return(point2)).WillOnce(Return(point3));
        EXPECT_CALL(parameter, set(_, point1.value())).Times(1);
        EXPECT_CALL(parameter, getEnvelope(_)).Times(AnyNumber());
        EXPECT_CALL(envRef, createPoints(_)).Times(AnyNumber());
        EXPECT_CALL(envRef, addPoint(_)).Times(AnyNumber());

        SECTION("apply() adds 3 points") {
          EXPECT_CALL(envRef, addPoint(_)).Times(3);
          element->apply(parameter, plugin);
        }

        SECTION("apply() creates points") {
          EXPECT_CALL(envRef, createPoints(_)).Times(1);
          element->apply(parameter, plugin);
        }

        REQUIRE(element->blocks().size() == 3);

    }

    SECTION("Jump Position Scenario 2: With jump position"){
        auto blockRange = ObjectTypeBlockRange{}.with(initialSphericalBlock().withDistance(20.0).withJumpPosition(true).withDuration(1.0))
                .followedBy(SphericalCoordBlock{}.withDistance(55.0).withJumpPosition(true).withDuration(1.0))
                .followedBy(SphericalCoordBlock{}.withDistance(40.0).withJumpPosition(true).withDuration(1.0));
        for(auto& block : blockRange.asConstRange()) {
            channelFormat->add(block);
        }
        auto element = std::make_unique<ObjectAutomationElement>(ADMChannel{nullptr, channelFormat, packFormat, adm::AudioTrackUid::create(), 0}, parentTrack, parentTake);

        // Expect jump position to create extra points
        EXPECT_CALL(parameter, forwardMap(Matcher<AutomationPoint>(_))).WillRepeatedly([](AutomationPoint ap) {
            if(ap.time() == 0.0 && ap.duration() == 0.0) return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.1 };
            if(ap.time() == 0.0 && ap.duration() == 1.0) return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.1 };
            if(ap.time() == 1.0 && ap.duration() == 0.0) return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.2 };
            if(ap.time() == 1.0 && ap.duration() == 1.0) return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.2 };
            if(ap.time() == 2.0 && ap.duration() == 0.0) return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.3 };
            if(ap.time() == 2.0 && ap.duration() == 1.0) return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.3 };
            ADD_FAILURE(); // Unexpected request to forwardMap - we were not expecting this point
            return ap;
        });

        EXPECT_CALL(parameter, set(_, 0.1)).Times(1);
        EXPECT_CALL(parameter, getEnvelope(_)).Times(AnyNumber());
        EXPECT_CALL(envRef, createPoints(_)).Times(AnyNumber());
        EXPECT_CALL(envRef, addPoint(_)).Times(AnyNumber());

        SECTION("apply() adds 5 points") { // Expect final point to be left out during simplify as redundant
          EXPECT_CALL(envRef, addPoint(_)).Times(5);
          element->apply(parameter, plugin);
        }

        SECTION("apply() creates points") {
          EXPECT_CALL(envRef, createPoints(_)).Times(1);
          element->apply(parameter, plugin);
        }

        REQUIRE(element->blocks().size() == 3);
    }

    SECTION("Jump Position Scenario 3: With jump position and interpolation"){
        auto blockRange = ObjectTypeBlockRange{}.with(initialSphericalBlock().withDistance(20.0).withJumpPosition(true, std::chrono::nanoseconds{300}).withDuration(1.0))
                .followedBy(SphericalCoordBlock{}.withDistance(55.0).withJumpPosition(true, std::chrono::nanoseconds{300}).withDuration(1.0))
                .followedBy(SphericalCoordBlock{}.withDistance(40.0).withJumpPosition(true, std::chrono::nanoseconds{300}).withDuration(1.0));
        for(auto& block : blockRange.asConstRange()) {
            channelFormat->add(block);
        }
        auto element = std::make_unique<ObjectAutomationElement>(ADMChannel{nullptr, channelFormat, packFormat, adm::AudioTrackUid::create(), 0}, parentTrack, parentTake);

        // Expect jump position to create extra points
        EXPECT_CALL(parameter, forwardMap(Matcher<AutomationPoint>(_))).WillRepeatedly([](AutomationPoint ap) {
            if(ap.timeNs() == std::chrono::nanoseconds{0}           && ap.durationNs() == std::chrono::nanoseconds{300})        return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.1 };
            if(ap.timeNs() == std::chrono::nanoseconds{300}         && ap.durationNs() == std::chrono::nanoseconds{999999700})  return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.1 };
            if(ap.timeNs() == std::chrono::nanoseconds{1000000000}  && ap.durationNs() == std::chrono::nanoseconds{300})        return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.2 };
            if(ap.timeNs() == std::chrono::nanoseconds{1000000300}  && ap.durationNs() == std::chrono::nanoseconds{999999700})  return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.2 };
            if(ap.timeNs() == std::chrono::nanoseconds{2000000000}  && ap.durationNs() == std::chrono::nanoseconds{300})        return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.3 };
            if(ap.timeNs() == std::chrono::nanoseconds{2000000300}  && ap.durationNs() == std::chrono::nanoseconds{999999700})  return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.3 };
            ADD_FAILURE(); // Unexpected request to forwardMap - we were not expecting this point
            return ap;
        });

        EXPECT_CALL(parameter, set(_, 0.1)).Times(1);
        EXPECT_CALL(parameter, getEnvelope(_)).Times(AnyNumber());
        EXPECT_CALL(envRef, createPoints(_)).Times(AnyNumber());
        EXPECT_CALL(envRef, addPoint(_)).Times(AnyNumber());

        SECTION("apply() adds 5 points") { // Expect final point to be left out during simplify as redundant
          EXPECT_CALL(envRef, addPoint(_)).Times(5);
          element->apply(parameter, plugin);
        }

        SECTION("apply() creates points") {
          EXPECT_CALL(envRef, createPoints(_)).Times(1);
          element->apply(parameter, plugin);
        }

        REQUIRE(element->blocks().size() == 3);
    }

    SECTION("Jump Position Scenario 4: With jump position and zero length blocks"){
        auto blockRange = ObjectTypeBlockRange{}.with(initialSphericalBlock().withDistance(55.0).withJumpPosition(true).withDuration(0.0))
                .followedBy(SphericalCoordBlock{}.withDistance(20.0).withJumpPosition(false).withDuration(1.0))
                .followedBy(SphericalCoordBlock{}.withDistance(55.0).withJumpPosition(true).withDuration(0.0))
                .followedBy(SphericalCoordBlock{}.withDistance(20.0).withJumpPosition(false).withDuration(1.0))
                .followedBy(SphericalCoordBlock{}.withDistance(55.0).withJumpPosition(true).withDuration(0.0))
                .followedBy(SphericalCoordBlock{}.withDistance(20.0).withJumpPosition(false).withDuration(1.0));
        for(auto& block : blockRange.asConstRange()) {
            channelFormat->add(block);
        }
        auto element = std::make_unique<ObjectAutomationElement>(ADMChannel{nullptr, channelFormat, packFormat, adm::AudioTrackUid::create(), 0}, parentTrack, parentTake);

        // Expect jump position not to create any extra points, because none of them have an interpolation length, so actually just shift effective time by block duration
        EXPECT_CALL(parameter, forwardMap(Matcher<AutomationPoint>(_))).WillRepeatedly([](AutomationPoint ap) {
            if(ap.timeNs() == std::chrono::nanoseconds{0}           && ap.durationNs() == std::chrono::nanoseconds{0})          return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.1 };
            if(ap.timeNs() == std::chrono::nanoseconds{0}           && ap.durationNs() == std::chrono::nanoseconds{1000000000}) return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.2 };
            if(ap.timeNs() == std::chrono::nanoseconds{1000000000}  && ap.durationNs() == std::chrono::nanoseconds{0})          return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.3 };
            if(ap.timeNs() == std::chrono::nanoseconds{1000000000}  && ap.durationNs() == std::chrono::nanoseconds{1000000000}) return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.4 };
            if(ap.timeNs() == std::chrono::nanoseconds{2000000000}  && ap.durationNs() == std::chrono::nanoseconds{0})          return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.5 };
            if(ap.timeNs() == std::chrono::nanoseconds{2000000000}  && ap.durationNs() == std::chrono::nanoseconds{1000000000}) return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.6 };
            ADD_FAILURE(); // Unexpected request to forwardMap - we were not expecting this point
            return ap;
        });

        EXPECT_CALL(parameter, set(_, 0.1)).Times(1);
        EXPECT_CALL(parameter, getEnvelope(_)).Times(AnyNumber());
        EXPECT_CALL(envRef, createPoints(_)).Times(AnyNumber());
        EXPECT_CALL(envRef, addPoint(_)).Times(AnyNumber());

        SECTION("apply() adds 6 points") {
            EXPECT_CALL(envRef, addPoint(_)).Times(6);
            element->apply(parameter, plugin);
        }

        SECTION("apply() creates points") {
          EXPECT_CALL(envRef, createPoints(_)).Times(1);
          element->apply(parameter, plugin);
        }

        REQUIRE(element->blocks().size() == 6);
    }

    SECTION("Jump Position Scenario 5: With jump position inc interpolation and zero length blocks"){
        auto blockRange = ObjectTypeBlockRange{}.with(initialSphericalBlock().withDistance(10.0).withJumpPosition(false).withDuration(0.0))
            .followedBy(SphericalCoordBlock{}.withDistance(20.0).withJumpPosition(false).withDuration(1.0))
            .followedBy(SphericalCoordBlock{}.withDistance(30.0).withJumpPosition(true).withDuration(1.0))
            .followedBy(SphericalCoordBlock{}.withDistance(40.0).withJumpPosition(true, std::chrono::nanoseconds{300}).withDuration(1.0))
            .followedBy(SphericalCoordBlock{}.withDistance(50.0).withJumpPosition(false).withDuration(1.0));
        for(auto& block : blockRange.asConstRange()) {
            channelFormat->add(block);
        }
        auto element = std::make_unique<ObjectAutomationElement>(ADMChannel{nullptr, channelFormat, packFormat, adm::AudioTrackUid::create(), 0}, parentTrack, parentTake);

        // Expect jump position not to create 2 extra points
        EXPECT_CALL(parameter, forwardMap(Matcher<AutomationPoint>(_))).WillRepeatedly([](AutomationPoint ap) {
            if(ap.timeNs() == std::chrono::nanoseconds{0}           && ap.durationNs() == std::chrono::nanoseconds{0})          return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.1 };
            if(ap.timeNs() == std::chrono::nanoseconds{0}           && ap.durationNs() == std::chrono::nanoseconds{1000000000}) return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.2 };
            if(ap.timeNs() == std::chrono::nanoseconds{1000000000}  && ap.durationNs() == std::chrono::nanoseconds{0})          return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.3 };
            if(ap.timeNs() == std::chrono::nanoseconds{1000000000}  && ap.durationNs() == std::chrono::nanoseconds{1000000000}) return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.3 };
            if(ap.timeNs() == std::chrono::nanoseconds{2000000000}  && ap.durationNs() == std::chrono::nanoseconds{300})        return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.4 };
            if(ap.timeNs() == std::chrono::nanoseconds{2000000300}  && ap.durationNs() == std::chrono::nanoseconds{999999700})  return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.4 };
            if(ap.timeNs() == std::chrono::nanoseconds{3000000000}  && ap.durationNs() == std::chrono::nanoseconds{1000000000}) return AutomationPoint{ ap.timeNs(), ap.durationNs(), 0.5 };
            ADD_FAILURE(); // Unexpected request to forwardMap - we were not expecting this point
            return ap;
        });

        EXPECT_CALL(parameter, set(_, 0.1)).Times(1);
        EXPECT_CALL(parameter, getEnvelope(_)).Times(AnyNumber());
        EXPECT_CALL(envRef, createPoints(_)).Times(AnyNumber());
        EXPECT_CALL(envRef, addPoint(_)).Times(AnyNumber());

        SECTION("apply() adds 7 points") {
            EXPECT_CALL(envRef, addPoint(_)).Times(7);
            element->apply(parameter, plugin);
        }

        SECTION("apply() creates points") {
            EXPECT_CALL(envRef, createPoints(_)).Times(1);
            element->apply(parameter, plugin);
        }

        REQUIRE(element->blocks().size() == 5);
    }
}