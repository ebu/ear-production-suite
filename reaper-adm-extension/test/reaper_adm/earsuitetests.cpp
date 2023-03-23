#include <catch2/catch_all.hpp>
#include <gmock/gmock.h>
#include "mocks/reaperapi.h"
#include "mocks/projectelements.h"
#include "mocks/track.h"
#include "mocks/parametised.h"
#include <projectnode.h>
#include <pluginsuite_ear.h>
#include <parameter.h>
#include <adm/utilities/object_creation.hpp>
#include <adm/adm.hpp>
#include <adm/common_definitions.hpp>
#include "fakeptr.h"
#include "objectautomationelement.h"
#include "directspeakerautomationelement.h"
#include "hoaautomationelement.h"
#include "mediatakeelement.h"
#include "mediatrackelement.h"

#define TRACK_MAPPING_PARAM_INDEX 0
#define TRACK_MAPPING_PARAM_MAXVAL 64

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::SaveArg;
using ::testing::ByRef;
using ::testing::ByMove;
using ::testing::AnyNumber;
using ::testing::InSequence;
using ::testing::_;
using ::testing::A;
using ::testing::An;
using ::testing::AtLeast;

using namespace admplug;

namespace {

FakePtrFactory fakePtr;

auto getPluginStr = [](std::string){
    return std::make_unique<NiceMock<MockPlugin>>();
};

auto getPluginInt = [](int){
    return std::make_unique<NiceMock<MockPlugin>>();
};

auto createPlugin = [](std::string){
    return std::make_unique<NiceMock<MockPlugin>>();
};

auto createTrack(std::function<void(MockTrack&)> setExpectations) {
    return [setExpectations]() {
      auto track = std::make_unique<NiceMock<MockTrack>>();
      setExpectations(*track);
      return track;
    };
}

auto getMockObjectAutoElement() {
    auto track = std::make_shared<NiceMock<MockTrack>>();
    auto take = std::make_shared<NiceMock<MockTakeElement>>();
    auto element = std::make_shared<NiceMock<MockObjectAutomation>>();
    ON_CALL(*element, getTrack()).WillByDefault(Return(track));
    ON_CALL(*element, parentTake()).WillByDefault(Return(take));
    ON_CALL(*track, getPlugin(An<std::string>())).WillByDefault(getPluginStr);
    ON_CALL(*track, createPlugin(An<std::string>())).WillByDefault(createPlugin);
    auto simpleObj = adm::createSimpleObject("Test");
    auto channel = ADMChannel(simpleObj.audioObject, simpleObj.audioChannelFormat, simpleObj.audioPackFormat, simpleObj.audioTrackUid, 0);
    auto channels = std::vector<ADMChannel>{ channel };
    ON_CALL(*element, channel()).WillByDefault(Return(channel));
    return element;
}

auto createDefaultTrack = createTrack([](MockTrack&) {});

auto admCommonDef = adm::Document::create();

std::vector<GUID> fxGuids{
    GUID { 1, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}},
    GUID { 1, 0, 0, {0, 0, 0, 0, 0, 0, 0, 1}},
    GUID { 1, 0, 0, {0, 0, 0, 0, 0, 0, 0, 2}},
    GUID { 1, 0, 0, {0, 0, 0, 0, 0, 0, 0, 3}},
    GUID { 1, 0, 0, {0, 0, 0, 0, 0, 0, 0, 4}},
    GUID { 1, 0, 0, {0, 0, 0, 0, 0, 0, 0, 5}},
    GUID { 1, 0, 0, {0, 0, 0, 0, 0, 0, 0, 6}},
    GUID { 1, 0, 0, {0, 0, 0, 0, 0, 0, 0, 7}}
};
ProjectNode initProject(EARPluginSuite& earSuite, MockReaperAPI const& api) {
    adm::addCommonDefinitionsTo(admCommonDef);
    auto project = std::make_shared<NiceMock<MockProjectElement>>();
    auto node = ProjectNode{project};
    static GUID guid{ 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
    ON_CALL(api, GetTrackGUID(_)).WillByDefault(Return(&guid));
    ON_CALL(api, TrackFX_GetFXGUID(_, 0)).WillByDefault([&api](MediaTrack* track, int fxNum) {
        // This assumes vsts are not reordered and we are not interested in different guids across tracks!
        return &fxGuids[fxNum];
    });
    ON_CALL(api, ValidatePtr(_,_)).WillByDefault(Return(true));
    ON_CALL(api, createTrack()).WillByDefault([&api](){
        return std::make_unique<TrackInstance>(api.createTrackAtIndex(0, false), api);
    });
    ON_CALL(api, createTrackAtIndex(_, _)).WillByDefault([&api](int index, bool fromEnd) {
        return fakePtr.get<MediaTrack>();
    });
    return node;
}

std::shared_ptr<ADMMetaData> genericMetadata;
std::shared_ptr<ADMMetaData> getGenericMetadata() {
    if(!genericMetadata) {
        genericMetadata = std::make_shared<ADMMetaData>("data/channels_mono_adm.wav");
    }
    return genericMetadata;
}

auto createAdmDocument() {
    auto doc = adm::Document::create();
    adm::addCommonDefinitionsTo(doc);
    return doc;
}

auto createElementsFromCommonDefinition(std::shared_ptr<adm::Document> doc, const std::string &pfIdStr) {
    auto helper = AdmCommonDefinitionHelper::getSingleton();
    auto obj = adm::AudioObject::create(adm::AudioObjectName("AO"));
    auto pfId = adm::parseAudioPackFormatId(pfIdStr);
    doc->add(obj);
    auto tdVal = std::stoi(pfIdStr.substr(3,4), nullptr, 16);
    auto pfIdVal = std::stoi(pfIdStr.substr(7,4), nullptr, 16);
    auto pf = doc->lookup(pfId);
    obj->addReference(pf);
    //Use helper to recurse all nested PFs
    auto pfData = helper->getPackFormatData(tdVal, pfIdVal);
    auto cfs = pf->getReferences<adm::AudioChannelFormat>();
    for(auto relCf : pfData->relatedChannelFormats) {
        // Need the doc instance of the CF, not the helpers
        auto cfId = relCf->channelFormat->get<adm::AudioChannelFormatId>();
        auto cf = doc->lookup(cfId);
        // Need to find the trackformat for the CF
        for(auto checkTf : doc->getElements<adm::AudioTrackFormat>()) {
            auto checkSf = checkTf->getReference<adm::AudioStreamFormat>();
            auto checkCf = checkSf->getReference<adm::AudioChannelFormat>();
            if(checkCf == cf) {
                auto uid = adm::AudioTrackUid::create();
                uid->setReference(pf);
                uid->setReference(checkTf);
                obj->addReference(uid);
                break;
            }
        }
    }
    return obj;
}

std::string SCENEMASTER_NAME{"EAR Scene"};
std::string RENDERER_NAME{"EAR Monitoring 0+2+0"};

MATCHER_P(HasParameter, param, "") { return (arg.admParameter() == param); }
MATCHER_P(HasIndex, index, "")
{
    return (arg.index() == index);
}

}

