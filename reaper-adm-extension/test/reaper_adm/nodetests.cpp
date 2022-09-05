#include <catch2/catch_all.hpp>
#include <gmock/gmock.h>
#include <adm/utilities/object_creation.hpp>
#include "projectnode.h"
#include "nodefactory.h"
#include "mocks/projectelements.h"
#include "mocks/reaperapi.h"
#include "mocks/pluginsuite.h"
#include "mocks/pcmsourcecreator.h"
#include "mocks/track.h"
#include "mocks/importlistener.h"

using namespace admplug;
using ::testing::NiceMock;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::_;
using ::testing::Ref;

TEST_CASE("Given two Project Nodes") {
    auto parentProjectNode = std::make_unique<ProjectNode>(std::make_unique<NiceMock<MockProjectElement>>());
    auto childProjectNode = std::make_shared<ProjectNode>(std::make_unique<NiceMock<MockProjectElement>>());
    SECTION("addChildNode(node) does NOT add a child node by default") {
        auto childRes = parentProjectNode->addChildNode(childProjectNode);
        REQUIRE(parentProjectNode->children().size() == 0);
        REQUIRE(childRes == false);
    }

    auto parentTrackNode = std::make_unique<ProjectNode>(std::make_unique<NiceMock<MockTrackElement>>());
    auto childTrackElement = std::make_shared<NiceMock<MockTakeElement>>();
    ON_CALL(*childTrackElement, addParentProjectElement(_)).WillByDefault(Return(true));
    auto childTrackNode = std::make_shared<ProjectNode>(childTrackElement);
    SECTION("addChildNode(node) DOES adds a child node, where the child node is a valid child of the parent") {
        auto childRes = parentTrackNode->addChildNode(childTrackNode);
        auto nodeChildren = parentTrackNode->children();
        REQUIRE(nodeChildren.size() == 1);
        REQUIRE(nodeChildren[0]->getProjectElement() == childTrackElement);
        REQUIRE(childRes == true);
    }
}

TEST_CASE("ProjectNode forwards createProjectElements to children") {
    auto node = std::make_unique<ProjectNode>(std::make_unique<NiceMock<MockTrackElement>>());
    auto childElement = std::make_shared<NiceMock<MockTakeElement>>();
    ON_CALL(*childElement, addParentProjectElement(_)).WillByDefault(Return(true));
    auto childNode = std::make_shared<ProjectNode>(childElement);
    node->addChildNode(childNode);
    NiceMock<MockReaperAPI> api;
    NiceMock<MockPluginSuite> pluginSuite;
    NiceMock<MockImportListener> listener;
    EXPECT_CALL(*childElement, createProjectElements(Ref(pluginSuite), Ref(api))).Times(1);
    node->createProjectElements(pluginSuite, api, listener);
}

TEST_CASE("With a node creator and a simple object") {
    auto pcmSourceCreator = std::make_unique<NiceMock<MockIPCMSourceCreator>>();
    auto& pcmCreator = *pcmSourceCreator;
    NodeCreator creator{ std::move(pcmSourceCreator) };
    auto simpleObject = adm::createSimpleObject("Test");
    SECTION("when a track node is created") {
        auto trackNode = creator.createObjectTrackNode(simpleObject.audioObject, simpleObject.audioTrackUid, std::vector<adm::ElementConstVariant>(1, simpleObject.audioObject), nullptr);
        SECTION("The node is not null") {
            REQUIRE(trackNode);
            SECTION("and has the provided element") {
                REQUIRE(trackNode->getProjectElement()->hasAdmElement(simpleObject.audioObject));
            }
        }
    }

    SECTION("With a track Element") {
        auto trackElement = std::make_shared<NiceMock<MockTrackElement>>();
//        auto trackVal{ 2 };
        auto track = std::make_shared<NiceMock<MockTrack>>();
        NiceMock<MockReaperAPI> api;
        ON_CALL(*trackElement, getTrack()).WillByDefault(Return(track));

        SECTION("When a take is created") {
            std::shared_ptr<TakeElement> savedElement;
            auto saveTake = [&savedElement](std::shared_ptr<TakeElement> el) {
                savedElement = el;
            };

            ON_CALL(pcmCreator, addTake(_)).WillByDefault(Invoke(saveTake));

            auto takeNode = creator.createTakeNode(simpleObject.audioObject,
                trackElement);
            SECTION("It is not null") {
                REQUIRE(takeNode);
            }

            SECTION("It is registered with creator") {
                REQUIRE(savedElement != nullptr);
                REQUIRE(takeNode->getProjectElement() == savedElement);
            }
        }

        SECTION("If created without a uid the node creator adds a take to the source creator") {
            EXPECT_CALL(pcmCreator, addTake(_)).Times(1);
            auto takeElement = creator.createTakeNode(simpleObject.audioObject,
                trackElement);
        }

        SECTION("If created with a uid the node creator adds a take to the source creator") {
            EXPECT_CALL(pcmCreator, addTake(_)).Times(1);
            auto takeElement = creator.createTakeNode(simpleObject.audioObject,
                trackElement);
        }
        SECTION("When an automation node is created") {
            auto takeElement = std::make_shared<NiceMock<MockTakeElement>>();
            auto node = creator.createAutomationNode(ADMChannel{simpleObject.audioObject, simpleObject.audioChannelFormat, simpleObject.audioPackFormat, simpleObject.audioTrackUid}, trackElement, takeElement);
            SECTION("It is not null") {
                REQUIRE(node);
            }
        }
    }
}


