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
using namespace admplug;
namespace {

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
    ON_CALL(*track, getPlugin(An<int>())).WillByDefault(getPluginInt);
    ON_CALL(*track, createPlugin(An<std::string>())).WillByDefault(createPlugin);
    auto simpleObj = adm::createSimpleObject("Test");
    auto channel = ADMChannel(simpleObj.audioObject, simpleObj.audioChannelFormat, simpleObj.audioPackFormat, simpleObj.audioTrackUid);
    auto channels = std::vector<ADMChannel>{ channel };
    ON_CALL(*element, channel()).WillByDefault(Return(channel));
    ON_CALL(*take, channels()).WillByDefault(Return(channels));
    return element;
}

auto createDefaultTrack = createTrack([](MockTrack&) {});

auto admCommonDef = adm::Document::create();

void initProject(EARPluginSuite& earSuite, MockReaperAPI const& api) {
    adm::addCommonDefinitionsTo(admCommonDef);
    auto project = std::make_shared<NiceMock<MockProjectElement>>();
    auto node = ProjectNode{project};
    ON_CALL(api, createTrack()).WillByDefault(createDefaultTrack);
    earSuite.onCreateProject(node, api);
}

std::shared_ptr<ADMMetaData> genericMetadata;
std::shared_ptr<ADMMetaData> getGenericMetadata() {
    if(!genericMetadata) {
        genericMetadata = std::make_shared<ADMMetaData>("data/channels_mono_adm.wav");
    }
    return genericMetadata;
}


std::string SCENEMASTER_NAME{"EAR Scene"};
std::string RENDERER_NAME{"EAR Monitoring 0+2+0"};


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

/* TODO - Not sure why you'd expect the renderer not to route to master? It doesn't follow the pattern of any other plugin suite

TEST_CASE("On first create project, bus and renderer tracks have master send disabled") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};
    auto project = std::make_shared<NiceMock<MockProjectElement>>();
    auto node = ProjectNode{project};

    auto returnTrackWithPluginExpectations = createTrack([](MockTrack& track){
        EXPECT_CALL(track, disableMasterSend());
    });

    ON_CALL(api, createTrack()).WillByDefault(returnTrackWithPluginExpectations);
    earSuite.onCreateProject(node, api);
}
*/

/*
std::unique_ptr<NiceMock<MockTrack>> getMockTrack(std::string) {
    auto track = std::make_unique<NiceMock<MockTrack>>();
    ON_CALL(*track, stillExists()).WillByDefault(Return(true)); // TODO: How to fix this? track is a unique_ptr so gets moved and "attempting to reference deleted function"
    return track;
};

TEST_CASE("On subsequent create project, scene master and renderer not added") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};
    auto project = std::make_shared<NiceMock<MockProjectElement>>();
    auto node = ProjectNode{project};

    auto returnPersistentSceneMasterTrack = createTrack([](MockTrack& track){
        ON_CALL(track, stillExists()).WillByDefault(Return(true));
        EXPECT_CALL(track, createPlugin(StrEq(SCENEMASTER_NAME)));
    });

    auto returnPersistentRendererTrack = createTrack([](MockTrack& track){
        ON_CALL(track, stillExists()).WillByDefault(Return(true));
        EXPECT_CALL(track, createPlugin(StrEq(RENDERER_NAME)));
    });


    EXPECT_CALL(api, firstTrackWithPluginNamed(StrEq(RENDERER_NAME))).WillOnce(Return(nullptr)).WillRepeatedly(getMockTrack);

    EXPECT_CALL(api, createTrack()).Times(2).WillOnce(returnPersistentSceneMasterTrack).WillOnce(returnPersistentRendererTrack);
    earSuite.onCreateProject(node, api);
    earSuite.onCreateProject(node, api);
}
*/

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

TEST_CASE("Object tracks positioned") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};
    NiceMock<MockTrackElement> trackElement;
    auto track = std::make_shared<NiceMock<MockTrack>>();
    ON_CALL(*track, getPlugin(An<std::string>())).WillByDefault(getPluginStr);
    ON_CALL(*track, getPlugin(An<int>())).WillByDefault(getPluginInt);
    ON_CALL(trackElement, getTrack()).WillByDefault(Return(track));

    SECTION("Before scene master and renderer tracks") {
        int const MEDIA_INDEX{7};
        int const BUS_TRACK_INDEX{MEDIA_INDEX + 2};
        ON_CALL(api, getTrackIndexOfSelectedMediaItem).WillByDefault(Return(MEDIA_INDEX));
        EXPECT_CALL(*track, createPlugin(StrEq("EAR Object"))).WillRepeatedly(createPlugin);
        //EXPECT_CALL(*track, moveToBefore(BUS_TRACK_INDEX));
        initProject(earSuite, api);
        earSuite.onCreateObjectTrack(trackElement, api);
    }

    SECTION("Sequentially") {
      int index{4};
      auto checkIndex = [&index](int secondIndex) {
          REQUIRE(secondIndex == index + 1);
      };

      //EXPECT_CALL(*track, moveToBefore(_)).WillOnce(SaveArg<0>(&index)).WillOnce(checkIndex);
      EXPECT_CALL(*track, createPlugin(StrEq("EAR Object"))).WillRepeatedly(createPlugin);
      initProject(earSuite, api);
      earSuite.onCreateObjectTrack(trackElement, api);
      earSuite.onCreateObjectTrack(trackElement, api);
    }
}