TEST_CASE("On create project, two tracks are created") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};
    auto project = std::make_shared<NiceMock<MockProjectElement>>();
    auto node = ProjectNode{project};
    EXPECT_CALL(api, createTrack()).Times(2).WillRepeatedly(createDefaultTrack);
    earSuite.onCreateProject(node, api);
}

TEST_CASE("On first create project, scene master and renderer added") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};
    auto project = std::make_shared<NiceMock<MockProjectElement>>();
    auto node = ProjectNode{project};

    auto returnTrackWithSceneMasterPluginExpectation = createTrack([](MockTrack& track){
        EXPECT_CALL(track, createPlugin(StrEq(SCENEMASTER_NAME)));
    });

    auto returnTrackWithRendererPluginExpectation = createTrack([](MockTrack& track){
        EXPECT_CALL(track, createPlugin(StrEq(RENDERER_NAME)));
    });

    EXPECT_CALL(api, createTrack()).WillOnce(returnTrackWithSceneMasterPluginExpectation).WillOnce(returnTrackWithRendererPluginExpectation);
    earSuite.onCreateProject(node, api);
}

TEST_CASE("On first create project, bus and renderer tracks set to 64 channels") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};
    auto project = std::make_shared<NiceMock<MockProjectElement>>();
    auto node = ProjectNode{project};

    auto returnTrackWithPluginExpectations = createTrack([](MockTrack& track){
        EXPECT_CALL(track, setChannelCount(64));
    });

    ON_CALL(api, createTrack()).WillByDefault(returnTrackWithPluginExpectations);
    earSuite.onCreateProject(node, api);
}

