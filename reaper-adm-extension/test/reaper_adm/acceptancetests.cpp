#include <gmock/gmock.h>
#include <catch2/catch_all.hpp>
#include <fstream>
#include "reaperapi.h"
#include "tempdir.h"
#include "mocks/reaperapi.h"
#include "mocks/importlistener.h"
#include "mocks/importreporter.h"
#include "admimporter.h"
#include "admmetadata.h"
#include "admvstcontrol.h"
#include "pluginregistry.h"
#include "pluginsuite.h"
#include "pluginsuite_fb360.h"
#include "pluginsuite_ear.h"
#include "mocks/pluginsuite.h"
#include "fakeptr.h"
#include <track.h>
#include <helper/container_helpers.hpp>
#include <fstream>

using namespace admplug;
using ::testing::_;
using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using ::testing::DoubleEq;
using ::testing::StrEq;
using ::testing::StrNe;
using ::testing::NiceMock;
using ::testing::Gt;
using ::testing::Ne;
using ::testing::AnyOf;
using ::testing::InSequence;
using ::testing::DoAll;
using ::testing::SetArgumentPointee;

namespace {
    int constexpr SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX{ 0 };
    int constexpr SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX{ 1 };
    int constexpr SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX{ 2 };
    auto constexpr SPATIALISER_PLUGIN_NAME{ "FB360 Spatialiser (ambiX)" };
    auto constexpr CONTROL_PLUGIN_NAME{ "FB360 Control (ambiX)" };
    int constexpr ADM_VST_COMMANDPORT_PARAMETER_INDEX{ 0 };
    int constexpr ADM_VST_SAMPLESPORT_PARAMETER_INDEX{ 1 };
    int constexpr ADM_VST_INCLUDEINADM_PARAMETER_INDEX{ 2 };
    int constexpr ADM_VST_SAMPLERATE_PARAMETER_INDEX{ 3 };
    int constexpr ADM_VST_NUMCHNS_PARAMETER_INDEX{ 4 };
    int constexpr ADM_VST_ADMTYPEDEFINITION_PARAMETER_INDEX{ 5 };
    int constexpr ADM_VST_ADMPACKFORMAT_PARAMETER_INDEX{ 6 };
    int constexpr ADM_VST_ADMCHANNELFORMAT_PARAMETER_INDEX{ 7 };
    auto constexpr ADM_VST_PLUGIN_NAME{ "ADM Export Source" };
    auto constexpr EAR_SCENE_PLUGIN_NAME{ "EAR Scene" };
    auto constexpr EAR_OBJECT_PLUGIN_NAME{ "EAR Object" };
    auto constexpr EAR_DEFAULT_MONITORING_PLUGIN_NAME{ "EAR Monitoring 0+2+0" };

    struct FakeTracks {
        explicit FakeTracks(FakePtrFactory& fakePtr) :
            renderer{fakePtr.get<MediaTrack>()},
            submix{fakePtr.get<MediaTrack>()},
            directSpeakers{fakePtr.get<MediaTrack>()} {}

        MediaTrack* renderer{};
        MediaTrack* submix{};

        MediaTrack* directSpeakers{}; // Used as a bus of directspeakers channels by FB360
    };

    struct FakeEnvelopes {
        explicit FakeEnvelopes(FakePtrFactory& fakePtr) :
            azimuth{fakePtr.get<TrackEnvelope>()},
            elevation{fakePtr.get<TrackEnvelope>()},
            distance{fakePtr.get<TrackEnvelope>()},
            volume{fakePtr.get<TrackEnvelope>()} {}

        TrackEnvelope* azimuth{};
        TrackEnvelope* elevation{};
        TrackEnvelope* distance{};
        TrackEnvelope* volume{};
    };

    class FakeReaperObjects {
    private:
        FakePtrFactory fakePtr;
    public:
        explicit FakeReaperObjects() : source{fakePtr.get<PCM_source>()},
            mediaItem{fakePtr.get<MediaItem>()},
            take{fakePtr.get<MediaItem_Take>()},
            trackFor{fakePtr},
            envelopeFor{fakePtr} {}
        PCM_source* source;
        MediaItem* mediaItem;
        MediaItem_Take* take;
        FakeTracks trackFor;
        FakeEnvelopes envelopeFor;
        int sendIndex{4};
        int fxIndex{0};
        std::vector<MediaTrack*> tracksWithItems;
        std::vector<MediaTrack*> generatedTracks{
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>(),
            fakePtr.get<MediaTrack>()
        };
        int nextAvailableGeneratedTrack = 0;
        MediaTrack* generateFakeTrack() {
            return generatedTracks[nextAvailableGeneratedTrack++];
        }
    };