TEST_CASE("Object track has object metadata plugin instantiated") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};
    NiceMock<MockTrackElement> trackElement;
    auto track = std::make_shared<NiceMock<MockTrack>>();
    ON_CALL(trackElement, getTrack()).WillByDefault(Return(track));
    ON_CALL(*track, getPlugin(An<std::string>())).WillByDefault(getPluginStr);
    ON_CALL(*track, getPlugin(An<int>())).WillByDefault(getPluginInt);

    EXPECT_CALL(*track, moveToBefore(_)).Times(AnyNumber());
    EXPECT_CALL(*track, createPlugin(StrEq("EAR Object"))).WillRepeatedly(createPlugin);
    initProject(earSuite, api);
    earSuite.onCreateObjectTrack(trackElement, api);
}
/* TODO - mock UniqueValueAssigner for this to work
TEST_CASE("Object tracks are routed to bus track") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};
    NiceMock<MockTrackElement> trackElement;
    auto track = std::make_shared<NiceMock<MockTrack>>();
    auto objectAuto = getMockObjectAutoElement();
    ON_CALL(trackElement, getTrack()).WillByDefault(Return(track));
    ON_CALL(*track, getPlugin(An<std::string>())).WillByDefault(getPluginStr);
    ON_CALL(*track, getPlugin(An<int>())).WillByDefault(getPluginInt);

    const int FIRST_INDEX = 0;
    EXPECT_CALL(*track, route(_, 1, 0, FIRST_INDEX));
    EXPECT_CALL(*track, createPlugin(StrEq("EAR Object"))).WillRepeatedly(createPlugin);
    initProject(earSuite, api);

    earSuite.onProjectBuildBegin(getGenericMetadata(), api);
    earSuite.onCreateObjectTrack(trackElement, api);
    earSuite.onObjectAutomation(*objectAuto, api);

    SECTION("sequentially") {
      EXPECT_CALL(*track, route(_, 1, 0, FIRST_INDEX + 1));
      earSuite.onCreateObjectTrack(trackElement, api);
      earSuite.onObjectAutomation(*objectAuto, api);
    }
}
*/
TEST_CASE("Object tracks are not sent to master") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};
    NiceMock<MockTrackElement> trackElement;
    auto track = std::make_shared<NiceMock<MockTrack>>();
    ON_CALL(trackElement, getTrack()).WillByDefault(Return(track));
    ON_CALL(*track, getPlugin(An<std::string>())).WillByDefault(getPluginStr);
    ON_CALL(*track, getPlugin(An<int>())).WillByDefault(getPluginInt);
    EXPECT_CALL(*track, createPlugin(StrEq("EAR Object"))).WillRepeatedly(createPlugin);
    EXPECT_CALL(*track, disableMasterSend());
    initProject(earSuite, api);
    earSuite.onCreateObjectTrack(trackElement, api);
}
namespace {
  MATCHER_P(HasParameter, param, "") { return (arg.admParameter() == param); }
  MATCHER_P(HasIndex, index, "")
  {
      return (arg.index() == index);
  }
  } // namespace

  TEST_CASE("Automation is applied for all Automatable Object plugin parameters")
  {
      std::vector<AdmParameter> supportedParameters{{AdmParameter::OBJECT_AZIMUTH,
                                                     AdmParameter::OBJECT_DISTANCE,
                                                     AdmParameter::OBJECT_ELEVATION,
                                                     AdmParameter::OBJECT_GAIN,
                                                     AdmParameter::OBJECT_HEIGHT,
                                                     AdmParameter::OBJECT_WIDTH,
                                                     AdmParameter::OBJECT_DEPTH,
                                                     AdmParameter::OBJECT_DIFFUSE,
//                                                     AdmParameter::OBJECT_DIVERGENCE,
//                                                     AdmParameter::OBJECT_DIVERGENCE_AZIMUTH_RANGE
      }};

      EARPluginSuite earSuite;
      auto api = NiceMock<MockReaperAPI>{};
      auto autoElement = getMockObjectAutoElement();
      for (auto parameter : supportedParameters) {
          EXPECT_CALL(*autoElement, apply(HasParameter(parameter), A<Plugin const &>()));
      }
      initProject(earSuite, api);
      earSuite.onProjectBuildBegin(getGenericMetadata(), api);
      earSuite.onObjectAutomation(*autoElement, api);
  }

  TEST_CASE("Object tracks have trackmapping applied")
  {
      EARPluginSuite earSuite;
      auto api = NiceMock<MockReaperAPI>{};
      NiceMock<MockTrackElement> trackElement;
      auto track = std::make_shared<NiceMock<MockTrack>>();

      ON_CALL(trackElement, getTrack()).WillByDefault(Return(track));

      const double FIRST_INDEX = 1.0 / 64.0; // from 0 to 1 inclusive with 65 steps (-1 to 63), -1 == unmapped so first should be step after 0.
      const int TRACK_MAPPING_PARAMETER = 0;

      auto createPlugin = [=](std::string) {
          auto plugin = std::make_unique<MockPlugin>();
          EXPECT_CALL(*plugin, setParameter(HasIndex(TRACK_MAPPING_PARAMETER), FIRST_INDEX));
          return plugin;
      };
      EXPECT_CALL(*track, createPlugin(StrEq("EAR Object"))).WillRepeatedly(createPlugin);
      initProject(earSuite, api);
      earSuite.onCreateObjectTrack(trackElement, api);

      //    SECTION("sequentially") {
      //      EXPECT_CALL(*track, route(_, 1, 0, FIRST_INDEX + 1));
      //      earSuite.onCreateObjectTrack(trackElement, api);
      //    }
  }

  TEST_CASE("Directspeaker tracks are not sent to master") {
      EARPluginSuite earSuite;
      auto api = NiceMock<MockReaperAPI>{};
      NiceMock<MockTrackElement> trackElement;
      auto track = std::make_shared<NiceMock<MockTrack>>();
      ON_CALL(trackElement, getTrack()).WillByDefault(Return(track));
      ON_CALL(*track, getPlugin(An<std::string>())).WillByDefault(getPluginStr);
      ON_CALL(*track, getPlugin(An<int>())).WillByDefault(getPluginInt);
      EXPECT_CALL(*track, disableMasterSend());
      initProject(earSuite, api);
      earSuite.onCreateDirectTrack(trackElement, api);
  }

  TEST_CASE("Channels Metadata plugin added on first directspeakers automation")
  {
      EARPluginSuite earSuite;
      auto api = NiceMock<MockReaperAPI>{};
      NiceMock<MockDirectSpeakersAutomation> autoElement;
      initProject(earSuite, api);
      auto pf = admCommonDef->lookup(adm::parseAudioPackFormatId("AP_00010001"));
      auto cf = admCommonDef->lookup(adm::parseAudioChannelFormatId("AC_00010003"));
      auto atu = adm::AudioTrackUid::create();
      std::vector<ADMChannel> channels{ ADMChannel(nullptr, cf, pf, atu) };
      auto track = std::make_shared<NiceMock<MockTrack>>();
      ON_CALL(autoElement, getTrack()).WillByDefault(Return(track));
      ON_CALL(autoElement, takeChannels()).WillByDefault(Return(channels));
      ON_CALL(autoElement, channel()).WillByDefault(Return(channels[0]));
      ON_CALL(*track, getPlugin(An<std::string>())).WillByDefault(Return(ByMove(nullptr)));
      ON_CALL(*track, getPlugin(An<int>())).WillByDefault(Return(ByMove(nullptr)));
      EXPECT_CALL(*track, createPlugin(StrEq("EAR DirectSpeakers"))).WillRepeatedly(createPlugin);
      earSuite.onProjectBuildBegin(getGenericMetadata(), api);
      earSuite.onDirectSpeakersAutomation(autoElement, api);
      SECTION("But not if already present")
      {
          auto plugin = std::make_unique<NiceMock<MockPlugin>>();
          ON_CALL(*track, getPlugin(An<std::string>())).WillByDefault(Return(ByMove(std::move(plugin))));
          ON_CALL(*track, getPlugin(An<int>())).WillByDefault(Return(ByMove(std::move(plugin))));
          EXPECT_CALL(*track, createPlugin(_)).Times(0);
          earSuite.onDirectSpeakersAutomation(autoElement, api);
      }
  }