TEST_CASE("On first create project, bus track has master send disabled") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};
    EXPECT_CALL(api, disableTrackMasterSend(_)).Times(1);
    auto node = initProject(earSuite, api);
    earSuite.onCreateProject(node, api);
}

TEST_CASE("On subsequent create project, scene master and renderer not added") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};

    auto smTrack = createTrack([](MockTrack& track){
        ON_CALL(track, stillExists()).WillByDefault(Return(true));
        EXPECT_CALL(track, createPlugin(_)).Times(0);
    });

    auto smTrackExpectPluginCreate = createTrack([](MockTrack& track){
        ON_CALL(track, stillExists()).WillByDefault(Return(true));
        EXPECT_CALL(track, createPlugin(StrEq(SCENEMASTER_NAME))).Times(1);
    });

    auto monTrack = createTrack([](MockTrack& track){
        ON_CALL(track, stillExists()).WillByDefault(Return(true));
        EXPECT_CALL(track, createPlugin(_)).Times(0);
    });

    auto monTrackExpectPluginCreate = createTrack([](MockTrack& track){
        ON_CALL(track, stillExists()).WillByDefault(Return(true));
        EXPECT_CALL(track, createPlugin(StrEq(RENDERER_NAME))).Times(1);
    });

    auto node = initProject(earSuite, api);
    EXPECT_CALL(api, firstTrackWithPluginNamed(StrEq(RENDERER_NAME))).Times(2)
        .WillOnce(Return(ByMove(nullptr)))
        .WillOnce(Return(ByMove(std::move(monTrack()))));
    EXPECT_CALL(api, firstTrackWithPluginNamed(StrEq(SCENEMASTER_NAME))).Times(2)
        .WillOnce(Return(ByMove(nullptr)))
        .WillOnce(Return(ByMove(std::move(smTrack()))));
    EXPECT_CALL(api, createTrack()).Times(2)
        .WillOnce(Return(ByMove(std::move(smTrackExpectPluginCreate()))))
        .WillOnce(Return(ByMove(std::move(monTrackExpectPluginCreate()))));

    earSuite.onCreateProject(node, api);
    earSuite.onCreateProject(node, api);
}

TEST_CASE("Bus and renderer tracks positioned") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};
    auto project = std::make_shared<NiceMock<MockProjectElement>>();
    auto node = ProjectNode{project};
    auto mediaPosition{7};
    auto projectStart{1};

    SECTION("Before media if selected") {
      auto returnBusTrack = createTrack([&](MockTrack& track){
          EXPECT_CALL(track, moveToBefore(mediaPosition++));
      });

      ON_CALL(api, createTrack()).WillByDefault(returnBusTrack);
      ON_CALL(api, getTrackIndexOfSelectedMediaItem).WillByDefault(Return(mediaPosition));
      earSuite.onCreateProject(node, api);
    }

    SECTION("At start of project if no media selected") {
      auto returnBusTrack = createTrack([&](MockTrack& track){
          EXPECT_CALL(track, moveToBefore(projectStart++));
      });

      ON_CALL(api, createTrack()).WillByDefault(returnBusTrack);
      ON_CALL(api, getTrackIndexOfSelectedMediaItem).WillByDefault(Return(-1));
      earSuite.onCreateProject(node, api);
    }
}