    void setApiDefaults(MockReaperAPI& api, FakeReaperObjects& fake) {

        ON_CALL(api, GetResourcePath()).WillByDefault(Return("data"));
        ON_CALL(api, PCM_Source_CreateFromFile(_)).WillByDefault(Return(fake.source));
        ON_CALL(api, AddMediaItemToTrack(_)).WillByDefault([&api, &fake](MediaTrack* tr){
            fake.tracksWithItems.push_back(tr);
            return fake.mediaItem;
        });
        ON_CALL(api, GetTrackNumMediaItems(_)).WillByDefault([&api, &fake](MediaTrack* tr){
            return contains(fake.tracksWithItems, tr);
        });
        ON_CALL(api, AddTakeToMediaItem(fake.mediaItem)).WillByDefault(Return(fake.take));
        ON_CALL(api, getPluginEnvelope(_, _, SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX)).WillByDefault(Return(fake.envelopeFor.azimuth));
        ON_CALL(api, getPluginEnvelope(_, _, SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX)).WillByDefault(Return(fake.envelopeFor.elevation));
        ON_CALL(api, getPluginEnvelope(_, _, SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX)).WillByDefault(Return(fake.envelopeFor.distance));
        ON_CALL(api, SetMediaItemTake_Source(fake.take, fake.source)).WillByDefault(Return(true));
        ON_CALL(api, CreateTrackSend(_, _)).WillByDefault(Return(fake.sendIndex));
        ON_CALL(api, TrackFX_AddByName(_, StrEq(EAR_SCENE_PLUGIN_NAME), _, _)).WillByDefault(Return(0));
        ON_CALL(api, TrackFX_AddByName(_, StrEq(EAR_OBJECT_PLUGIN_NAME), _, _)).WillByDefault(Return(0));
        ON_CALL(api, TrackFX_AddByName(_, StrEq(EAR_DEFAULT_MONITORING_PLUGIN_NAME), _, _)).WillByDefault(Return(0));
        ON_CALL(api, TrackFX_AddByName(_, StrEq(ADM_VST_PLUGIN_NAME), _, _)).WillByDefault(Return(0));
        ON_CALL(api, TrackFX_AddByName(_, StrEq(SPATIALISER_PLUGIN_NAME), _, _)).WillByDefault(Return(1));
        ON_CALL(api, TrackFX_GetByName(_, _, _)).WillByDefault(Return(fake.fxIndex));
        ON_CALL(api, TrackFX_GetCount(_)).WillByDefault(Return(2)); // Object VST and ADM VST
        ON_CALL(api, GetTrackEnvelopeByName(_, StrEq("Volume"))).WillByDefault(Return(fake.envelopeFor.volume));
        ON_CALL(api, CountTracks(_)).WillByDefault(Return(2));
        static GUID guid{ 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
        static GUID fxGuid0{ 1, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
        static GUID fxGuid1{ 1, 0, 0, {0, 0, 0, 0, 0, 0, 0, 1}};
        ON_CALL(api, GetTrackGUID(_)).WillByDefault(Return(&guid));
        // WARNING!!! This presumes new VSTs are only ever appended, not inserted to the chain!!
        ON_CALL(api, TrackFX_GetFXGUID(_, 0)).WillByDefault(Return(&fxGuid0));
        ON_CALL(api, TrackFX_GetFXGUID(_, 1)).WillByDefault(Return(&fxGuid1));
        ON_CALL(api, ValidatePtr(_, _)).WillByDefault(Return(true));
        ON_CALL(api, TrackFX_SetPreset(_, _, _)).WillByDefault(Return(true));
        ON_CALL(api, forceAmplitudeScaling(_)).WillByDefault(Return(true));
        ON_CALL(api, GetTrackName(_, _, _)).WillByDefault(Return(true));

    }


    void setObjectImportExpectations(MockReaperAPI& api, FakeReaperObjects& fake, int expectedGroupTracks, int expectedTracks) {
        ON_CALL(api, createTrack()).WillByDefault([&api](){
            return std::make_unique<TrackInstance>(api.createTrackAtIndex(0, false), api);
        });

        EXPECT_CALL(api, createTrackAtIndex(_, _)).Times(AnyNumber()).
            WillOnce(Return(fake.trackFor.renderer)).
            WillOnce(Return(fake.trackFor.submix)).
            WillRepeatedly([&fake](int index, bool fromEnd) {
            return fake.generateFakeTrack();
        });

        SECTION("Creates a submix, 3D render bus and expected number of Group and Media Tracks") {
            if ((expectedGroupTracks + expectedTracks) > 0) {
                EXPECT_CALL(api, createTrackAtIndex(0, _)).Times(2 + expectedGroupTracks + expectedTracks).
                    WillOnce(Return(fake.trackFor.submix)).
                    WillOnce(Return(fake.trackFor.renderer)).
                    WillRepeatedly([&fake](int index, bool fromEnd) {
                    return fake.generateFakeTrack();
                });
            }
            else {
                EXPECT_CALL(api, createTrackAtIndex(0, _)).Times(2).
                    WillOnce(Return(fake.trackFor.submix)).
                    WillOnce(Return(fake.trackFor.renderer));
            }
        }

        SECTION("Instantiates a spatialiser plugin on the object track and a control plugin on the render bus") {
            EXPECT_CALL(api, TrackFX_AddByName(_, _, _, TrackFXAddMode::QueryPresence)).Times(AnyNumber());
            EXPECT_CALL(api, TrackFX_AddByName(_, StrEq(ADM_VST_PLUGIN_NAME), _, AnyOf(TrackFXAddMode::CreateNew, TrackFXAddMode::CreateIfMissing))).Times(expectedTracks * 2); // Once via MediaTrackElement::addAdmExportVst, again via MediaTakeElement::createProjectElements
            EXPECT_CALL(api, TrackFX_AddByName(_, StrEq(SPATIALISER_PLUGIN_NAME), _, AnyOf(TrackFXAddMode::CreateNew, TrackFXAddMode::CreateIfMissing))).Times(expectedTracks);
            EXPECT_CALL(api, TrackFX_AddByName(fake.trackFor.renderer, StrEq(CONTROL_PLUGIN_NAME), _, AnyOf(TrackFXAddMode::CreateNew, TrackFXAddMode::CreateIfMissing)));
        }

        SECTION("Sets the number of track channels to 16, disables object and submix track master sends") {
            EXPECT_CALL(api, setTrackChannelCount(fake.trackFor.renderer, 16)).Times(1);
            EXPECT_CALL(api, setTrackChannelCount(fake.trackFor.submix, 16)).Times(1);
            for(int i = 0; i < (expectedGroupTracks + expectedTracks); ++i) {
                EXPECT_CALL(api, setTrackChannelCount(fake.generatedTracks[i], 16)).Times(1);
                EXPECT_CALL(api, disableTrackMasterSend(fake.generatedTracks[i])).Times(1);
            }
            EXPECT_CALL(api, disableTrackMasterSend(fake.trackFor.submix)).Times(1);
        }

        SECTION("Adds a 16 channel send from the object track to the submix") {
            EXPECT_CALL(api, RouteTrackToTrack(_,_,_,_,_,_,_)).Times(AnyNumber());
            EXPECT_CALL(api, RouteTrackToTrack(_,_,fake.trackFor.submix,_,16,_,_)).Times(expectedTracks);
        }

        SECTION("Adds audio to the track") {
            EXPECT_CALL(api, AddMediaItemToTrack(_)).Times(expectedTracks);
            EXPECT_CALL(api, AddTakeToMediaItem(fake.mediaItem)).Times(expectedTracks);
            EXPECT_CALL(api, SetMediaItemLength(fake.mediaItem, _, _)).Times(expectedTracks);
            if (expectedTracks > 0) {
                EXPECT_CALL(api, PCM_Source_CreateFromFile(_)).Times(expectedTracks).WillRepeatedly(Return(fake.source));
            }
            else {
                EXPECT_CALL(api, PCM_Source_CreateFromFile(_)).Times(0);
            }
            EXPECT_CALL(api, SetMediaItemTake_Source(fake.take, fake.source)).Times(expectedTracks);
        }
    }

    void setDirectSpeakersImportExpectations(MockReaperAPI& api, FakeReaperObjects& fake, int expectedDirectChannels) {
        ON_CALL(api, createTrack()).WillByDefault([&api](){
            return std::make_unique<TrackInstance>(api.createTrackAtIndex(0, false), api);
        });

        EXPECT_CALL(api, createTrackAtIndex(_, _)).Times(AnyNumber()).
            WillOnce(Return(fake.trackFor.renderer)).
            WillOnce(Return(fake.trackFor.submix)).
            WillOnce(Return(fake.trackFor.directSpeakers)).
            WillRepeatedly([&fake](int index, bool fromEnd) {
            return fake.generateFakeTrack();
        });

        SECTION("Creates a submix, 3D render bus and expected number of Group and Media Tracks") {
            if(expectedDirectChannels > 0) {
                EXPECT_CALL(api, createTrackAtIndex(_, _)).Times(3 + expectedDirectChannels).
                    WillOnce(Return(fake.trackFor.submix)).
                    WillOnce(Return(fake.trackFor.renderer)).
                    WillOnce(Return(fake.trackFor.directSpeakers)).
                    WillRepeatedly([&fake](int index, bool fromEnd) {
                    return fake.generateFakeTrack();
                });
            } else {
                EXPECT_CALL(api, createTrackAtIndex(_, _)).Times(3).
                    WillOnce(Return(fake.trackFor.renderer)).
                    WillOnce(Return(fake.trackFor.submix)).
                    WillOnce(Return(fake.trackFor.directSpeakers));
            }
        }

        SECTION("Instantiates a spatialiser plugin on the channel tracks and a control plugin on the render bus") {
            if(expectedDirectChannels > 0) {
                EXPECT_CALL(api, TrackFX_AddByName(fake.trackFor.directSpeakers, StrEq(ADM_VST_PLUGIN_NAME), false, TrackFXAddMode::QueryPresence)).WillOnce(Return(-1)).WillRepeatedly(Return(0));
                EXPECT_CALL(api, TrackFX_AddByName(_, StrNe(ADM_VST_PLUGIN_NAME), _, TrackFXAddMode::QueryPresence)).Times(AnyNumber());
                EXPECT_CALL(api, TrackFX_AddByName(fake.trackFor.directSpeakers, StrEq(ADM_VST_PLUGIN_NAME), _, AnyOf(TrackFXAddMode::CreateNew, TrackFXAddMode::CreateIfMissing))).Times(1);
            }
            EXPECT_CALL(api, TrackFX_AddByName(_, StrEq(SPATIALISER_PLUGIN_NAME) ,_ ,AnyOf(TrackFXAddMode::CreateNew, TrackFXAddMode::CreateIfMissing))).Times(expectedDirectChannels);
            EXPECT_CALL(api, TrackFX_AddByName(fake.trackFor.renderer, StrEq(CONTROL_PLUGIN_NAME), _, AnyOf(TrackFXAddMode::CreateNew, TrackFXAddMode::CreateIfMissing)));
        }

        SECTION("Sets the number of track channels to 16, disables directspeaker, common definition and submix track master sends") {
            EXPECT_CALL(api, setTrackChannelCount(fake.trackFor.renderer, 16)).Times(1);
            EXPECT_CALL(api, setTrackChannelCount(fake.trackFor.submix, 16)).Times(1);
            if(expectedDirectChannels > 0) {
                EXPECT_CALL(api, setTrackChannelCount(fake.trackFor.directSpeakers, expectedDirectChannels)).Times(AtLeast(1));
            } else {
                EXPECT_CALL(api, setTrackChannelCount(fake.trackFor.directSpeakers, _)).Times(AtLeast(1));
            }
            for(int i = 0; i < expectedDirectChannels; ++i) {
                EXPECT_CALL(api, setTrackChannelCount(fake.generatedTracks[i], 16)).Times(1);
                EXPECT_CALL(api, disableTrackMasterSend(fake.generatedTracks[i])).Times(1);
            }
            EXPECT_CALL(api, disableTrackMasterSend(fake.trackFor.directSpeakers)).Times(1);
            EXPECT_CALL(api, disableTrackMasterSend(fake.trackFor.submix)).Times(1);
        }

        SECTION("Sets sends from direct track to channels") {
            EXPECT_CALL(api, RouteTrackToTrack(_, _, _, _, _, _,_)).Times(AnyNumber());
            EXPECT_CALL(api, RouteTrackToTrack(fake.trackFor.directSpeakers, _, _, _, 1, _, _)).Times(expectedDirectChannels);
            EXPECT_CALL(api, RouteTrackToTrack(_, _, fake.trackFor.submix, _, 16, _, _)).Times(expectedDirectChannels);
        }

        SECTION("Adds audio to the track") {
            EXPECT_CALL(api, AddMediaItemToTrack(fake.trackFor.directSpeakers)).Times(1);
            EXPECT_CALL(api, AddTakeToMediaItem(fake.mediaItem)).Times(1);
            EXPECT_CALL(api, SetMediaItemLength(fake.mediaItem, _, _)).Times(1);
            EXPECT_CALL(api, PCM_Source_CreateFromFile(_)).Times(1).WillRepeatedly(Return(fake.source));
            EXPECT_CALL(api, SetMediaItemTake_Source(fake.take, fake.source)).Times(1);
        }
    }

    void setHoaImportExpectations(MockReaperAPI& api, FakeReaperObjects& fake, int expectedHoaObjects){
        ON_CALL(api, createTrack()).WillByDefault([&api](){
            return std::make_unique<TrackInstance>(api.createTrackAtIndex(0, false), api);
        });

        EXPECT_CALL(api, createTrackAtIndex(_, _)).Times(2 + expectedHoaObjects).
            WillOnce(Return(fake.trackFor.renderer)).
            WillOnce(Return(fake.trackFor.submix)).
            WillRepeatedly([&fake](int index, bool fromEnd) {
            return fake.generateFakeTrack();
        });

        EXPECT_CALL(api, GetResourcePath()).WillRepeatedly(Return("../../test/reaper_adm/data"));
        EXPECT_CALL(api, TrackFX_AddByName(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(api, TrackFX_AddByName(_, StrEq(SPATIALISER_PLUGIN_NAME), false, 0)).WillOnce(Return(1)).WillRepeatedly(Return(-1));
        EXPECT_CALL(api, TrackFX_AddByName(_, StrEq(ADM_VST_PLUGIN_NAME), false, 0)).WillOnce(Return(0)).WillRepeatedly(Return(-1));
        char* notNothing = "not_nothing";
        char* isNothing = "";
        EXPECT_CALL(api, TrackFX_GetPreset(_, 1, _, _)).WillOnce(DoAll(SetArgumentPointee<2>(*isNothing), Return(true))).WillRepeatedly(DoAll(SetArgumentPointee<2>(*notNothing), Return(true)));
        EXPECT_CALL(api, TrackFX_SetPreset(_, _, _)).Times(expectedHoaObjects);
    }

    template<typename T>
    void doImport(std::string fileName, MockReaperAPI const& api) {
      auto fakePluginSuite = std::make_shared<T>();
      test::TempDir dir;
      auto tempPath = dir.path();
      {
        std::ifstream file(fileName);
        REQUIRE(file.good());
      }
      ADMImporter importer(nullptr,
                           fileName,
                           ImportContext{
                               std::make_unique<NiceMock<MockImportListener>>(),
                               std::make_unique<NiceMock<MockImportReporter>>(),
                               fakePluginSuite,
                               api},
                           tempPath.string());
      importer.parse();
      importer.extractAudio();
      importer.buildProject();
    }

    void setEarObjectImportExpectations(MockReaperAPI& api, FakeReaperObjects& fake, int expectedTracks, int expectedPCMSources, int expectedObjectPlugins) {
        ON_CALL(api, createTrack()).WillByDefault([&api](){
            return std::make_unique<TrackInstance>(api.createTrackAtIndex(0, false), api);
        });

        EXPECT_CALL(api, createTrackAtIndex(_, _)).Times(AnyNumber()).
        WillOnce(Return(fake.trackFor.submix)).
        WillOnce(Return(fake.trackFor.renderer)).
            WillRepeatedly([&fake](int index, bool fromEnd) {
            return fake.generateFakeTrack();
        });

        SECTION("Creates a scene bus, monitoring bus and expected number of object tracks") {
            if (expectedTracks > 0) {
                EXPECT_CALL(api, createTrackAtIndex(0, _)).Times(2 + expectedTracks).
                    WillOnce(Return(fake.trackFor.submix)).
                    WillOnce(Return(fake.trackFor.renderer)).
                    WillRepeatedly([&fake](int index, bool fromEnd) {
                    return fake.generateFakeTrack();
                });
            }
            else {
                EXPECT_CALL(api, createTrackAtIndex(0, _)).Times(2).
                    WillOnce(Return(fake.trackFor.submix)).
                    WillOnce(Return(fake.trackFor.renderer));
            }
        }

        SECTION("Instantiates a spatialiser plugin on the object track and a control plugin on the render bus") {
            EXPECT_CALL(api, TrackFX_AddByName(_, _, _, TrackFXAddMode::QueryPresence)).Times(AnyNumber());
            EXPECT_CALL(api, TrackFX_AddByName(_, _, _, TrackFXAddMode::CreateIfMissing)).Times(AnyNumber());
            EXPECT_CALL(api, TrackFX_AddByName(_, StrEq(EAR_OBJECT_PLUGIN_NAME), _, TrackFXAddMode::CreateNew)).Times(expectedObjectPlugins);
            EXPECT_CALL(api, TrackFX_AddByName(fake.trackFor.submix, StrEq(EAR_SCENE_PLUGIN_NAME), _, TrackFXAddMode::CreateNew));
            EXPECT_CALL(api, TrackFX_AddByName(fake.trackFor.renderer, StrEq(EAR_DEFAULT_MONITORING_PLUGIN_NAME), _, TrackFXAddMode::CreateNew));
        }

        SECTION("Sets the number of track channels to 64, disables object and scene track master sends") {
            EXPECT_CALL(api, setTrackChannelCount(fake.trackFor.renderer, 64)).Times(1);
            EXPECT_CALL(api, setTrackChannelCount(fake.trackFor.submix, 64)).Times(1);
            for(int i = 0; i < expectedTracks; ++i) {
                EXPECT_CALL(api, setTrackChannelCount(fake.generatedTracks[i], _)).Times(1);
                EXPECT_CALL(api, disableTrackMasterSend(fake.generatedTracks[i])).Times(1);
            }
            EXPECT_CALL(api, disableTrackMasterSend(fake.trackFor.submix)).Times(1);
        }

        SECTION("Adds a send from the object track to the scene, and to the monitoring") {
            EXPECT_CALL(api, RouteTrackToTrack(_,_,_,_,_,_,_)).Times(AnyNumber());
            for(int i = 0; i < expectedTracks; ++i) {
                EXPECT_CALL(api, RouteTrackToTrack(fake.generatedTracks[i],_,fake.trackFor.submix,_,_,_,_)).Times(1);
            }
            EXPECT_CALL(api, RouteTrackToTrack(fake.trackFor.submix,_,fake.trackFor.renderer,_,64,_,_)).Times(1);
        }

        SECTION("Adds audio to the track") {
            for(int i = 0; i < expectedTracks; ++i) {
                EXPECT_CALL(api, AddMediaItemToTrack(fake.generatedTracks[i])).Times(1);
            }
            EXPECT_CALL(api, AddTakeToMediaItem(fake.mediaItem)).Times(expectedTracks);
            EXPECT_CALL(api, SetMediaItemLength(fake.mediaItem, _, _)).Times(expectedTracks);
            if (expectedPCMSources > 0) {
                EXPECT_CALL(api, PCM_Source_CreateFromFile(_)).Times(expectedPCMSources).WillRepeatedly(Return(fake.source));
            }
            else {
                EXPECT_CALL(api, PCM_Source_CreateFromFile(_)).Times(0);
            }
            EXPECT_CALL(api, SetMediaItemTake_Source(fake.take, fake.source)).Times(expectedTracks);
        }

    }

}

TEST_CASE("Check required plug-ins are available") {
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;
    setApiDefaults(api, fake);
    auto registry = PluginRegistry::getInstance();

    // This is partly a test for the tests
    // - it makes sure the dummy reaper-vstplugins64.ini data includes all the plug-ins we will need.
    //   (otherwise it can be difficult to track down why tests are failing)
    std::vector<std::string> reqPlugins{
        "FB360 Control (ambiX)",
        "FB360 Converter (ambiX)",
        "FB360 Spatialiser (ambiX)",
        "VISR ObjectEditor",
        "VISR SceneMaster",
        "VISR LoudspeakerRenderer",
        "ADM Export Source"
    };
    ASSERT_TRUE(registry->checkPluginsAvailable(reqPlugins, api));

    // This makes sure the registry isn't just saying "yes" to all (and also that the old ADM Stem is removed)
    ASSERT_FALSE(registry->checkPluginAvailable("ADM Stem", api));

}

TEST_CASE("Import object based panning noise using FB360 plugin suite", "[objects][FB360]") {
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    setObjectImportExpectations(api, fake, 0, 1);

    SECTION("Names the render bus 3D MASTER and the MediaTrack after the AudioObject") {
        EXPECT_CALL(api, setTrackName(fake.trackFor.renderer, StrEq("3D MASTER"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.submix, StrEq("3D Sub-Master"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.generatedTracks[0], StrEq("Noise"))).Times(1);
    }

    SECTION("Adds automation envelopes for object azimuth, elevation distance and volume") {
        EXPECT_CALL(api, getPluginEnvelope(fake.generatedTracks[0], _, SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX)).Times((AtLeast(1)));
        // only 1 point so set directly
        EXPECT_CALL(api, getPluginEnvelope(fake.generatedTracks[0], _, SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX)).Times(0);
        EXPECT_CALL(api, getPluginEnvelope(fake.generatedTracks[0], _, SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX)).Times(0);
        EXPECT_CALL(api, GetTrackEnvelopeByName(fake.generatedTracks[0], StrEq("Volume"))).Times(0);
    }

    SECTION("Adds automation points to the envelopes (24 direct, 1 for start of first block as it has non-zero duration, 1 for handling azimuth discontinuity)") {
        int const numBlocks = 24;
        int const azimuthDiscontinuityPoints = 1; // Was previously 2 - new logic doesn't add ineffective point (duplicate of point data derived from audioBlockFormat)
        EXPECT_CALL(api, InsertEnvelopePoint(fake.envelopeFor.azimuth, _, _, _, _, _, _)).Times(numBlocks*2 + azimuthDiscontinuityPoints - 1); // Final point becomes redundant
        EXPECT_CALL(api, InsertEnvelopePoint(fake.envelopeFor.elevation, _, _, _, _, _, _)).Times(0); // Elevation never changes
        EXPECT_CALL(api, InsertEnvelopePoint(fake.envelopeFor.distance, _, _, _, _, _, _)).Times(0); // Distance never changes
        EXPECT_CALL(api, InsertEnvelopePoint(fake.envelopeFor.volume, _, _, _, _, _, _)).Times(0); // Gain never changes
    }

    SECTION("Sets plugin values for azimuth elevation, distance and track volume") {
        EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[0], 1, SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX, _)).Times(AtLeast(1));
        EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[0], 1, SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX, _)).Times(AtLeast(1));
        EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[0], 1, SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX, _)).Times(AtLeast(1));
        EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[0], 0, ADM_VST_INCLUDEINADM_PARAMETER_INDEX, _)).Times(AnyNumber());
        EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[0], 0, ADM_VST_ADMTYPEDEFINITION_PARAMETER_INDEX, _)).Times(AnyNumber());
        EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[0], 0, ADM_VST_ADMPACKFORMAT_PARAMETER_INDEX, _)).Times(AnyNumber());
        EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[0], 0, ADM_VST_ADMCHANNELFORMAT_PARAMETER_INDEX, _)).Times(AnyNumber());
    }

    doImport<Facebook360PluginSuite>("data/panned_noise_adm.wav", api);
}