TEST_CASE("Track routed to bus on directspeakers automation") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};
    NiceMock<MockDirectSpeakersAutomation> autoElement;
    initProject(earSuite, api);
    auto pf = admCommonDef->lookup(adm::parseAudioPackFormatId("AP_00010001"));
    auto cf = admCommonDef->lookup(adm::parseAudioChannelFormatId("AC_00010003"));
    auto atu = adm::AudioTrackUid::create();
    std::vector<ADMChannel> channels{ ADMChannel(nullptr, cf, pf, atu) };
    auto track = std::make_shared<NiceMock<MockTrack>>();
    ON_CALL(autoElement, getTrack()).WillByDefault(Return(track));
    ON_CALL(autoElement, takeChannels()).WillByDefault(Return(channels));
    ON_CALL(autoElement, channel()).WillByDefault(Return(channels[0]));
    ON_CALL(*track, createPlugin(StrEq("EAR DirectSpeakers"))).WillByDefault(createPlugin);
    EXPECT_CALL(*track, route(_, _, _, _));
    earSuite.onProjectBuildBegin(getGenericMetadata(), api);
    earSuite.onDirectSpeakersAutomation(autoElement, api);
}

TEST_CASE("Tracks are routed sequentially") {
    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};

    NiceMock<MockDirectSpeakersAutomation> directAuto;
    auto objectAuto = getMockObjectAutoElement();

    initProject(earSuite, api);

    auto pf = admCommonDef->lookup(adm::parseAudioPackFormatId("AP_00010003"));
    std::vector<ADMChannel> directChannels;
    for(auto cf : pf->getReferences<adm::AudioChannelFormat>()) {
        auto atu = adm::AudioTrackUid::create();
        directChannels.push_back(ADMChannel(nullptr, cf, pf, atu));
    }

    auto simpleObj = adm::createSimpleObject("Test");
    std::vector<ADMChannel> objectChannels{ADMChannel(simpleObj.audioObject, simpleObj.audioChannelFormat, simpleObj.audioPackFormat, simpleObj.audioTrackUid)};

    auto objectTrack = std::make_shared<NiceMock<MockTrack>>();
    auto directTrack = std::make_shared<NiceMock<MockTrack>>();

    int OBJECT_CHANNEL_COUNT = objectChannels.size();
    int DIRECT_CHANNEL_COUNT = directChannels.size();

    ON_CALL(directAuto, takeChannels()).WillByDefault(Return(directChannels));
    ON_CALL(directAuto, getTrack()).WillByDefault(Return(directTrack));
    ON_CALL(directAuto, channel()).WillByDefault(Return(directChannels[0]));
    ON_CALL(*objectAuto, takeChannels()).WillByDefault(Return(objectChannels));
    ON_CALL(*objectAuto, getTrack()).WillByDefault(Return(objectTrack));
    ON_CALL(*objectAuto, channel()).WillByDefault(Return(objectChannels[0]));

    ON_CALL(*objectTrack, createPlugin(StrEq("EAR Object"))).WillByDefault(createPlugin);
    ON_CALL(*directTrack, createPlugin(StrEq("EAR DirectSpeakers"))).WillByDefault(createPlugin);

    InSequence();
    earSuite.onProjectBuildBegin(getGenericMetadata(), api);
    EXPECT_CALL(*objectTrack, route(_, _, _, 0));
    //earSuite.onCreateObjectTrack(trackElement, api);
    earSuite.onObjectAutomation(*objectAuto, api);
    EXPECT_CALL(*directTrack, route(_, _, _, OBJECT_CHANNEL_COUNT));
    earSuite.onDirectSpeakersAutomation(directAuto, api);
    EXPECT_CALL(*objectTrack, route(_, _, _, OBJECT_CHANNEL_COUNT + DIRECT_CHANNEL_COUNT));
    //earSuite.onCreateObjectTrack(trackElement, api);
    earSuite.onObjectAutomation(*objectAuto, api);
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

