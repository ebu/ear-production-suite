#include <functional>
#include <catch2/catch.hpp>
#include <gmock/gmock.h>
#include <adm/elements.hpp>
#include "pluginsuite.h"
#include "projectnode.h"
#include "blockbuilders.h"
#include "mocks/reaperapi.h"
#include "mocks/automationenvelope.h"
#include "mocks/projectelements.h"
#include "pluginsuite_fb360.h"
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

TEST_CASE("Facebook360 plugin suite") {
    auto const SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX{ 0 };
    auto const SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX{ 1 };
    auto const SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX{ 2 };

    Facebook360PluginSuite pluginSuite;
    NiceMock<MockReaperAPI> api;

    SECTION("onTrackCreate()") {
        NiceMock<MockTrackElement> fakeElement;
        auto fakeTrack = std::make_shared<NiceMock<MockTrack>>();
        auto fakeTrackPtrVal{34};
        auto fakeTrackPtr = reinterpret_cast<MediaTrack*>(&fakeTrackPtrVal);
        static GUID guid{ 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
        ON_CALL(api, GetTrackGUID(_)).WillByDefault(Return(&guid));
        ON_CALL(api, TrackFX_GetFXGUID(_, _)).WillByDefault(Return(&guid));
        ON_CALL(api, ValidatePtr(_, _)).WillByDefault(Return(true));
        ON_CALL(fakeElement, getTrack()).WillByDefault(Return(fakeTrack));
        ON_CALL(*fakeTrack, get()).WillByDefault(Return(fakeTrackPtr));


        SECTION("suite sets track channel count to 16") {
            EXPECT_CALL(*fakeTrack, setChannelCount(16));
            SECTION("and track master send disabled") {
                EXPECT_CALL(*fakeTrack, disableMasterSend());
            }
        }
        SECTION("suite instatiates spatializer plugin") {
            EXPECT_CALL(*fakeTrack, createPlugin(StrEq("FB360 Spatialiser (ambiX)")));
        }
//        SECTION("suite activates track volume envelope") {
//            EXPECT_CALL(api, activateAndShowTrackVolumeEnvelope(fakeTrackPtr));
//        }
        pluginSuite.onCreateObjectTrack(fakeElement, api);
    }

    SECTION("onProjectInit()") {
        auto fakeRootElement = std::make_shared< NiceMock<MockRootElement>>();
        auto mockControlTrackPtr = std::make_unique<NiceMock<MockTrack>>();
        auto& mockControlTrack = *mockControlTrackPtr;
        auto mockSubmixTrackPtr = std::make_unique<NiceMock<MockTrack>>();
        auto& mockSubmixTrack = *mockSubmixTrackPtr;
        EXPECT_CALL(api, createTrack()).Times(2).WillOnce(Return(ByMove(std::move(mockControlTrackPtr)))).
            WillOnce(Return(ByMove(std::move(mockSubmixTrackPtr))));

        SECTION("3D Mix bus setup") {
            EXPECT_CALL(mockControlTrack, setChannelCount(16));
            EXPECT_CALL(mockControlTrack, setName(StrEq("3D MASTER")));
            EXPECT_CALL(mockControlTrack, createPlugin(StrEq("FB360 Control (ambiX)")));
        }

        SECTION("Submix bus setup") {
            EXPECT_CALL(mockSubmixTrack, setChannelCount(16));
            EXPECT_CALL(mockSubmixTrack, setName(StrEq("3D Sub-Master")));
        }

        SECTION("The submix is routed to the main 3D mix bus") {
            EXPECT_CALL(mockSubmixTrack, route(Ref(mockControlTrack), 16, 0, 0));
        }

        pluginSuite.onCreateProject(ProjectNode{ fakeRootElement }, api);
    }

    SECTION("onObjectAutomation()") {
//      SECTION("after onCreateObjectTrack()") {
//        pluginSuite.onCreateObjectTrack(fakeElement, api);
//        constexpr double CARTESIAN_ERROR_EPLISION {0.00001};
//        SECTION("Given the automation node has an initial cartesian block pointing straight forward") {
//          auto blocks = ObjectTypeBlockRange{}.with(initialCartesianBlock().withRtime(0).withX(0).withY(1).withZ(0));
//          ON_CALL(fakeAutoElement, blocks()).WillByDefault(Return(blocks));
//          SECTION("A track FX envelope is added to the object track referencing the azimuth parameter it's spatializer plugin") {
//            EXPECT_CALL(api, getPluginEnvelope(fakeObjectTrack, _, Ne(SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX))).Times(AnyNumber());
//            EXPECT_CALL(api, getPluginEnvelope(fakeObjectTrack, _, SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX)).Times(AtLeast(1));
//            SECTION("And a point is added to the envelope with the correct position") {
//              EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeAzimuthEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//              EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, _, DoubleEq(0.5), 0, _, false, _)).Times(1);
//              pluginSuite.onObjectAutomation(fakeAutoElement, api);
//            }
//          }
//        }
//        SECTION("Given the automation node has an initial cartesian block pointing 45 degrees to the left") {
//          auto blocks = ObjectTypeBlockRange{}.with(initialCartesianBlock().withRtime(0).withX(-1).withY(1).withZ(0));
//          ON_CALL(fakeAutoElement, blocks()).WillByDefault(Return(blocks));
//          SECTION("A point is added to the envelope with the correct position") {
//            EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeAzimuthEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//            EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, _, DoubleNear(0.375, CARTESIAN_ERROR_EPLISION), 0, _, false, _)).Times(1);
//            pluginSuite.onObjectAutomation(fakeAutoElement, api);

//                }
//            }

//        SECTION("Given the automation node has an initial cartesian block pointing forward and 45 degrees up") {
//          auto blocks = ObjectTypeBlockRange{}.with(initialCartesianBlock().withRtime(0).withX(0).withY(1).withZ(1));
//          ON_CALL(fakeAutoElement, blocks()).WillByDefault(Return(blocks));
//          SECTION("A point is added to the envelope with the correct position") {
//            EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeElevationEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//            EXPECT_CALL(api, InsertEnvelopePoint(fakeElevationEnvelope, _, DoubleNear(0.75, CARTESIAN_ERROR_EPLISION), 0, _, false, _)).Times(1);
//            pluginSuite.onObjectAutomation(fakeAutoElement, api);
//          }
//        }

//        SECTION("Given the automation node has an initial cartesian block pointing forward at a distance of 2") {
//          auto blocks = ObjectTypeBlockRange{}.with(initialCartesianBlock().withRtime(0).withX(0).withY(2).withZ(0));
//          ON_CALL(fakeAutoElement, blocks()).WillByDefault(Return(blocks));
//          SECTION("A point is added to the envelope with the correct position") {
//            EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeDistanceEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//            EXPECT_CALL(api, InsertEnvelopePoint(fakeDistanceEnvelope, _, DoubleNear(2.00 / 60.0, CARTESIAN_ERROR_EPLISION), 0, _, false, _)).Times(1);
//            pluginSuite.onObjectAutomation(fakeAutoElement, api);
//          }
//        }

//        SECTION("Given the automation node has an initial cartesian block pointing straigh up at the pole at a distance of 60") {
//          auto blocks = ObjectTypeBlockRange{}.with(initialCartesianBlock().withRtime(0).withX(0).withY(0).withZ(60));
//          ON_CALL(fakeAutoElement, blocks()).WillByDefault(Return(blocks));
//          SECTION("A point is added to the envelope with the correct position") {
//            EXPECT_CALL(api, InsertEnvelopePoint(fakeGainEnvelope, _, _, _, _, _, _)).Times(AnyNumber());
//            EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, _, _, _, _, _, _)).Times(AnyNumber()); // it's undefined at the poles so anything
//            EXPECT_CALL(api, InsertEnvelopePoint(fakeDistanceEnvelope, _, DoubleNear(1.0, CARTESIAN_ERROR_EPLISION), 0, _, false, _)).Times(1);
//            EXPECT_CALL(api, InsertEnvelopePoint(fakeElevationEnvelope, _, DoubleNear(1.0, CARTESIAN_ERROR_EPLISION), 0, _, false, _)).Times(1);
//            pluginSuite.onObjectAutomation(fakeAutoElement, api);
//          }
//        }

//        SECTION("Given the automation node has a 0 duration block with azimuth and rtime") {
//          adm::SphericalPosition pos(adm::Azimuth{0.0f});
//          auto const BLOCK_RTIME = 1.0;
//          auto blocks = ObjectTypeBlockRange{}.with(initialSphericalBlock().withRtime(BLOCK_RTIME).withAzimuth(0));
//          ON_CALL(fakeAutoElement, blocks()).WillByDefault(Return(blocks));
//          SECTION("A track FX envelope is added to the object track referencing the azimuth parameter it's spatializer plugin") {
//            EXPECT_CALL(api, getPluginEnvelope(fakeObjectTrack, _, Ne(SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX))).Times(AnyNumber());
//            EXPECT_CALL(api, getPluginEnvelope(fakeObjectTrack, _, SPATIALISER_PLUGIN_AZIMUTH_PARAMETER_INDEX)).Times(AtLeast(1));
//            SECTION("And a point is added to the envelope with the correct position") {
//              EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeAzimuthEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//              EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, _, DoubleEq(0.5), 0, _, false, _)).Times(1);
//              pluginSuite.onObjectAutomation(fakeAutoElement, api);
//            }
//            SECTION("And a point is added to the envelope at the rtime") {
//              EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeAzimuthEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//              EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(BLOCK_RTIME), _, 0, _, false, _)).Times(1);
//              pluginSuite.onObjectAutomation(fakeAutoElement, api);
//            }
//          }
//          SECTION("and a second block with azimuth and timing data") {
//            auto twoBlocks = blocks.followedBy(SphericalCoordBlock{}.withDuration(1));
//            ON_CALL(fakeAutoElement, blocks()).WillByDefault(Return(twoBlocks));
//            SECTION("Two points are added to a track envelope") {
//              EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeAzimuthEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//              EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, _, DoubleEq(0.5), 0, _, false, _)).Times(2);
//              SECTION("And sorted") {
//                EXPECT_CALL(api, Envelope_SortPoints(Ne(fakeAzimuthEnvelope))).Times(AnyNumber());
//                EXPECT_CALL(api, Envelope_SortPoints(fakeAzimuthEnvelope)).Times(AtLeast(1));
//              }
//              pluginSuite.onObjectAutomation(fakeAutoElement, api);
//            }
//          }
//        }
//        SECTION("Given the automation node has a 0 duration block with azimuth, elevation, rtime and duration") {
//          auto blocks = ObjectTypeBlockRange{}.with(initialSphericalBlock().withAzimuth(0.0).withElevation(45.0).withRtime(1));
//          ON_CALL(fakeAutoElement, blocks()).WillByDefault(Return(blocks));
//          SECTION("A track FX envelope is added to the object track referencing the elevation parameter of its spatializer plugin") {
//            EXPECT_CALL(api, getPluginEnvelope(fakeObjectTrack, _, Ne(SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX))).Times(AnyNumber());
//            EXPECT_CALL(api, getPluginEnvelope(fakeObjectTrack, _, SPATIALISER_PLUGIN_ELEVATION_PARAMETER_INDEX)).Times(AtLeast(1));
//            pluginSuite.onObjectAutomation(fakeAutoElement, api);
//          }
//          SECTION("An automation point is added at 0.75") {
//            EXPECT_CALL(api, InsertEnvelopePoint(fakeElevationEnvelope, _, DoubleEq(0.75), 0, _, false, _)).Times(1);
//            EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeElevationEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//            pluginSuite.onObjectAutomation(fakeAutoElement, api);
//          }
//        }
//        SECTION("Given the automation node has a 0 duration block with azimuth, elevation, distance, rtime and duration") {
//          auto blocks = ObjectTypeBlockRange{}.with(initialSphericalBlock().withDistance(6.0));
//          ON_CALL(fakeAutoElement, blocks()).WillByDefault(Return(blocks));
//          SECTION("A track FX envelope is added to the object track referencing the elevation parameter of its spatializer plugin") {
//            EXPECT_CALL(api, getPluginEnvelope(fakeObjectTrack, _, Ne(SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX))).Times(AnyNumber());
//            EXPECT_CALL(api, getPluginEnvelope(fakeObjectTrack, _, SPATIALISER_PLUGIN_DISTANCE_PARAMETER_INDEX)).Times(AtLeast(1));
//            pluginSuite.onObjectAutomation(fakeAutoElement, api);
//          }
//          SECTION("An automation point is added at 0.1") {
//            EXPECT_CALL(api, InsertEnvelopePoint(fakeDistanceEnvelope, _, DoubleEq(0.1), 0, _, false, _)).Times(1);
//            EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeDistanceEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//            pluginSuite.onObjectAutomation(fakeAutoElement, api);
//          }
//        }

//        SECTION("Given the automation node has a non-zero block with azimuth and rtime") {
//          auto blocks = ObjectTypeBlockRange{}.with(SphericalCoordBlock{}.withRtime(1).withDuration(1));
//          ON_CALL(fakeAutoElement, blocks()).WillByDefault(Return(blocks));
//          SECTION("The last envelope point is added at time rtime + duration") {
//              {
//                  InSequence s;
//                  EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, _, _, 0, _, false, _)).Times(AnyNumber());
//                  EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(2.0), _, 0, _, false, _)).Times(1);
//              }
//              EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeAzimuthEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//              pluginSuite.onObjectAutomation(fakeAutoElement, api);
//          }
//        }

//        SECTION("Givent the automation node has an initial block with a gain value") {
//            double const GAIN_VAL = 0.5;
//            auto blocks = ObjectTypeBlockRange{}.with(initialSphericalBlock().withGain(GAIN_VAL));
//            ON_CALL(fakeAutoElement, blocks()).WillByDefault(Return(blocks));
//            SECTION("A gain automation point is added with the supplied value") {
//              EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeGainEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//              EXPECT_CALL(api, InsertEnvelopePoint(fakeGainEnvelope, _, DoubleEq(GAIN_VAL), _, _, _, _)).Times(1);
//              pluginSuite.onObjectAutomation(fakeAutoElement, api);
//            }
//        }

//        SECTION("Given the automation node has an initial block with a non-zero duration") {
//          auto blocks = ObjectTypeBlockRange{}.with(SphericalCoordBlock{}.withRtime(0).withDuration(1));
//          ON_CALL(fakeAutoElement, blocks()).WillByDefault(Return(blocks));
//          SECTION("Azimuth automation points are added at the start and end of the block") {
//              EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(0.0), _, 0, _, false, _)).Times(1);
//              EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(1.0), _, 0, _, false, _)).Times(1);
//              EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeAzimuthEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//              pluginSuite.onObjectAutomation(fakeAutoElement, api);
//          }
//          SECTION("Distance automation points are added at the start and end of the block") {
//              EXPECT_CALL(api, InsertEnvelopePoint(fakeDistanceEnvelope, DoubleEq(0.0), _, 0, _, false, _)).Times(1);
//              EXPECT_CALL(api, InsertEnvelopePoint(fakeDistanceEnvelope, DoubleEq(1.0), _, 0, _, false, _)).Times(1);
//              EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeDistanceEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//              pluginSuite.onObjectAutomation(fakeAutoElement, api);
//          }
//        }

//        SECTION("Given the automation node has two blocks that when following the shortest path traverse an azimuth disconinuity from bottom to top") {
//          auto blocks = ObjectTypeBlockRange{}.with(initialSphericalBlock().withAzimuth(179.0)).followedBy(SphericalCoordBlock{}.withAzimuth(-179.0).withDuration(1.0));
//          ON_CALL(fakeAutoElement, blocks()).WillByDefault(Return(blocks));
//            SECTION("two additional points are added at the time of the discontinuity") {
//                EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeAzimuthEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//                EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(0.0), _, 0, _, false, _)).Times(1);
//                EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(0.5), DoubleEq(0.0), 0, _, false, _)).Times(1);
//                EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(0.5), DoubleEq(1.0), 0, _, false, _)).Times(1);
//                EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(1.0), _, 0, _, false, _)).Times(1);
//                pluginSuite.onObjectAutomation(fakeAutoElement, api);
//            }
//        }

//        SECTION("Given the automation node has two blocks that do not start at rtime 0 and when following the shortest path, traverse an azimuth disconinuity from bottom to top") {
//          auto blocks = ObjectTypeBlockRange{}.with(initialSphericalBlock().withAzimuth(179.0).withRtime(1.0)).followedBy(SphericalCoordBlock{}.withAzimuth(-179.0).withDuration(1.0));
//          ON_CALL(fakeAutoElement, blocks()).WillByDefault(Return(blocks));
//            SECTION("two additional points are added at the time of the discontinuity") {
//                EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeAzimuthEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//                EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(1.0), _, 0, _, false, _)).Times(1);
//                EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(1.5), DoubleEq(0.0), 0, _, false, _)).Times(1);
//                EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(1.5), DoubleEq(1.0), 0, _, false, _)).Times(1);
//                EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(2.0), _, 0, _, false, _)).Times(1);
//                pluginSuite.onObjectAutomation(fakeAutoElement, api);
//            }
//        }

//        SECTION("Given the automation node has two blocks that when following the shortest path traverse an azimuth disconinuity from top to bottom") {
//          auto blocks = ObjectTypeBlockRange{}.with(initialSphericalBlock().withAzimuth(-179.0)).followedBy(SphericalCoordBlock{}.withAzimuth(179.0).withDuration(1.0));
//          ON_CALL(fakeAutoElement, blocks()).WillByDefault(Return(blocks));
//            SECTION("two additional points are added at the time of the discontinuity") {
//                EXPECT_CALL(api, InsertEnvelopePoint(Ne(fakeAzimuthEnvelope), _, _, _, _, _, _)).Times(AnyNumber());
//                EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(0.0), _, 0, _, false, _)).Times(1);
//                EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(0.5), DoubleEq(0.0), 0, _, false, _)).Times(1);
//                EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(0.5), DoubleEq(1.0), 0, _, false, _)).Times(1);
//                EXPECT_CALL(api, InsertEnvelopePoint(fakeAzimuthEnvelope, DoubleEq(1.0), _, 0, _, false, _)).Times(1);
//                pluginSuite.onObjectAutomation(fakeAutoElement, api);
//            }
//        }
//    }
}
}