TEST_CASE("Import of nested pack formats of object type", "[objects][FB360][nesting]") {
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    setObjectImportExpectations(api, fake, 1, 2);

    SECTION("Names the render bus 3D MASTER and the MediaTracks after the AudioObject") {
        EXPECT_CALL(api, setTrackName(fake.trackFor.renderer, StrEq("3D MASTER"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.submix, StrEq("3D Sub-Master"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("audObj"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("2nd CF (child pack)"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("1st CF (parent pack)"))).Times(1);
    }

    SECTION("Sets Azimuth, Elevation, distance and gain parameters") {
        EXPECT_CALL(api, SetMediaTrackInfo_Value(_, _, _)).Times(AnyNumber());
        for(int i = 1; i < 3; i++) {
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_INCLUDEINADM_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMTYPEDEFINITION_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMPACKFORMAT_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMCHANNELFORMAT_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, SetMediaTrackInfo_Value(fake.generatedTracks[i], StrEq("D_VOL"), _)).Times(1);
        }
    }

    doImport<Facebook360PluginSuite>("data/nesting1-1ao_with_1pf_reffing_1pf.wav", api);
}

TEST_CASE("Import of audioobject referencing multiple pack formats of object type", "[objects][FB360][invalid]") {
    // This is valid ADM but not compliant with EBU 3392 due to multiple packs referenced from an object.

    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    setObjectImportExpectations(api, fake, 0, 2);

    SECTION("Names the render bus 3D MASTER and the MediaTracks after the AudioObject") {
        EXPECT_CALL(api, setTrackName(fake.trackFor.renderer, StrEq("3D MASTER"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.submix, StrEq("3D Sub-Master"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.generatedTracks[0], StrEq("audObj"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.generatedTracks[1], StrEq("audObj"))).Times(1);
    }

    SECTION("Sets Azimuth, Elevation, distance and gain parameters") {
        EXPECT_CALL(api, SetMediaTrackInfo_Value(_, _, _)).Times(AnyNumber());
        for(int i = 0; i < 2; i++) {
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_INCLUDEINADM_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMTYPEDEFINITION_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMPACKFORMAT_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMCHANNELFORMAT_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, SetMediaTrackInfo_Value(fake.generatedTracks[i], StrEq("D_VOL"), _)).Times(1);
        }
    }

    doImport<Facebook360PluginSuite>("data/nesting2-1ao_with_2pf_each_with_1cf.wav", api);
}

TEST_CASE("Import of nested audioobjects referencing object type packs", "[objects][FB360][nesting]") {
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    setObjectImportExpectations(api, fake, 0, 2);

    SECTION("Names the render bus 3D MASTER and the MediaTracks after the AudioObjects") {
        EXPECT_CALL(api, setTrackName(fake.trackFor.renderer, StrEq("3D MASTER"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.submix, StrEq("3D Sub-Master"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("Child AO 1"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("Child AO 2"))).Times(1);
    }

    SECTION("Sets Azimuth, Elevation, distance and gain parameters") {
        EXPECT_CALL(api, SetMediaTrackInfo_Value(_, _, _)).Times(AnyNumber());
        for(int i = 0; i < 2; i++) {
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_INCLUDEINADM_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMTYPEDEFINITION_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMPACKFORMAT_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMCHANNELFORMAT_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, SetMediaTrackInfo_Value(fake.generatedTracks[i], StrEq("D_VOL"), _)).Times(1);
        }
    }

    doImport<Facebook360PluginSuite>("data/nesting3-1ao_reffing_2ao_each_with_1pf_with_1cf.wav", api);
}

TEST_CASE("On import of two audioobjects of object type, one which references the other, ADMImporter") {
    // This is valid ADM but not compliant with EBU 3392 due to both pack and object referenced from an object.

    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    setObjectImportExpectations(api, fake, 0, 2);

    SECTION("Names the render bus 3D MASTER and the MediaTracks after the AudioObjects") {
        EXPECT_CALL(api, setTrackName(fake.trackFor.renderer, StrEq("3D MASTER"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.submix, StrEq("3D Sub-Master"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("Parent AO"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("Child AO"))).Times(1);
    }

    SECTION("Sets Azimuth, Elevation, distance and gain parameters") {
        EXPECT_CALL(api, SetMediaTrackInfo_Value(_, _, _)).Times(AnyNumber());
        for(int i = 0; i < 2; i++) {
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_INCLUDEINADM_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMTYPEDEFINITION_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMPACKFORMAT_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMCHANNELFORMAT_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, SetMediaTrackInfo_Value(fake.generatedTracks[i], StrEq("D_VOL"), _)).Times(1);
        }
    }

    doImport<Facebook360PluginSuite>("data/nesting4-1ao_reffing_1ao_both_with_1pf_with_1cf.wav", api);
}

TEST_CASE("Import object packformat with multiple channelformats", "[objects][FB360][nesting]") {
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    setObjectImportExpectations(api, fake, 1, 2);

    SECTION("Names the render bus 3D MASTER and the MediaTracks after the AudioObject") {
        EXPECT_CALL(api, setTrackName(fake.trackFor.renderer, StrEq("3D MASTER"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.submix, StrEq("3D Sub-Master"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("audObj"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("1st CF"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("2nd CF"))).Times(1);
    }

    SECTION("Sets Azimuth, Elevation, distance and gain parameters") {
        EXPECT_CALL(api, SetMediaTrackInfo_Value(_, _, _)).Times(AnyNumber());
        for(int i = 1; i < 3; i++) {
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_INCLUDEINADM_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMTYPEDEFINITION_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMPACKFORMAT_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMCHANNELFORMAT_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, SetMediaTrackInfo_Value(fake.generatedTracks[i], StrEq("D_VOL"), _)).Times(1);
        }
    }

    doImport<Facebook360PluginSuite>("data/nesting5-1ao_with_1pf_with_2cf.wav", api);
}

TEST_CASE("Importing audioobject with shared child object", "[objects][FB360]") {
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    setObjectImportExpectations(api, fake, 2, 1);

    SECTION("Names the render bus 3D MASTER and the MediaTracks after the AudioObject") {
        EXPECT_CALL(api, setTrackName(fake.trackFor.renderer, StrEq("3D MASTER"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.submix, StrEq("3D Sub-Master"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("Parent AO 1"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("Parent AO 2"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("Child AO"))).Times(1);
    }

    SECTION("Sets Azimuth, Elevation, distance and gain parameters") {
        EXPECT_CALL(api, TrackFX_SetParam(_, 1, SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX, _)).Times(1);
        EXPECT_CALL(api, TrackFX_SetParam(_, 1, SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX, _)).Times(1);
        EXPECT_CALL(api, TrackFX_SetParam(_, 1, SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX, _)).Times(1);
        EXPECT_CALL(api, TrackFX_SetParam(_, 0, ADM_VST_INCLUDEINADM_PARAMETER_INDEX, _)).Times(1);
        EXPECT_CALL(api, TrackFX_SetParam(_, 0, ADM_VST_ADMTYPEDEFINITION_PARAMETER_INDEX, _)).Times(1);
        EXPECT_CALL(api, TrackFX_SetParam(_, 0, ADM_VST_ADMPACKFORMAT_PARAMETER_INDEX, _)).Times(1);
        EXPECT_CALL(api, TrackFX_SetParam(_, 0, ADM_VST_ADMCHANNELFORMAT_PARAMETER_INDEX, _)).Times(1);

        EXPECT_CALL(api, SetMediaTrackInfo_Value(_, _, _)).Times(AnyNumber());
        EXPECT_CALL(api, SetMediaTrackInfo_Value(_, StrEq("D_VOL"), _)).Times(1);
    }

    doImport<Facebook360PluginSuite>("data/nesting6-2ao_both_reffing_1ao_with_1pf_with_1cf.wav", api);
}

TEST_CASE("Import of complex audioobject nesting: nesting6b-2ao_parents_1ao_child_shared_and_1ao_child_with_one_parent.wav", "[objects][FB360][nesting]") {
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    setObjectImportExpectations(api, fake, 2, 2);

    SECTION("Names the render bus 3D MASTER and the MediaTracks after the AudioObject") {
        EXPECT_CALL(api, setTrackName(fake.trackFor.renderer, StrEq("3D MASTER"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.submix, StrEq("3D Sub-Master"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("Parent AO 1"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("Parent AO 2"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("Shared Child AO"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("Child of AO 2"))).Times(1);
    }

    SECTION("Sets Azimuth, Elevation, distance and gain parameters") {
        EXPECT_CALL(api, TrackFX_SetParam(_, 1, SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX, _)).Times(2);
        EXPECT_CALL(api, TrackFX_SetParam(_, 1, SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX, _)).Times(2);
        EXPECT_CALL(api, TrackFX_SetParam(_, 1, SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX, _)).Times(2);
        EXPECT_CALL(api, TrackFX_SetParam(_, 0, ADM_VST_INCLUDEINADM_PARAMETER_INDEX, _)).Times(2);
        EXPECT_CALL(api, TrackFX_SetParam(_, 0, ADM_VST_ADMTYPEDEFINITION_PARAMETER_INDEX, _)).Times(2);
        EXPECT_CALL(api, TrackFX_SetParam(_, 0, ADM_VST_ADMPACKFORMAT_PARAMETER_INDEX, _)).Times(2);
        EXPECT_CALL(api, TrackFX_SetParam(_, 0, ADM_VST_ADMCHANNELFORMAT_PARAMETER_INDEX, _)).Times(2);

        EXPECT_CALL(api, SetMediaTrackInfo_Value(_, _, _)).Times(AnyNumber());
        EXPECT_CALL(api, SetMediaTrackInfo_Value(_, StrEq("D_VOL"), _)).Times(2);
    }

    doImport<Facebook360PluginSuite>("data/nesting6b-2ao_parents_1ao_child_shared_and_1ao_child_with_one_parent.wav", api);
}

TEST_CASE("Import of file with objects referencing different packs but common channelformat", "[objects][FB360][nesting]") {
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    setObjectImportExpectations(api, fake, 0, 2);

    SECTION("Names the render bus 3D MASTER and the MediaTracks after the AudioObject") {
        EXPECT_CALL(api, setTrackName(fake.trackFor.renderer, StrEq("3D MASTER"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.submix, StrEq("3D Sub-Master"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("1st AO"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("2nd AO"))).Times(1);
    }

    SECTION("Sets Azimuth, Elevation, distance and gain parameters") {
        EXPECT_CALL(api, SetMediaTrackInfo_Value(_, _, _)).Times(AnyNumber());
        for(int i = 0; i < 2; i++) {
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_INCLUDEINADM_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMTYPEDEFINITION_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMPACKFORMAT_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMCHANNELFORMAT_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, SetMediaTrackInfo_Value(fake.generatedTracks[i], StrEq("D_VOL"), _)).Times(1);
        }
    }

    doImport<Facebook360PluginSuite>("data/nesting7-2ao_with_1pf_each_reffing_same_1cf.wav", api);
}

TEST_CASE("On import of nesting example: nesting8-2ao_reffing_same_1pf_with_1cf.wav, ADMImporter") {
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    setObjectImportExpectations(api, fake, 0, 2);

    SECTION("Names the render bus 3D MASTER and the MediaTracks after the AudioObject") {
        EXPECT_CALL(api, setTrackName(fake.trackFor.renderer, StrEq("3D MASTER"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.submix, StrEq("3D Sub-Master"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("1st AO"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("2nd AO"))).Times(1);
    }

    SECTION("Sets Azimuth, Elevation, distance and gain parameters") {
        EXPECT_CALL(api, SetMediaTrackInfo_Value(_, _, _)).Times(AnyNumber());
        for(int i = 0; i < 2; i++) {
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 1, SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_INCLUDEINADM_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMTYPEDEFINITION_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMPACKFORMAT_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, TrackFX_SetParam(fake.generatedTracks[i], 0, ADM_VST_ADMCHANNELFORMAT_PARAMETER_INDEX, _)).Times(1);
            EXPECT_CALL(api, SetMediaTrackInfo_Value(fake.generatedTracks[i], StrEq("D_VOL"), _)).Times(1);
        }
    }

    doImport<Facebook360PluginSuite>("data/nesting8-2ao_reffing_same_1pf_with_1cf.wav", api);
}

TEST_CASE("Importing mono directspeaker file to FB360 plugin suite", "[directspeaker][FB360]") {
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    setDirectSpeakersImportExpectations(api, fake, 1);

    SECTION("Names the render bus 3D MASTER and the MediaTracks after the AudioObject and Speaker Channels") {
        EXPECT_CALL(api, setTrackName(fake.trackFor.renderer, StrEq("3D MASTER"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.submix, StrEq("3D Sub-Master"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.directSpeakers, StrEq("Main"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.generatedTracks[0], StrEq("FrontCentre"))).Times(1);
   }
    doImport<Facebook360PluginSuite>("data/channels_mono_adm.wav", api);
}

TEST_CASE("Importing stereo directspeaker file to FB360 plugin suite", "[directspeaker][FB360]") {
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    setDirectSpeakersImportExpectations(api, fake, 2);

    SECTION("Names the render bus 3D MASTER and the MediaTracks after the AudioObject and Speaker Channels") {
        EXPECT_CALL(api, setTrackName(fake.trackFor.renderer, StrEq("3D MASTER"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.submix, StrEq("3D Sub-Master"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.directSpeakers, StrEq("Main"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("FrontLeft"))).Times(1);
        EXPECT_CALL(api, setTrackName(_, StrEq("FrontRight"))).Times(1);
    }
    doImport<Facebook360PluginSuite>("data/channels_stereo_adm.wav", api);
}

TEST_CASE("On import of directspeakers file with nested pack formats", "[directspeaker][FB360][nesting]") {
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    setDirectSpeakersImportExpectations(api, fake, 0); // Note that this is not a common def DS pack, so we don't support it! - I.e, 0 direct channel tracks

    SECTION("Names the render bus 3D MASTER and the MediaTrack after the AudioObject. Does not create non-common-def Speaker Channels") {
        EXPECT_CALL(api, setTrackName(fake.trackFor.renderer, StrEq("3D MASTER"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.submix, StrEq("3D Sub-Master"))).Times(1);
        EXPECT_CALL(api, setTrackName(fake.trackFor.directSpeakers, StrEq("audObj"))).Times(1);
    }
    doImport<Facebook360PluginSuite>("data/nesting1b-same_for_directspeakers.wav", api);
}

TEST_CASE("Importing ambix1stOrder HOA file to FB360 plugin suite", "[hoa][FB360]"){
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    setHoaImportExpectations(api, fake, 1);
    EXPECT_CALL(api, TrackFX_Delete(_, 0)).WillOnce(Return(true)); // We expect the export vst to be moved because it's not a common def

    doImport<Facebook360PluginSuite>("data/hoa_4ch_bwf_1stOrderAmbix.wav", api);
}

TEST_CASE("On import of AudioObject with multiple TrackUIDs, creates one track per TrackUID", "[ear]") {
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    ON_CALL(api, CountTracks(_)).WillByDefault(Return(0)); // EARs UniqueValueAssigner will attempt to iterate through existing tracks
    ON_CALL(api, TrackFX_GetCount(_)).WillByDefault(Return(1));
    setEarObjectImportExpectations(api, fake, 3, 3, 3);

    doImport<EARPluginSuite>("data/one_ao-multiple_uid.wav", api);
}

TEST_CASE("On import of three AudioObjects with shared TrackUIDs, creates one track for all three AudioObjects", "[ear]") {
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    ON_CALL(api, CountTracks(_)).WillByDefault(Return(0)); // EARs UniqueValueAssigner will attempt to iterate through existing tracks
    ON_CALL(api, TrackFX_GetCount(_)).WillByDefault(Return(1));
    setEarObjectImportExpectations(api, fake, 1, 1, 3);

    doImport<EARPluginSuite>("data/three_ao-one_atu.wav", api);
}

TEST_CASE("On import of three AudioObjects with shared TrackUIDs and some orphaned, still creates one track per AudioObject", "[ear]") {
    NiceMock<MockReaperAPI> api;
    FakeReaperObjects fake;

    setApiDefaults(api, fake);
    ON_CALL(api, CountTracks(_)).WillByDefault(Return(0)); // EARs UniqueValueAssigner will attempt to iterate through existing tracks
    ON_CALL(api, TrackFX_GetCount(_)).WillByDefault(Return(1));
    setEarObjectImportExpectations(api, fake, 1, 1, 3);

    doImport<EARPluginSuite>("data/aos_sharing_atu-some_without_parent.wav", api);
}