TEST_CASE("Object types create exactly one AudioObject track mapping entry") {

    auto expectedAoMapping = std::vector<uint32_t>(64, 0);
    expectedAoMapping[0] = 0x1001;

    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};
    NiceMock<MockObjectAutomation> autoElement;
    initProject(earSuite, api);

    auto doc = createAdmDocument();
    auto simpleObj = adm::createSimpleObject("Test");
    doc->add(simpleObj.audioObject);
    auto channel = ADMChannel(simpleObj.audioObject, simpleObj.audioChannelFormat, simpleObj.audioPackFormat, simpleObj.audioTrackUid);

    auto track = std::make_shared<NiceMock<MockTrack>>();
    ON_CALL(autoElement, getTrack()).WillByDefault(Return(track));
    ON_CALL(autoElement, takeChannels()).WillByDefault(Return(std::vector<ADMChannel>{ channel }));
    ON_CALL(autoElement, channel()).WillByDefault(Return(channel));
    ON_CALL(*track, createPlugin(An<std::string>())).WillByDefault(createPlugin);
    ON_CALL(*track, getPlugin(An<std::string>())).WillByDefault(Return(ByMove(nullptr)));
    ON_CALL(*track, getPlugin(An<int>())).WillByDefault(Return(ByMove(nullptr)));

    initProject(earSuite, api);
    earSuite.onProjectBuildBegin(getGenericMetadata(), api);
    earSuite.onObjectAutomation(autoElement, api);

    REQUIRE(earSuite.getTrackMappingToAo() == expectedAoMapping);
}