TEST_CASE("Object tracks created and plugin instantiated") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};

    auto ao = adm::AudioObject::create(adm::AudioObjectName("test"));
    auto atu = adm::AudioTrackUid::create();

    auto trackElement = std::make_shared<ObjectTrack>(std::vector<adm::ElementConstVariant>{ao, atu}, nullptr);
    auto takeElement = std::make_shared<MediaTakeElement>(ao, trackElement, nullptr);

    takeElement->addChannelOfOriginal(0);
    trackElement->setTakeElement(takeElement);
    trackElement->setRepresentedAudioObject(ao);
    trackElement->setRepresentedAudioTrackUid(atu);

    auto node = initProject(earSuite, api);
    earSuite.onCreateProject(node, api);

    SECTION("A track element with no automation data does not create plugins") {
        EXPECT_CALL(api, createTrackAtIndex(_,_)).Times(1);
        EXPECT_CALL(api, TrackFX_AddByActualName(_,_,_,_)).Times(0);
        earSuite.onCreateObjectTrack(*trackElement, api);
    }

    SECTION("Single Object") {
        auto autoElement = std::make_shared<ObjectAutomationElement>(ADMChannel{ nullptr, nullptr, nullptr, nullptr, 0 }, trackElement, takeElement);
        trackElement->addAutomationElement(autoElement);

        SECTION("Creates track and plugin") {
            EXPECT_CALL(api, createTrackAtIndex(_, _)).Times(1);
            EXPECT_CALL(api, TrackFX_AddByActualName(_, StrEq("EAR Object"), _, _)).Times(1);
            earSuite.onCreateObjectTrack(*trackElement, api);
        }

        SECTION("Creates route from track") {
            EXPECT_CALL(api, RouteTrackToTrack(_, 0, _, 0, 1, _, _)).Times(1);
            ON_CALL(api, TrackFX_GetCount(_)).WillByDefault(Return(1)); // Needed by PluginInstance::getPluginIndex()
            float trackMappingVal = 1.f / (float)TRACK_MAPPING_PARAM_MAXVAL;
            EXPECT_CALL(api, TrackFX_SetParam(_, _, TRACK_MAPPING_PARAM_INDEX, trackMappingVal)).Times(1);
            earSuite.onCreateObjectTrack(*trackElement, api);
        }
    }

    SECTION("Multiple Objects (not sharing takes)") {
        auto autoElement = std::make_shared<ObjectAutomationElement>(ADMChannel{ nullptr, nullptr, nullptr, nullptr, 0 }, trackElement, takeElement);
        trackElement->addAutomationElement(autoElement);

        auto ao2 = adm::AudioObject::create(adm::AudioObjectName("test2"));
        auto atu2 = adm::AudioTrackUid::create();

        auto trackElement2 = std::make_shared<ObjectTrack>(std::vector<adm::ElementConstVariant>{ao2, atu2}, nullptr);
        auto takeElement2 = std::make_shared<MediaTakeElement>(ao2, trackElement2, nullptr);

        takeElement2->addChannelOfOriginal(1);
        trackElement2->setTakeElement(takeElement2);
        trackElement2->setRepresentedAudioObject(ao2);
        trackElement2->setRepresentedAudioTrackUid(atu2);

        auto autoElement2 = std::make_shared<ObjectAutomationElement>(ADMChannel{ nullptr, nullptr, nullptr, nullptr, 1 }, trackElement2, takeElement2);
        trackElement2->addAutomationElement(autoElement2);

        SECTION("Creates 2 tracks and 2 plugins") {
            EXPECT_CALL(api, createTrackAtIndex(_, _)).Times(2);
            EXPECT_CALL(api, TrackFX_AddByActualName(_, StrEq("EAR Object"), _, _)).Times(2);
            earSuite.onCreateObjectTrack(*trackElement, api);
            earSuite.onCreateObjectTrack(*trackElement2, api);
        }

        SECTION("Creates routes from tracks (sequentially)") {
            EXPECT_CALL(api, RouteTrackToTrack(_, 0, _, 0, 1, _, _)).Times(1);
            EXPECT_CALL(api, RouteTrackToTrack(_, 0, _, 1, 1, _, _)).Times(1);
            ON_CALL(api, TrackFX_GetCount(_)).WillByDefault(Return(1)); // Needed by PluginInstance::getPluginIndex()
            float trackMappingVal = 1.f / (float)TRACK_MAPPING_PARAM_MAXVAL;
            EXPECT_CALL(api, TrackFX_SetParam(_, _, TRACK_MAPPING_PARAM_INDEX, trackMappingVal)).Times(1);
            trackMappingVal = 2.f / (float)TRACK_MAPPING_PARAM_MAXVAL;
            EXPECT_CALL(api, TrackFX_SetParam(_, _, TRACK_MAPPING_PARAM_INDEX, trackMappingVal)).Times(1);
            earSuite.onCreateObjectTrack(*trackElement, api);
            earSuite.onCreateObjectTrack(*trackElement2, api);
        }
    }

    SECTION("Multiple Objects (sharing takes, same atu)") {
        auto autoElement = std::make_shared<ObjectAutomationElement>(ADMChannel{ nullptr, nullptr, nullptr, nullptr, 0 }, trackElement, takeElement);
        trackElement->addAutomationElement(autoElement);

        auto ao2 = adm::AudioObject::create(adm::AudioObjectName("test2"));

        auto trackElement2 = std::make_shared<ObjectTrack>(std::vector<adm::ElementConstVariant>{ao2, atu}, nullptr);

        trackElement2->setTakeElement(takeElement);
        trackElement2->setRepresentedAudioObject(ao2);
        trackElement2->setRepresentedAudioTrackUid(atu);

        auto autoElement2 = std::make_shared<ObjectAutomationElement>(ADMChannel{ nullptr, nullptr, nullptr, nullptr, 0 }, trackElement2, takeElement);
        trackElement2->addAutomationElement(autoElement2);

        SECTION("Creates track and 2 plugins") {
            EXPECT_CALL(api, createTrackAtIndex(_, _)).Times(1);
            EXPECT_CALL(api, TrackFX_AddByActualName(_, StrEq("EAR Object"), _, _)).Times(2);
            earSuite.onCreateObjectTrack(*trackElement, api);
            earSuite.onCreateObjectTrack(*trackElement2, api);
        }

        SECTION("Creates route from track") {
            EXPECT_CALL(api, RouteTrackToTrack(_, 0, _, 0, 1, _, _)).Times(1);
            ON_CALL(api, TrackFX_GetCount(_)).WillByDefault(Return(1)); // Needed by PluginInstance::getPluginIndex()
            float trackMappingVal = 1.f / (float)TRACK_MAPPING_PARAM_MAXVAL;
            EXPECT_CALL(api, TrackFX_SetParam(_, _, TRACK_MAPPING_PARAM_INDEX, trackMappingVal)).Times(AtLeast(1));
            earSuite.onCreateObjectTrack(*trackElement, api);
            earSuite.onCreateObjectTrack(*trackElement2, api);
        }
    }

    SECTION("Multiple Objects (sharing takes, different atu)") {
        auto autoElement = std::make_shared<ObjectAutomationElement>(ADMChannel{ nullptr, nullptr, nullptr, nullptr, 0 }, trackElement, takeElement);
        trackElement->addAutomationElement(autoElement);

        auto ao2 = adm::AudioObject::create(adm::AudioObjectName("test2"));
        auto atu2 = adm::AudioTrackUid::create();

        auto trackElement2 = std::make_shared<ObjectTrack>(std::vector<adm::ElementConstVariant>{ao2, atu2}, nullptr);

        trackElement2->setTakeElement(takeElement);
        trackElement2->setRepresentedAudioObject(ao2);
        trackElement2->setRepresentedAudioTrackUid(atu2);

        auto autoElement2 = std::make_shared<ObjectAutomationElement>(ADMChannel{ nullptr, nullptr, nullptr, nullptr, 0 }, trackElement2, takeElement);
        trackElement2->addAutomationElement(autoElement2);

        SECTION("Creates track and 2 plugins") {
            EXPECT_CALL(api, createTrackAtIndex(_, _)).Times(1);
            EXPECT_CALL(api, TrackFX_AddByActualName(_, StrEq("EAR Object"), _, _)).Times(2);
            earSuite.onCreateObjectTrack(*trackElement, api);
            earSuite.onCreateObjectTrack(*trackElement2, api);
        }

        SECTION("Creates route from track") {
            EXPECT_CALL(api, RouteTrackToTrack(_, 0, _, 0, 1, _, _)).Times(1);
            ON_CALL(api, TrackFX_GetCount(_)).WillByDefault(Return(1)); // Needed by PluginInstance::getPluginIndex()
            float trackMappingVal = 1.f / (float)TRACK_MAPPING_PARAM_MAXVAL;
            EXPECT_CALL(api, TrackFX_SetParam(_, _, TRACK_MAPPING_PARAM_INDEX, trackMappingVal)).Times(AtLeast(1));
            earSuite.onCreateObjectTrack(*trackElement, api);
            earSuite.onCreateObjectTrack(*trackElement2, api);
        }
    }

    SECTION("Automation is applied for all Automatable Object plugin parameters") {
        std::vector<AdmParameter> supportedParameters{
            AdmParameter::OBJECT_AZIMUTH,
            AdmParameter::OBJECT_DISTANCE,
            AdmParameter::OBJECT_ELEVATION,
            AdmParameter::OBJECT_GAIN,
            AdmParameter::OBJECT_HEIGHT,
            AdmParameter::OBJECT_WIDTH,
            AdmParameter::OBJECT_DEPTH,
            AdmParameter::OBJECT_DIFFUSE,
            //AdmParameter::OBJECT_DIVERGENCE,
            //AdmParameter::OBJECT_DIVERGENCE_AZIMUTH_RANGE
        };

        auto autoElement = getMockObjectAutoElement();
        for(auto parameter : supportedParameters) {
            EXPECT_CALL(*autoElement, apply(HasParameter(parameter), A<Plugin const &>()));
        }
        trackElement->addAutomationElement(std::shared_ptr<ObjectAutomation>(autoElement));
        earSuite.onCreateObjectTrack(*trackElement, api);
    }
}

