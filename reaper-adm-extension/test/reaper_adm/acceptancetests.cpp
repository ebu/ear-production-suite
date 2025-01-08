#include "include_gmock.h"
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
#include "pluginsuite_ear.h"
#include "mocks/pluginsuite.h"
#include "fakeptr.h"
#include <track.h>
#include <helper/container_helpers.hpp>
#include <fstream>
#include <daw_channel_count.h>

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
        ON_CALL(api, GetAppVersion()).WillByDefault(Return("7.0"));
        ON_CALL(api, GetDawChannelCount()).WillByDefault(Return(MAX_DAW_CHANNELS));
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
        ON_CALL(api, TrackFX_AddByActualName(_, StrEq(EAR_SCENE_PLUGIN_NAME), _, _)).WillByDefault(Return(0));
        ON_CALL(api, TrackFX_AddByActualName(_, StrEq(EAR_OBJECT_PLUGIN_NAME), _, _)).WillByDefault(Return(0));
        ON_CALL(api, TrackFX_AddByActualName(_, StrEq(EAR_DEFAULT_MONITORING_PLUGIN_NAME), _, _)).WillByDefault(Return(0));
        ON_CALL(api, TrackFX_AddByActualName(_, StrEq(ADM_VST_PLUGIN_NAME), _, _)).WillByDefault(Return(0));
        ON_CALL(api, TrackFX_AddByActualName(_, StrEq(SPATIALISER_PLUGIN_NAME), _, _)).WillByDefault(Return(1));
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
            EXPECT_CALL(api, TrackFX_AddByActualName(_, _, _, TrackFXAddMode::QueryPresence)).Times(AnyNumber());
            EXPECT_CALL(api, TrackFX_AddByActualName(_, _, _, TrackFXAddMode::CreateIfMissing)).Times(AnyNumber());
            EXPECT_CALL(api, TrackFX_AddByActualName(_, StrEq(EAR_OBJECT_PLUGIN_NAME), _, TrackFXAddMode::CreateNew)).Times(expectedObjectPlugins);
            EXPECT_CALL(api, TrackFX_AddByActualName(fake.trackFor.submix, StrEq(EAR_SCENE_PLUGIN_NAME), _, TrackFXAddMode::CreateNew));
            EXPECT_CALL(api, TrackFX_AddByActualName(fake.trackFor.renderer, StrEq(EAR_DEFAULT_MONITORING_PLUGIN_NAME), _, TrackFXAddMode::CreateNew));
        }

        SECTION("Sets the number of track channels to MAX_DAW_CHANNELS, disables object and scene track master sends") {
            EXPECT_CALL(api, setTrackChannelCount(fake.trackFor.renderer, MAX_DAW_CHANNELS)).Times(1);
            EXPECT_CALL(api, setTrackChannelCount(fake.trackFor.submix, MAX_DAW_CHANNELS)).Times(1);
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
            EXPECT_CALL(api, RouteTrackToTrack(fake.trackFor.submix,_,fake.trackFor.renderer,_, MAX_DAW_CHANNELS,_,_)).Times(1);
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
