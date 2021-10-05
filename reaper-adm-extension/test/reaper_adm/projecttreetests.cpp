#include <catch2/catch_all.hpp>
#include <gmock/gmock.h>
#include <adm/utilities/object_creation.hpp>
#include <adm/document.hpp>
#include "mocks/reaperapi.h"
#include "mocks/channelindexer.h"
#include "mocks/pcmsourcecreator.h"
#include "mocks/pluginsuite.h"
#include "mocks/projectelements.h"
#include "mocks/nodefactory.h"
#include "mocks/importlistener.h"
#include "projecttree.h"

using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ByMove;
using ::testing::ReturnRef;
using ::testing::_;
using ::testing::Ref;
using ::testing::ByRef;
using ::testing::ByMove;
using ::testing::ResultOf;
using namespace admplug;

TEST_CASE("Project tree", "") {
    auto rootElement = std::make_shared< NiceMock<MockRootElement>>();
    auto fakeCreator = std::make_unique<NiceMock<MockNodeFactory>>();
    fakeCreator->delegateToFake();
    auto& fakeCreatorRef = *fakeCreator;
    NiceMock<MockPluginSuite> fakePluginSuite{};
    auto fakeListener = std::make_shared<NiceMock<MockImportListener>>();

    auto expectNodes = [&fakeCreatorRef](int numGroups, int numObjectTracks, int numTakes, int numAutomation, int numDirectTracks = 0) {
        std::string sectionName;

        sectionName = "it adds " + std::to_string(numGroups) + " group node(s)";
        SECTION(sectionName) {
            EXPECT_CALL(fakeCreatorRef, createGroupNode(_, _)).Times(numGroups);
        }

        sectionName = "it adds " + std::to_string(numObjectTracks) + " object track node(s)";
        SECTION(sectionName) {
            EXPECT_CALL(fakeCreatorRef, createObjectTrackNode(_, _)).Times(numObjectTracks);
        }

        sectionName = "it adds " + std::to_string(numDirectTracks) + " direct track node(s)";
        SECTION(sectionName) {
            EXPECT_CALL(fakeCreatorRef, createDirectTrackNode(_, _)).Times(numDirectTracks);
        }

        sectionName = "it adds " + std::to_string(numTakes) + " take node(s)";
        SECTION(sectionName) {
            EXPECT_CALL(fakeCreatorRef, createTakeNode(_, _, _)).Times(numTakes);
        }

        sectionName = "it adds " + std::to_string(numAutomation) + " automation node(s)";
        SECTION(sectionName) {
            EXPECT_CALL(fakeCreatorRef, createAutomationNode(_, _)).Times(numAutomation);
        }
    };

    SECTION("Given a project tree in initial state") {
        ProjectTree tree(std::move(fakeCreator),
                         std::make_unique<ProjectNode>(rootElement),
                         fakeListener);
        auto doc = adm::Document::create();

        SECTION("When it receives an audioProgramme") {
            auto programme = adm::AudioProgramme::create(adm::AudioProgrammeName{ "Test Prog" });
            doc->add(programme);

            SECTION("With no content references") {
                expectNodes(0, 0, 0, 0);

                tree.resetRoot();
                tree(programme);
            }
            SECTION("With one content reference") {
                auto content = adm::AudioContent::create(adm::AudioContentName("Test content"));
                programme->addReference(content);

                expectNodes(0, 0, 0, 0);

                tree.resetRoot();
                tree(programme);
            }
            SECTION("With more than one content reference") {
                auto content = adm::AudioContent::create(adm::AudioContentName("Test content"));
                auto secondContent = adm::AudioContent::create(adm::AudioContentName("Second content"));
                programme->addReference(content);
                programme->addReference(secondContent);

                expectNodes(0, 0, 0, 0);

                tree.resetRoot();
                tree(programme);
            }
        }

        SECTION("When it receives an audioContent") {
            auto content = adm::AudioContent::create(adm::AudioContentName{ "Test Content" });
            doc->add(content);

            SECTION("With no object references") {
                expectNodes(0, 0, 0, 0);

                tree.resetRoot();
                tree(content);
            }
            SECTION("With one object reference") {
                auto object = adm::createSimpleObject("Test object").audioObject;
                content->addReference(object);

                expectNodes(0, 0, 0, 0);

                tree.resetRoot();
                tree(content);
            }
            SECTION("With more than one object reference") {
                auto object = adm::createSimpleObject("Test object").audioObject;
                auto secondObject = adm::createSimpleObject("Second object").audioObject;
                content->addReference(object);
                content->addReference(secondObject);

                expectNodes(0, 0, 0, 0);

                tree.resetRoot();
                tree(content);
            }
        }

        SECTION("When it receives an audioObject") {
            auto ao = adm::AudioObject::create(adm::AudioObjectName{ "Test obj" });
            doc->add(ao);

            SECTION("With no tuid/pack/object references") {
                expectNodes(0, 0, 0, 0);

                tree.resetRoot();
                tree(ao);
            }
            SECTION("With one tuid & one pack reference") {
                auto apf = adm::AudioPackFormat::create(adm::AudioPackFormatName("apf"), adm::TypeDefinition::OBJECTS);
                auto asf = adm::AudioStreamFormat::create(adm::AudioStreamFormatName("asf"), adm::FormatDefinition::PCM);
                auto atf = adm::AudioTrackFormat::create(adm::AudioTrackFormatName("atf"), adm::FormatDefinition::PCM);
                auto acf = adm::AudioChannelFormat::create(adm::AudioChannelFormatName("acf"), adm::TypeDefinition::OBJECTS);
                auto atuid = adm::AudioTrackUid::create();
                ao->addReference(apf);
                apf->addReference(acf);
                asf->setReference(acf);
                atf->setReference(asf);
                ao->addReference(atuid);
                atuid->setReference(atf);
                atuid->setReference(apf);

                expectNodes(0, 0, 0, 0);

                tree.resetRoot();
                tree(ao);
            }
            SECTION("With multiple tuid & one pack references") {
                auto apf = adm::AudioPackFormat::create(adm::AudioPackFormatName("apf"), adm::TypeDefinition::OBJECTS);
                ao->addReference(apf);

                auto asf_1 = adm::AudioStreamFormat::create(adm::AudioStreamFormatName("asf_1"), adm::FormatDefinition::PCM);
                auto atf_1 = adm::AudioTrackFormat::create(adm::AudioTrackFormatName("atf_1"), adm::FormatDefinition::PCM);
                auto acf_1 = adm::AudioChannelFormat::create(adm::AudioChannelFormatName("acf_1"), adm::TypeDefinition::OBJECTS);
                auto atuid_1 = adm::AudioTrackUid::create();
                apf->addReference(acf_1);
                asf_1->setReference(acf_1);
                atf_1->setReference(asf_1);
                ao->addReference(atuid_1);
                atuid_1->setReference(atf_1);
                atuid_1->setReference(apf);

                auto asf_2 = adm::AudioStreamFormat::create(adm::AudioStreamFormatName("asf_2"), adm::FormatDefinition::PCM);
                auto atf_2 = adm::AudioTrackFormat::create(adm::AudioTrackFormatName("atf_2"), adm::FormatDefinition::PCM);
                auto acf_2 = adm::AudioChannelFormat::create(adm::AudioChannelFormatName("acf_2"), adm::TypeDefinition::OBJECTS);
                auto atuid_2 = adm::AudioTrackUid::create();
                apf->addReference(acf_2);
                asf_2->setReference(acf_2);
                atf_2->setReference(asf_2);
                ao->addReference(atuid_2);
                atuid_2->setReference(atf_2);
                atuid_2->setReference(apf);

                expectNodes(0, 0, 0, 0);

                tree.resetRoot();
                tree(ao);
            }
        }
    }
    
    SECTION("Given a project tree with programme visited") {
        ProjectTree tree(std::move(fakeCreator),
                         std::make_unique<ProjectNode>(rootElement),
                         fakeListener);
        TreeState state;

        auto doc = adm::Document::create();
        auto programme = adm::AudioProgramme::create(adm::AudioProgrammeName{ "Test Prog" });
        doc->add(programme);
        state.currentProgramme = programme;

        SECTION("When it receives a single audioContent") {
            auto content = adm::AudioContent::create(adm::AudioContentName{ "Test Content" });
            programme->addReference(content);

            SECTION("With no object references") {
                expectNodes(0, 0, 0, 0);

                tree.resetRoot();
                tree.setState(state);
                tree(content);
            }
            SECTION("With one object reference") {
                auto ao = adm::AudioObject::create(adm::AudioObjectName{ "Test obj" });
                content->addReference(ao);

                expectNodes(0, 0, 0, 0);

                tree.resetRoot();
                tree.setState(state);
                tree(content);
            }
            SECTION("With more than one object reference") {
                auto ao_1 = adm::AudioObject::create(adm::AudioObjectName{ "Test obj 1" });
                content->addReference(ao_1);
                auto ao_2 = adm::AudioObject::create(adm::AudioObjectName{ "Test obj 2" });
                content->addReference(ao_2);

                expectNodes(0, 0, 0, 0);

                tree.resetRoot();
                tree.setState(state);
                tree(content);
            }
        }

        SECTION("When it receives multiple audioContent") {
            auto content = adm::AudioContent::create(adm::AudioContentName{ "Test Content" });
            auto secondContent = adm::AudioContent::create(adm::AudioContentName("Second content"));
            programme->addReference(content);
            programme->addReference(secondContent);
            state.currentProgramme = programme;

            SECTION("With no object references") {
                expectNodes(2, 0, 0, 0);

                tree.resetRoot();
                tree.setState(state);
                tree(content);

                tree.resetRoot();
                tree.setState(state);
                tree(secondContent);
            }
            SECTION("With one object reference from one content") {
                auto ao = adm::AudioObject::create(adm::AudioObjectName{ "Test obj" });
                content->addReference(ao);

                expectNodes(2, 0, 0, 0);

                tree.resetRoot();
                tree.setState(state);
                tree(content);

                tree.resetRoot();
                tree.setState(state);
                tree(secondContent);
            }
            SECTION("With more than one object reference from one content") {
                auto ao_1 = adm::AudioObject::create(adm::AudioObjectName{ "Test obj 1" });
                content->addReference(ao_1);
                auto ao_2 = adm::AudioObject::create(adm::AudioObjectName{ "Test obj 2" });
                content->addReference(ao_2);

                expectNodes(2, 0, 0, 0);

                tree.resetRoot();
                tree.setState(state);
                tree(content);

                tree.resetRoot();
                tree.setState(state);
                tree(secondContent);
            }
            SECTION("With one object reference from both contents") {
                auto ao = adm::AudioObject::create(adm::AudioObjectName{ "Test obj" });
                content->addReference(ao);
                secondContent->addReference(ao);

                expectNodes(2, 0, 0, 0);

                tree.resetRoot();
                tree.setState(state);
                tree(content);

                tree.resetRoot();
                tree.setState(state);
                tree(secondContent);
            }
        }
    }
    
    SECTION("Given a project tree where a child of the current node has an element") {
        auto trackElement = std::make_unique<NiceMock<MockTrackElement>>();
        auto trackPtr = trackElement.get();
        ON_CALL(*trackElement, hasAdmElement(_)).WillByDefault(Return(true));

        auto rootNode = std::make_unique<ProjectNode>(std::move(trackElement));
        ProjectTree tree(std::move(fakeCreator),
                         std::move(rootNode),
                         fakeListener);
        auto doc = adm::Document::create();

        SECTION("When the tree receives that element") {
            auto obj = adm::AudioObject::create(adm::AudioObjectName{ "test" });
            doc->add(obj);

            expectNodes(0, 0, 0, 0);
            tree(obj);
            SECTION("it moves the tree to that child") {
                REQUIRE(tree.getState().currentNode->getProjectElement().get() == trackPtr);
            }
        }
    }

    SECTION("Given a project tree in initial state") {
        ProjectTree tree(std::move(fakeCreator),
                         std::make_unique<ProjectNode>(std::move(rootElement)),
                         fakeListener);
        auto doc = adm::Document::create();

        SECTION("For pack and channel type of 'objects'") {
            auto object = adm::createSimpleObject("Object");
            doc->add(object.audioObject);

            auto ao = object.audioObject;
            auto apf = object.audioPackFormat;
            auto acf = object.audioChannelFormat;
            auto atuid = object.audioTrackUid;

            SECTION("When it receives an object followed by a referenced tuid and channel format") {
                expectNodes(0, 1, 1, 1);
                tree.resetRoot();
                tree(ao);
                tree(atuid);
                tree(acf);
                SECTION("the the trees state is correctly updated") {
                    REQUIRE(tree.getState().currentObject == ao);
                    REQUIRE(tree.getState().currentUid == atuid);
                    REQUIRE(tree.getState().currentPack == apf);
                    REQUIRE(tree.getState().rootPack == apf);
                }
            }

            SECTION("When it receives an object followed by two referenced tuids and channel formats") {
                auto secondTuid = adm::AudioTrackUid::create();
                auto secondTF = adm::AudioTrackFormat::create(adm::AudioTrackFormatName{ "secondTF" }, adm::FormatDefinition::PCM);
                auto secondSF = adm::AudioStreamFormat::create(adm::AudioStreamFormatName{ "secondSF" }, adm::FormatDefinition::PCM);
                auto secondCF = adm::AudioChannelFormat::create(adm::AudioChannelFormatName{ "secondCF" }, adm::TypeDefinition::OBJECTS);
                ao->addReference(secondTuid);
                secondTuid->setReference(apf);
                secondTuid->setReference(secondTF);
                secondTF->setReference(secondSF);
                secondSF->setReference(secondCF);
                apf->addReference(secondCF);

                expectNodes(1, 2, 2, 2);

                tree.resetRoot();
                tree(ao);
                tree(atuid);
                tree(acf);
                SECTION("the the trees state is correctly updated") {
                    REQUIRE(tree.getState().currentObject == ao);
                    REQUIRE(tree.getState().currentUid == atuid);
                    REQUIRE(tree.getState().currentPack == apf);
                    REQUIRE(tree.getState().rootPack == apf);
                }

                tree.resetRoot();
                tree(ao);
                tree(secondTuid);
                tree(secondCF);
                SECTION("the the trees state is correctly updated") {
                    REQUIRE(tree.getState().currentObject == ao);
                    REQUIRE(tree.getState().currentUid == secondTuid);
                    REQUIRE(tree.getState().currentPack == apf);
                    REQUIRE(tree.getState().rootPack == apf);
                }
            }
        }

        SECTION("For pack and channel type NOT of 'objects'") {
            auto ao = adm::AudioObject::create(adm::AudioObjectName("ao"));
            auto apf = adm::AudioPackFormat::create(adm::AudioPackFormatName("apf"), adm::TypeDefinition::DIRECT_SPEAKERS);
            auto asf = adm::AudioStreamFormat::create(adm::AudioStreamFormatName("asf"), adm::FormatDefinition::PCM);
            auto atf = adm::AudioTrackFormat::create(adm::AudioTrackFormatName("atf"), adm::FormatDefinition::PCM);
            auto acf = adm::AudioChannelFormat::create(adm::AudioChannelFormatName("acf"), adm::TypeDefinition::DIRECT_SPEAKERS);
            auto atuid= adm::AudioTrackUid::create();
            doc->add(ao);
            ao->addReference(apf);
            apf->addReference(acf);
            asf->setReference(acf);
            atf->setReference(asf);
            ao->addReference(atuid);
            atuid->setReference(atf);
            atuid->setReference(apf);

            SECTION("When it receives an object followed by a referenced tuid and channel format") {
                expectNodes(0, 0, 1, 1, 1);
                tree.resetRoot();
                tree(ao);
                tree(atuid);
                tree(acf);
                SECTION("the the trees state is correctly updated") {
                    REQUIRE(tree.getState().currentObject == ao);
                    REQUIRE(tree.getState().currentUid == atuid);
                    REQUIRE(tree.getState().currentPack == apf);
                    REQUIRE(tree.getState().rootPack == apf);
                }
            }

            SECTION("When it receives an object followed by two referenced tuids and channel formats") {
                auto secondTuid = adm::AudioTrackUid::create();
                auto secondTF = adm::AudioTrackFormat::create(adm::AudioTrackFormatName{ "secondTF" }, adm::FormatDefinition::PCM);
                auto secondSF = adm::AudioStreamFormat::create(adm::AudioStreamFormatName{ "secondSF" }, adm::FormatDefinition::PCM);
                auto secondCF = adm::AudioChannelFormat::create(adm::AudioChannelFormatName{ "secondCF" }, adm::TypeDefinition::DIRECT_SPEAKERS);
                ao->addReference(secondTuid);
                secondTuid->setReference(apf);
                secondTuid->setReference(secondTF);
                secondTF->setReference(secondSF);
                secondSF->setReference(secondCF);
                apf->addReference(secondCF);

                expectNodes(0, 0, 1, 2, 1);

                tree.resetRoot();
                tree(ao);
                tree(atuid);
                tree(acf);
                SECTION("the the trees state is correctly updated") {
                    REQUIRE(tree.getState().currentObject == ao);
                    REQUIRE(tree.getState().currentUid == atuid);
                    REQUIRE(tree.getState().currentPack == apf);
                    REQUIRE(tree.getState().rootPack == apf);
                }

                tree.resetRoot();
                tree(ao);
                tree(secondTuid);
                tree(secondCF);
                SECTION("the the trees state is correctly updated") {
                    REQUIRE(tree.getState().currentObject == ao);
                    REQUIRE(tree.getState().currentUid == secondTuid);
                    REQUIRE(tree.getState().currentPack == apf);
                    REQUIRE(tree.getState().rootPack == apf);
                }
            }
        }

        SECTION("When it follows a route via a parent audioObject to multiple child audioObjects") {
            auto ao = adm::AudioObject::create(adm::AudioObjectName("ao"));
            auto ao_child1 = adm::AudioObject::create(adm::AudioObjectName("aoc1"));
            auto apf_child1 = adm::AudioPackFormat::create(adm::AudioPackFormatName("apfc1"), adm::TypeDefinition::OBJECTS);
            auto asf_child1 = adm::AudioStreamFormat::create(adm::AudioStreamFormatName("asfc1"), adm::FormatDefinition::PCM);
            auto atf_child1 = adm::AudioTrackFormat::create(adm::AudioTrackFormatName("atfc1"), adm::FormatDefinition::PCM);
            auto acf_child1 = adm::AudioChannelFormat::create(adm::AudioChannelFormatName("acfc1"), adm::TypeDefinition::OBJECTS);
            auto atuid_child1 = adm::AudioTrackUid::create();
            auto ao_child2 = adm::AudioObject::create(adm::AudioObjectName("aoc2"));
            auto apf_child2 = adm::AudioPackFormat::create(adm::AudioPackFormatName("apfc2"), adm::TypeDefinition::OBJECTS);
            auto asf_child2 = adm::AudioStreamFormat::create(adm::AudioStreamFormatName("asfc2"), adm::FormatDefinition::PCM);
            auto atf_child2 = adm::AudioTrackFormat::create(adm::AudioTrackFormatName("atfc2"), adm::FormatDefinition::PCM);
            auto acf_child2 = adm::AudioChannelFormat::create(adm::AudioChannelFormatName("acfc2"), adm::TypeDefinition::OBJECTS);
            auto atuid_child2 = adm::AudioTrackUid::create();

            doc->add(ao);
            ao->addReference(ao_child1);
            ao->addReference(ao_child2);

            ao_child1->addReference(apf_child1);
            apf_child1->addReference(acf_child1);
            asf_child1->setReference(acf_child1);
            atf_child1->setReference(asf_child1);
            ao_child1->addReference(atuid_child1);
            atuid_child1->setReference(atf_child1);
            atuid_child1->setReference(apf_child1);

            ao_child2->addReference(apf_child2);
            apf_child2->addReference(acf_child2);
            asf_child2->setReference(acf_child2);
            atf_child2->setReference(asf_child2);
            ao_child2->addReference(atuid_child2);
            atuid_child2->setReference(atf_child2);
            atuid_child2->setReference(apf_child2);

            expectNodes(0, 2, 2, 2);

            tree.resetRoot();
            tree(ao_child1);
            tree(atuid_child1);
            tree(acf_child1);
            SECTION("the the trees state is correctly updated") {
                REQUIRE(tree.getState().currentObject == ao_child1);
                REQUIRE(tree.getState().currentUid == atuid_child1);
                REQUIRE(tree.getState().currentPack == apf_child1);
                REQUIRE(tree.getState().rootPack == apf_child1);
            }

            tree.resetRoot();
            tree(ao_child2);
            tree(atuid_child2);
            tree(acf_child2);
            SECTION("the the trees state is correctly updated") {
                REQUIRE(tree.getState().currentObject == ao_child2);
                REQUIRE(tree.getState().currentUid == atuid_child2);
                REQUIRE(tree.getState().currentPack == apf_child2);
                REQUIRE(tree.getState().rootPack == apf_child2);
            }
        }

        SECTION("When it follows routes via multiple parent audioObjects to a shared child audioObject") {
            auto ac = adm::AudioContent::create(adm::AudioContentName("ac"));
            auto ao_parent1 = adm::AudioObject::create(adm::AudioObjectName("aop1"));
            auto ao_parent2 = adm::AudioObject::create(adm::AudioObjectName("aop2"));

            auto ao = adm::AudioObject::create(adm::AudioObjectName("ao"));
            auto apf = adm::AudioPackFormat::create(adm::AudioPackFormatName("apf"), adm::TypeDefinition::OBJECTS);
            auto asf = adm::AudioStreamFormat::create(adm::AudioStreamFormatName("asf"), adm::FormatDefinition::PCM);
            auto atf = adm::AudioTrackFormat::create(adm::AudioTrackFormatName("atf"), adm::FormatDefinition::PCM);
            auto acf = adm::AudioChannelFormat::create(adm::AudioChannelFormatName("acf"), adm::TypeDefinition::OBJECTS);
            auto atuid = adm::AudioTrackUid::create();

            doc->add(ac);
            ac->addReference(ao_parent1);
            ac->addReference(ao_parent2);
            ao_parent1->addReference(ao);
            ao_parent2->addReference(ao);

            ao->addReference(apf);
            apf->addReference(acf);
            asf->setReference(acf);
            atf->setReference(asf);
            ao->addReference(atuid);
            atuid->setReference(atf);
            atuid->setReference(apf);

            expectNodes(2, 1, 1, 1);

            tree.resetRoot();
            tree(ac);
            tree(ao_parent1);
            tree(ao);
            tree(atuid);
            tree(acf);
            SECTION("the the trees state is correctly updated") {
                REQUIRE(tree.getState().currentContent == ac);
                REQUIRE(tree.getState().currentObject == ao);
                REQUIRE(tree.getState().currentUid == atuid);
                REQUIRE(tree.getState().currentPack == apf);
                REQUIRE(tree.getState().rootPack == apf);
            }

            tree.resetRoot();
            tree(ac);
            tree(ao_parent2);
            tree(ao);
            tree(atuid);
            tree(acf);
            SECTION("the the trees state is correctly updated") {
                REQUIRE(tree.getState().currentContent == ac);
                REQUIRE(tree.getState().currentObject == ao);
                REQUIRE(tree.getState().currentUid == atuid);
                REQUIRE(tree.getState().currentPack == apf);
                REQUIRE(tree.getState().rootPack == apf);
            }

        }

        SECTION("When an audioObject references a nested pack structure of type 'objects'") {
            auto ao = adm::AudioObject::create(adm::AudioObjectName("ao"));
            auto apf_1 = adm::AudioPackFormat::create(adm::AudioPackFormatName("apf_1"), adm::TypeDefinition::OBJECTS);
            auto apf_2 = adm::AudioPackFormat::create(adm::AudioPackFormatName("apf_2"), adm::TypeDefinition::OBJECTS);
            doc->add(ao);
            ao->addReference(apf_1);
            apf_1->addReference(apf_2);

            auto asf_1 = adm::AudioStreamFormat::create(adm::AudioStreamFormatName("asf_1"), adm::FormatDefinition::PCM);
            auto atf_1 = adm::AudioTrackFormat::create(adm::AudioTrackFormatName("atf_1"), adm::FormatDefinition::PCM);
            auto acf_1 = adm::AudioChannelFormat::create(adm::AudioChannelFormatName("acf_1"), adm::TypeDefinition::OBJECTS);
            auto atuid_1 = adm::AudioTrackUid::create();
            apf_1->addReference(acf_1);
            asf_1->setReference(acf_1);
            atf_1->setReference(asf_1);
            ao->addReference(atuid_1);
            atuid_1->setReference(atf_1);
            atuid_1->setReference(apf_1);

            auto asf_2 = adm::AudioStreamFormat::create(adm::AudioStreamFormatName("asf_2"), adm::FormatDefinition::PCM);
            auto atf_2 = adm::AudioTrackFormat::create(adm::AudioTrackFormatName("atf_2"), adm::FormatDefinition::PCM);
            auto acf_2 = adm::AudioChannelFormat::create(adm::AudioChannelFormatName("acf_2"), adm::TypeDefinition::OBJECTS);
            auto atuid_2 = adm::AudioTrackUid::create();
            apf_2->addReference(acf_2);
            asf_2->setReference(acf_2);
            atf_2->setReference(asf_2);
            ao->addReference(atuid_2);
            atuid_2->setReference(atf_2);
            atuid_2->setReference(apf_2);

            expectNodes(1, 2, 2, 2);
            
            tree.resetRoot();
            tree(ao);
            tree(atuid_1);
            tree(acf_1);
            SECTION("the the trees state is correctly updated") {
                REQUIRE(tree.getState().currentObject == ao);
                REQUIRE(tree.getState().currentUid == atuid_1);
                REQUIRE(tree.getState().currentPack == apf_1);
                REQUIRE(tree.getState().rootPack == apf_1);
            }

            tree.resetRoot();
            tree(ao);
            tree(atuid_2);
            tree(acf_2);
            SECTION("the the trees state is correctly updated") {
                REQUIRE(tree.getState().currentObject == ao);
                REQUIRE(tree.getState().currentUid == atuid_2);
                REQUIRE(tree.getState().currentPack == apf_2);
                REQUIRE(tree.getState().rootPack == apf_1);
            }
        }

        SECTION("When an audioObject references a nested pack structure NOT of type 'objects'") {



            auto ao = adm::AudioObject::create(adm::AudioObjectName("ao"));
            auto apf_1 = adm::AudioPackFormat::create(adm::AudioPackFormatName("apf_1"), adm::TypeDefinition::DIRECT_SPEAKERS);
            auto apf_2 = adm::AudioPackFormat::create(adm::AudioPackFormatName("apf_2"), adm::TypeDefinition::DIRECT_SPEAKERS);
            doc->add(ao);
            ao->addReference(apf_1);
            apf_1->addReference(apf_2);

            auto asf_1 = adm::AudioStreamFormat::create(adm::AudioStreamFormatName("asf_1"), adm::FormatDefinition::PCM);
            auto atf_1 = adm::AudioTrackFormat::create(adm::AudioTrackFormatName("atf_1"), adm::FormatDefinition::PCM);
            auto acf_1 = adm::AudioChannelFormat::create(adm::AudioChannelFormatName("acf_1"), adm::TypeDefinition::DIRECT_SPEAKERS);
            auto atuid_1 = adm::AudioTrackUid::create();
            apf_1->addReference(acf_1);
            asf_1->setReference(acf_1);
            atf_1->setReference(asf_1);
            ao->addReference(atuid_1);
            atuid_1->setReference(atf_1);
            atuid_1->setReference(apf_1);

            auto asf_2 = adm::AudioStreamFormat::create(adm::AudioStreamFormatName("asf_2"), adm::FormatDefinition::PCM);
            auto atf_2 = adm::AudioTrackFormat::create(adm::AudioTrackFormatName("atf_2"), adm::FormatDefinition::PCM);
            auto acf_2 = adm::AudioChannelFormat::create(adm::AudioChannelFormatName("acf_2"), adm::TypeDefinition::DIRECT_SPEAKERS);
            auto atuid_2 = adm::AudioTrackUid::create();
            apf_2->addReference(acf_2);
            asf_2->setReference(acf_2);
            atf_2->setReference(asf_2);
            ao->addReference(atuid_2);
            atuid_2->setReference(atf_2);
            atuid_2->setReference(apf_2);

            expectNodes(0, 0, 1, 2, 1);

            tree.resetRoot();
            tree(ao);
            tree(atuid_1);
            tree(acf_1);
            SECTION("the the trees state is correctly updated") {
                REQUIRE(tree.getState().currentObject == ao);
                REQUIRE(tree.getState().currentUid == atuid_1);
                REQUIRE(tree.getState().currentPack == apf_1);
                REQUIRE(tree.getState().rootPack == apf_1);
            }

            tree.resetRoot();
            tree(ao);
            tree(atuid_2);
            tree(acf_2);
            SECTION("the the trees state is correctly updated") {
                REQUIRE(tree.getState().currentObject == ao);
                REQUIRE(tree.getState().currentUid == atuid_2);
                REQUIRE(tree.getState().currentPack == apf_2);
                REQUIRE(tree.getState().rootPack == apf_1);
            }
        }

    }

    SECTION("Given a project tree state with an object-type parent object and trackuid") {
        auto node = std::make_shared<ProjectNode>(rootElement);
        ProjectTree tree(std::move(fakeCreator),
                         node,
                         fakeListener);
        TreeState state;

        auto object = adm::createSimpleObject("Test object");
        state.currentObject = object.audioObject;
        state.currentUid = object.audioTrackUid;
        state.currentNode = node;
        state.packRepresentation = PackRepresentation::SingleMonoTrack;
        tree.setState(state);

        SECTION("when it receives an audioChannelFormat") {
            expectNodes(0, 1, 1, 1);
            tree(object.audioChannelFormat);
        }
    }

    SECTION("Given a project tree with a non object-type parent object and a track node") {
        auto node = std::make_shared<ProjectNode>(rootElement);
        ProjectTree tree(std::move(fakeCreator),
                         node,
                         fakeListener);
        TreeState state;
        auto takeElement = std::make_unique<NiceMock<MockTakeElement>>();
        auto audioObject = adm::AudioObject::create(adm::AudioObjectName{ "Test" });
        auto channelFormat = adm::AudioChannelFormat::create(adm::AudioChannelFormatName{ "Channel" }, adm::TypeDefinition::DIRECT_SPEAKERS);
        auto pack = adm::AudioPackFormat::create(adm::AudioPackFormatName{ "TestPack" }, adm::TypeDefinition::DIRECT_SPEAKERS);
        auto uid = adm::AudioTrackUid::create();
        pack->addReference(channelFormat);
        audioObject->addReference(pack);
        audioObject->addReference(uid);

        state.currentObject = audioObject;
        state.currentUid = uid;
        state.currentNode = std::make_shared<ProjectNode>(std::move(takeElement));
        state.packRepresentation = PackRepresentation::SingleMultichannelTrack;
        tree.setState(state);
        
        // non-object type automation not yet implemented
//        SECTION("it adds an automation node") {
//            EXPECT_CALL(fakeCreatorRef, createAutomationNode(_, _)).Times(1);
//        }
//        tree(channelFormat);
//        SECTION("it sets its current node to the automation node") {
//            REQUIRE(tree.getState().currentNode->element().get() == automationPtr);
//        }
    }

    SECTION("Given a project tree that is not at the project root") {
        auto rootNode = std::make_shared<ProjectNode>(rootElement);
        ProjectTree tree(std::move(fakeCreator),
                         rootNode,
                         fakeListener);
        TreeState state;
        auto trackElement = std::make_unique<NiceMock<MockTrackElement>>();


        state.currentNode = std::make_shared<ProjectNode>(std::move(trackElement));

        state.currentProgramme = adm::AudioProgramme::create(adm::AudioProgrammeName("ap"));
        state.currentContent = adm::AudioContent::create(adm::AudioContentName("ac"));
        state.currentObject = adm::AudioObject::create(adm::AudioObjectName("ao"));
        state.currentPack = adm::AudioPackFormat::create(adm::AudioPackFormatName("apf"), adm::TypeDefinition::OBJECTS);
        state.rootPack = adm::AudioPackFormat::create(adm::AudioPackFormatName("apf_root"), adm::TypeDefinition::OBJECTS);

        tree.setState(state);

        SECTION("On resetRoot()") {
            tree.resetRoot();
            SECTION("The current node is reset to the project root") {
                REQUIRE(tree.getState().currentNode == rootNode);
            }
            SECTION("The current programme is reset") {
                REQUIRE(tree.getState().currentProgramme == nullptr);
            }
            SECTION("The current content is reset") {
                REQUIRE(tree.getState().currentContent == nullptr);
            }
            SECTION("The current object is reset") {
                REQUIRE(tree.getState().currentObject == nullptr);
            }
            SECTION("The current pack is reset") {
                REQUIRE(tree.getState().currentPack == nullptr);
            }
            SECTION("The root pack is reset") {
                REQUIRE(tree.getState().rootPack == nullptr);
            }
        }
    }

    SECTION("On project creation") {
        auto fakeElement = std::make_shared<NiceMock<MockProjectElement>>();
        auto rootNode = std::make_shared<ProjectNode>(fakeElement);
        ProjectTree tree{std::move(fakeCreator),
                         rootNode,
                         fakeListener};
        NiceMock<MockReaperAPI> api;

        SECTION("Creation request is forwarded to root element") {
            EXPECT_CALL(*fakeElement, createProjectElements(Ref(fakePluginSuite), Ref(api)));
            SECTION("Arrage view is updated") {
                EXPECT_CALL(api, UpdateArrangeForAutomation()).Times(1);
                SECTION("Waveform peaks are rebuilt") {
                    EXPECT_CALL(api, Main_OnCommandEx(ReaperAPI::BUILD_MISSING_PEAKS, _, _)).Times(1);
                }
            }
        }
        SECTION("PluginSuite is notified of project creation") {
            EXPECT_CALL(fakePluginSuite, onCreateProject(Ref(*rootNode), _)).Times(1);
        }
        tree.create(fakePluginSuite, api);
    }

    SECTION("Given a tree processing a multichannel track") {
        auto fakeElement = std::make_shared<NiceMock<MockProjectElement>>();
        auto rootNode = std::make_shared<ProjectNode>(fakeElement);
        auto takeElement = std::make_shared<NiceMock<MockTakeElement>>();
        auto trackElement = std::make_shared<NiceMock<MockTrackElement>>();
        ON_CALL(*trackElement, addParentProjectElement(_)).WillByDefault(Return(true));
        ON_CALL(*takeElement, addParentProjectElement(_)).WillByDefault(Return(true));
        auto takeNode = std::make_shared<ProjectNode>(takeElement);
        auto trackNode = std::make_shared<ProjectNode>(trackElement);
        ON_CALL(*fakeCreator, createTakeNode(_, _, _)).WillByDefault(Return(takeNode));
        ON_CALL(*fakeCreator, createObjectTrackNode(_, _)).WillByDefault(Return(trackNode));
        auto simple = adm::createSimpleObject("Test");
        ProjectTree tree{std::move(fakeCreator),
                         rootNode,
                         fakeListener};
        tree.setState(TreeState{rootNode,
                                nullptr,
                                nullptr,
                                simple.audioObject,
                                simple.audioPackFormat,
                                simple.audioPackFormat,
                                simple.audioTrackUid,
                                SingleMultichannelTrack});

        SECTION("that receives and audioChannelFormat") {
            SECTION("a channel is added to the current take") {
                EXPECT_CALL(*takeElement, addChannel(_)).Times(1);
                tree(simple.audioChannelFormat);
            }
            // TODO investigate possible memory leak to do with parent elements

        }
    }
}