TEST_CASE("Object tracks are routed to scene, and not sent to master") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};

    auto ao = adm::AudioObject::create(adm::AudioObjectName("test"));
    auto atu = adm::AudioTrackUid::create();

    auto trackElement = std::make_shared<ObjectTrack>(std::vector<adm::ElementConstVariant>{ao, atu}, nullptr);
    auto takeElement = std::make_shared<MediaTakeElement>(ao, trackElement, nullptr);

    takeElement->addChannelOfOriginal(0);
    trackElement->setTakeElement(takeElement);
    trackElement->setRepresentedAudioObject(ao);
    trackElement->setRepresentedAudioTrackUid(atu);

    auto node = initProject(earSuite, api);
    earSuite.onCreateProject(node, api);

    auto autoElement = std::make_shared<ObjectAutomationElement>(ADMChannel{ ao, nullptr, nullptr, atu, 0 }, trackElement, takeElement);
    trackElement->addAutomationElement(autoElement);
    EXPECT_CALL(api, disableTrackMasterSend(_)).Times(1);
    EXPECT_CALL(api, RouteTrackToTrack(_, 0, _, 0, 1, _, _)).Times(1);
    earSuite.onCreateObjectTrack(*trackElement, api);
}

TEST_CASE("Directspeaker tracks are created and configured") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};

    auto ao = adm::AudioObject::create(adm::AudioObjectName("test"));
    auto apf = adm::AudioPackFormat::create(adm::AudioPackFormatName("test"), adm::TypeDefinition::DIRECT_SPEAKERS, adm::parseAudioPackFormatId("AP_00010002"));
    auto atu0 = adm::AudioTrackUid::create();
    auto atu1 = adm::AudioTrackUid::create();

    auto trackElement = std::make_shared<DirectTrack>(std::vector<adm::ElementConstVariant>{ao}, nullptr);
    auto takeElement = std::make_shared<MediaTakeElement>(ao, trackElement, nullptr);

    takeElement->addChannelOfOriginal(0);
    takeElement->addChannelOfOriginal(1);
    trackElement->setTakeElement(takeElement);
    trackElement->setRepresentedAudioObject(ao);

    auto node = initProject(earSuite, api);
    earSuite.onCreateProject(node, api);

    auto autoElement1 = std::make_shared<DirectSpeakersAutomationElement>(ADMChannel{ ao, nullptr, apf, atu0, 0 }, trackElement, takeElement);
    auto autoElement2 = std::make_shared<DirectSpeakersAutomationElement>(ADMChannel{ ao, nullptr, apf, atu1, 1 }, trackElement, takeElement);
    trackElement->addAutomationElement(autoElement1);
    trackElement->addAutomationElement(autoElement2);

    SECTION("Track is created") {
        EXPECT_CALL(api, createTrackAtIndex(_, _)).Times(1);
        earSuite.onCreateDirectTrack(*trackElement, api);
    }

    SECTION("Plugin is created") {
        EXPECT_CALL(api, TrackFX_AddByActualName(_,StrEq("EAR DirectSpeakers"),_,_)).Times(1);
        earSuite.onCreateDirectTrack(*trackElement, api);
    }

    SECTION("Track is routed to scene and not to master") {
        EXPECT_CALL(api, disableTrackMasterSend(_)).Times(1);
        EXPECT_CALL(api, RouteTrackToTrack(_, _, _, _, _, _, _)).Times(1);
        earSuite.onCreateDirectTrack(*trackElement, api);
    }

    SECTION("Additional Tracks are routed sequentially") {
        auto aoEx = adm::AudioObject::create(adm::AudioObjectName("testEx"));
        auto apfEx = adm::AudioPackFormat::create(adm::AudioPackFormatName("testEx"), adm::TypeDefinition::DIRECT_SPEAKERS, adm::parseAudioPackFormatId("AP_00010002"));
        auto atu0Ex = adm::AudioTrackUid::create();
        auto atu1Ex = adm::AudioTrackUid::create();

        auto trackElementEx = std::make_shared<DirectTrack>(std::vector<adm::ElementConstVariant>{aoEx}, nullptr);
        auto takeElementEx = std::make_shared<MediaTakeElement>(aoEx, trackElementEx, nullptr);

        takeElementEx->addChannelOfOriginal(2);
        takeElementEx->addChannelOfOriginal(3);
        trackElementEx->setTakeElement(takeElementEx);
        trackElementEx->setRepresentedAudioObject(aoEx);

        auto autoElement1Ex = std::make_shared<DirectSpeakersAutomationElement>(ADMChannel{ aoEx, nullptr, apfEx, atu0Ex, 2 }, trackElementEx, takeElementEx);
        auto autoElement2Ex = std::make_shared<DirectSpeakersAutomationElement>(ADMChannel{ aoEx, nullptr, apfEx, atu1Ex, 3 }, trackElementEx, takeElementEx);
        trackElementEx->addAutomationElement(autoElement1Ex);
        trackElementEx->addAutomationElement(autoElement2Ex);

        EXPECT_CALL(api, RouteTrackToTrack(_, 0, _, 0, 2, _, _)).Times(1);
        EXPECT_CALL(api, RouteTrackToTrack(_, 0, _, 2, 2, _, _)).Times(1);

        earSuite.onCreateDirectTrack(*trackElement, api);
        earSuite.onCreateDirectTrack(*trackElementEx, api);
    }
}