TEST_CASE("DirectSpeakers types create exactly one AudioObject track mapping entry regardless of channel count") {

    auto expectedAoMapping = std::vector<uint32_t>(64, 0);
    expectedAoMapping[0] = 0x1001;

    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};
    NiceMock<MockDirectSpeakersAutomation> autoElement;
    initProject(earSuite, api);

    std::vector<ADMChannel> channels;
    auto doc = createAdmDocument();
    auto obj = createElementsFromCommonDefinition(doc, "AP_00010003"); //5.1
    for(auto uid : obj->getReferences<adm::AudioTrackUid>()) {
        auto pf = uid->getReference<adm::AudioPackFormat>();
        auto tf = uid->getReference<adm::AudioTrackFormat>();
        auto sf = tf->getReference<adm::AudioStreamFormat>();
        auto cf = sf->getReference<adm::AudioChannelFormat>();
        channels.push_back(ADMChannel{obj, cf, pf, uid});
    }

    auto track = std::make_shared<NiceMock<MockTrack>>();
    ON_CALL(autoElement, getTrack()).WillByDefault(Return(track));
    ON_CALL(autoElement, takeChannels()).WillByDefault(Return(channels));
    ON_CALL(autoElement, channel()).WillByDefault(Return(channels[0]));
    ON_CALL(*track, createPlugin(An<std::string>())).WillByDefault(createPlugin);
    ON_CALL(*track, getPlugin(An<std::string>())).WillByDefault(Return(ByMove(nullptr)));
    ON_CALL(*track, getPlugin(An<int>())).WillByDefault(Return(ByMove(nullptr)));

    initProject(earSuite, api);
    earSuite.onProjectBuildBegin(getGenericMetadata(), api);
    earSuite.onDirectSpeakersAutomation(autoElement, api);

    REQUIRE(earSuite.getTrackMappingToAo() == expectedAoMapping);
}

TEST_CASE("HOA types create exactly one AudioObject track mapping entry regardless of channel count") {

    auto expectedAoMapping = std::vector<uint32_t>(64, 0);
    expectedAoMapping[0] = 0x1001;

    EARPluginSuite earSuite;
    auto api = NiceMock<MockReaperAPI>{};
    NiceMock<MockHoaAutomation> autoElement;
    initProject(earSuite, api);

    std::vector<ADMChannel> channels;
    auto doc = createAdmDocument();
    auto obj = createElementsFromCommonDefinition(doc, "AP_00040003"); //3rd Order - nesting PFs
    for(auto uid : obj->getReferences<adm::AudioTrackUid>()) {
        auto pf = uid->getReference<adm::AudioPackFormat>();
        auto tf = uid->getReference<adm::AudioTrackFormat>();
        auto sf = tf->getReference<adm::AudioStreamFormat>();
        auto cf = sf->getReference<adm::AudioChannelFormat>();
        channels.push_back(ADMChannel{obj, cf, pf, uid});
    }

    auto track = std::make_shared<NiceMock<MockTrack>>();
    ON_CALL(autoElement, getTrack()).WillByDefault(Return(track));
    ON_CALL(autoElement, takeChannels()).WillByDefault(Return(channels));
    ON_CALL(autoElement, channel()).WillByDefault(Return(channels[0]));
    ON_CALL(*track, createPlugin(An<std::string>())).WillByDefault(createPlugin);
    ON_CALL(*track, getPlugin(An<std::string>())).WillByDefault(Return(ByMove(nullptr)));
    ON_CALL(*track, getPlugin(An<int>())).WillByDefault(Return(ByMove(nullptr)));

    initProject(earSuite, api);
    earSuite.onProjectBuildBegin(getGenericMetadata(), api);
    earSuite.onHoaAutomation(autoElement, api);

    REQUIRE(earSuite.getTrackMappingToAo() == expectedAoMapping);
}