TEST_CASE("HOA tracks are created and configured") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};

    auto ao = adm::AudioObject::create(adm::AudioObjectName("test"));
    auto apf = admCommonDef->lookup(adm::parseAudioPackFormatId("AP_00040001"));
    auto atu0 = adm::AudioTrackUid::create();
    auto atu1 = adm::AudioTrackUid::create();
    auto atu2 = adm::AudioTrackUid::create();
    auto atu3 = adm::AudioTrackUid::create();

    auto trackElement = std::make_shared<HoaTrack>(std::vector<adm::ElementConstVariant>{ao}, nullptr);
    auto takeElement = std::make_shared<MediaTakeElement>(ao, trackElement, nullptr);

    takeElement->addChannelOfOriginal(0);
    takeElement->addChannelOfOriginal(1);
    takeElement->addChannelOfOriginal(2);
    takeElement->addChannelOfOriginal(3);
    trackElement->setTakeElement(takeElement);
    trackElement->setRepresentedAudioObject(ao);

    auto node = initProject(earSuite, api);
    earSuite.onCreateProject(node, api);

    auto autoElement1 = std::make_shared<HoaAutomationElement>(ADMChannel{ ao, nullptr, apf, atu0, 0 }, trackElement, takeElement);
    auto autoElement2 = std::make_shared<HoaAutomationElement>(ADMChannel{ ao, nullptr, apf, atu1, 1 }, trackElement, takeElement);
    auto autoElement3 = std::make_shared<HoaAutomationElement>(ADMChannel{ ao, nullptr, apf, atu1, 2 }, trackElement, takeElement);
    auto autoElement4 = std::make_shared<HoaAutomationElement>(ADMChannel{ ao, nullptr, apf, atu1, 3 }, trackElement, takeElement);
    trackElement->addAutomationElement(autoElement1);
    trackElement->addAutomationElement(autoElement2);
    trackElement->addAutomationElement(autoElement3);
    trackElement->addAutomationElement(autoElement4);

    SECTION("Track is created") {
        EXPECT_CALL(api, createTrackAtIndex(_, _)).Times(1);
        earSuite.onCreateHoaTrack(*trackElement, api);
    }

    SECTION("Plugin is created") {
        EXPECT_CALL(api, TrackFX_AddByActualName(_,StrEq("EAR HOA"),_,_)).Times(1);
        earSuite.onCreateHoaTrack(*trackElement, api);
    }

    SECTION("Track is routed to scene and not to master") {
        EXPECT_CALL(api, disableTrackMasterSend(_)).Times(1);
        EXPECT_CALL(api, RouteTrackToTrack(_, _, _, _, _, _, _)).Times(1);
        earSuite.onCreateHoaTrack(*trackElement, api);
    }

    SECTION("Additional Tracks are routed sequentially") {
        auto aoEx = adm::AudioObject::create(adm::AudioObjectName("testEx"));
        auto apfEx = admCommonDef->lookup(adm::parseAudioPackFormatId("AP_00040001"));
        auto atu0Ex = adm::AudioTrackUid::create();
        auto atu1Ex = adm::AudioTrackUid::create();
        auto atu2Ex = adm::AudioTrackUid::create();
        auto atu3Ex = adm::AudioTrackUid::create();

        auto trackElementEx = std::make_shared<HoaTrack>(std::vector<adm::ElementConstVariant>{aoEx}, nullptr);
        auto takeElementEx = std::make_shared<MediaTakeElement>(aoEx, trackElementEx, nullptr);

        takeElementEx->addChannelOfOriginal(4);
        takeElementEx->addChannelOfOriginal(5);
        takeElementEx->addChannelOfOriginal(6);
        takeElementEx->addChannelOfOriginal(7);
        trackElementEx->setTakeElement(takeElementEx);
        trackElementEx->setRepresentedAudioObject(aoEx);

        auto autoElement1Ex = std::make_shared<HoaAutomationElement>(ADMChannel{ aoEx, nullptr, apfEx, atu0Ex, 4 }, trackElementEx, takeElementEx);
        auto autoElement2Ex = std::make_shared<HoaAutomationElement>(ADMChannel{ aoEx, nullptr, apfEx, atu1Ex, 5 }, trackElementEx, takeElementEx);
        auto autoElement3Ex = std::make_shared<HoaAutomationElement>(ADMChannel{ aoEx, nullptr, apfEx, atu2Ex, 6 }, trackElementEx, takeElementEx);
        auto autoElement4Ex = std::make_shared<HoaAutomationElement>(ADMChannel{ aoEx, nullptr, apfEx, atu3Ex, 7 }, trackElementEx, takeElementEx);
        trackElementEx->addAutomationElement(autoElement1Ex);
        trackElementEx->addAutomationElement(autoElement2Ex);
        trackElementEx->addAutomationElement(autoElement3Ex);
        trackElementEx->addAutomationElement(autoElement4Ex);

        EXPECT_CALL(api, RouteTrackToTrack(_, 0, _, 0, 4, _, _)).Times(1);
        EXPECT_CALL(api, RouteTrackToTrack(_, 0, _, 4, 4, _, _)).Times(1);

        earSuite.onCreateHoaTrack(*trackElement, api);
        earSuite.onCreateHoaTrack(*trackElementEx, api);
    }
}