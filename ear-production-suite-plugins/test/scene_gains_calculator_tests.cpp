#include "scene_store.pb.h"
#include "scene_gains_calculator.hpp"
#include "helper/eps_to_ear_metadata_converter.hpp"
#include "eigen_catch2.hpp"
#include <ear/bs2051.hpp>
#include <catch2/catch_all.hpp>

using namespace ear::plugin;

const int INPUT_CHANNELS = 64;

void updateExpectedGainMatrix(const ear::ObjectsTypeMetadata& metadata,
                              int track, ear::GainCalculatorObjects& calculator,
                              Eigen::MatrixXf& destDirect,
                              Eigen::MatrixXf& destDiffuse) {
  std::vector<float> direct(destDirect.rows(), 0.f);
  std::vector<float> diffuse(destDiffuse.rows(), 0.f);
  calculator.calculate(metadata, direct, diffuse);
  for (std::size_t n = 0; n < direct.size(); ++n) {
    destDirect(n, track) = direct[n];
  }
  for (std::size_t n = 0; n < diffuse.size(); ++n) {
    destDiffuse(n, track) = diffuse[n];
  }
}

TEST_CASE("scene gain calculation (Objects)") {
  proto::SceneStore store;
  auto obj1 = new proto::ObjectsTypeMetadata();

  obj1->mutable_position()->set_azimuth(9.0);
  obj1->mutable_position()->set_elevation(10.0);
  obj1->mutable_position()->set_distance(11.0);
  obj1->set_gain(0.7);

  auto layout = ear::getLayout("0+5+0");

  ear::GainCalculatorObjects referenceCalculator(layout);

  ear::plugin::SceneGainsCalculator calculator(layout, INPUT_CHANNELS);

  Eigen::MatrixXf expectedDirect =
      Eigen::MatrixXf::Zero(layout.channels().size(), INPUT_CHANNELS);
  Eigen::MatrixXf expectedDiffuse =
      Eigen::MatrixXf::Zero(layout.channels().size(), INPUT_CHANNELS);

  SECTION("initial condition") {
    auto diffuseGains = calculator.diffuseGains();
    auto directGains = calculator.directGains();
    REQUIRE(diffuseGains.isZero());
    REQUIRE(directGains.isZero());
  }

  SECTION("update empty store") {
    calculator.update(store);

    auto diffuseGains = calculator.diffuseGains();
    auto directGains = calculator.directGains();
    REQUIRE(diffuseGains.isZero());
    REQUIRE(directGains.isZero());
  }

  SECTION("new item") {
    auto item1 = store.add_monitoring_items();
    item1->set_connection_id(communication::ConnectionId::generate().string());
    item1->set_routing(1);
    item1->set_changed(true);
    item1->set_allocated_obj_metadata(obj1);

    calculator.update(store);
    updateExpectedGainMatrix(EpsToEarMetadataConverter::convert(*obj1), 1,
                             referenceCalculator, expectedDirect,
                             expectedDiffuse);

    auto diffuseGains = calculator.diffuseGains();
    auto directGains = calculator.directGains();
    CHECK_THAT(directGains, IsApprox(expectedDirect));
    CHECK_THAT(diffuseGains, IsApprox(expectedDiffuse));
  }

  SECTION("remove item") {
    auto item1 = store.add_monitoring_items();
    item1->set_connection_id(communication::ConnectionId::generate().string());
    item1->set_routing(1);
    item1->set_changed(true);
    item1->set_allocated_obj_metadata(obj1);

    calculator.update(store);
    store.clear_monitoring_items();
    calculator.update(store);

    auto diffuseGains = calculator.diffuseGains();
    auto directGains = calculator.directGains();
    REQUIRE(diffuseGains.isZero());
    REQUIRE(directGains.isZero());
  }

  SECTION("change item") {
    auto item1 = store.add_monitoring_items();
    item1->set_connection_id(communication::ConnectionId::generate().string());
    item1->set_routing(1);
    item1->set_changed(true);
    item1->set_allocated_obj_metadata(obj1);

    calculator.update(store);

    obj1->set_gain(1.0);

    calculator.update(store);
    updateExpectedGainMatrix(EpsToEarMetadataConverter::convert(*obj1), 1,
                             referenceCalculator, expectedDirect,
                             expectedDiffuse);

    auto diffuseGains = calculator.diffuseGains();
    auto directGains = calculator.directGains();
    CHECK_THAT(directGains, IsApprox(expectedDirect));
    CHECK_THAT(diffuseGains, IsApprox(expectedDiffuse));
  }

  SECTION("change routing") {
    auto item1 = store.add_monitoring_items();
    item1->set_connection_id(communication::ConnectionId::generate().string());
    item1->set_routing(1);
    item1->set_changed(true);
    item1->set_allocated_obj_metadata(obj1);

    calculator.update(store);

    item1->set_routing(8);

    calculator.update(store);
    updateExpectedGainMatrix(EpsToEarMetadataConverter::convert(*obj1), 8,
                             referenceCalculator, expectedDirect,
                             expectedDiffuse);

    auto diffuseGains = calculator.diffuseGains();
    auto directGains = calculator.directGains();
    CHECK_THAT(directGains, IsApprox(expectedDirect));
    CHECK_THAT(diffuseGains, IsApprox(expectedDiffuse));
  }

  SECTION("negative routing") {
    auto item1 = store.add_monitoring_items();
    item1->set_connection_id(communication::ConnectionId::generate().string());
    item1->set_routing(-1);
    item1->set_changed(true);
    item1->set_allocated_obj_metadata(obj1);

    calculator.update(store);

    auto diffuseGains = calculator.diffuseGains();
    auto directGains = calculator.directGains();
    REQUIRE(diffuseGains.isZero());
    REQUIRE(directGains.isZero());
  }

  SECTION("honor change flag") {
    // if the changed flag is false, it is expected that calculated gains do NOT
    // change, i.e. are not re-calculated to test this, we change the object
    // metadata _on purpose_ while setting the changed flag to false. While this
    // would be an unexpected input under normal runtime conditions, we can use
    // it to see if the gain values are really not re-calculated

    auto item1 = store.add_monitoring_items();
    item1->set_connection_id(communication::ConnectionId::generate().string());
    item1->set_routing(1);
    item1->set_changed(true);
    item1->set_allocated_obj_metadata(obj1);

    calculator.update(store);
    updateExpectedGainMatrix(EpsToEarMetadataConverter::convert(*obj1), 1,
                             referenceCalculator, expectedDirect,
                             expectedDiffuse);

    obj1->set_gain(1.f);
    item1->set_changed(false);

    calculator.update(store);

    auto diffuseGains = calculator.diffuseGains();
    auto directGains = calculator.directGains();
    CHECK_THAT(directGains, IsApprox(expectedDirect));
    CHECK_THAT(diffuseGains, IsApprox(expectedDiffuse));
  }
}

void updateExpectedGainMatrixSingleChannel(
    const ear::DirectSpeakersTypeMetadata& metadata, int track,
    ear::GainCalculatorDirectSpeakers& calculator,
    Eigen::MatrixXf& destDirect) {
  std::vector<float> direct(destDirect.rows(), 0.f);
  calculator.calculate(metadata, direct);
  for (std::size_t n = 0; n < direct.size(); ++n) {
    destDirect(n, track) = direct[n];
  }
}

void updateExpectedGainMatrix(
    const std::vector<ear::DirectSpeakersTypeMetadata>& metadata, int track,
    ear::GainCalculatorDirectSpeakers& calculator,
    Eigen::MatrixXf& destDirect) {
  for (int i = 0; i < metadata.size(); ++i) {
    updateExpectedGainMatrixSingleChannel(metadata[i], track + i, calculator,
                                          destDirect);
  }
}

TEST_CASE("scene gain calculation (DirectSpeakers)") {
  proto::SceneStore store;
  auto obj1 = proto::convertSpeakerSetupToEpsMetadata(14);
  REQUIRE(obj1->speakers_size() == 10);

  auto layout = ear::getLayout("0+5+0");

  ear::GainCalculatorDirectSpeakers referenceCalculator(layout);

  ear::plugin::SceneGainsCalculator calculator(layout, INPUT_CHANNELS);

  Eigen::MatrixXf expectedDirect =
      Eigen::MatrixXf::Zero(layout.channels().size(), INPUT_CHANNELS);

  SECTION("initial condition") {
    auto directGains = calculator.directGains();
    REQUIRE(directGains.isZero());
  }

  SECTION("update empty store") {
    calculator.update(store);
    auto directGains = calculator.directGains();
    REQUIRE(directGains.isZero());
  }

  SECTION("new item") {
    auto item1 = store.add_monitoring_items();
    item1->set_connection_id(communication::ConnectionId::generate().string());
    item1->set_routing(1);
    item1->set_changed(true);
    item1->set_allocated_ds_metadata(obj1);
    calculator.update(store);
    updateExpectedGainMatrix(EpsToEarMetadataConverter::convert(*obj1),
                             item1->routing(), referenceCalculator,
                             expectedDirect);

    auto directGains = calculator.directGains();
    CHECK_THAT(directGains, IsApprox(expectedDirect));
  }

  SECTION("remove item") {
    auto item1 = store.add_monitoring_items();
    item1->set_connection_id(communication::ConnectionId::generate().string());
    item1->set_routing(1);
    item1->set_changed(true);
    item1->set_allocated_ds_metadata(obj1);

    calculator.update(store);
    store.clear_monitoring_items();
    calculator.update(store);

    auto directGains = calculator.directGains();
    REQUIRE(directGains.isZero());
  }

  SECTION("change item") {
    auto item1 = store.add_monitoring_items();
    item1->set_connection_id(communication::ConnectionId::generate().string());
    item1->set_routing(1);
    item1->set_changed(true);
    item1->set_allocated_ds_metadata(obj1);

    calculator.update(store);

    auto obj2 = proto::convertSpeakerSetupToEpsMetadata(6);
    item1->set_allocated_ds_metadata(obj2);

    calculator.update(store);
    updateExpectedGainMatrix(EpsToEarMetadataConverter::convert(*obj2),
                             item1->routing(), referenceCalculator,
                             expectedDirect);

    auto directGains = calculator.directGains();
    CHECK_THAT(directGains, IsApprox(expectedDirect));
  }

  // SECTION("change routing") {
  //   auto item1 = store.add_monitoring_items();
  //   item1->set_connection_id(communication::ConnectionId::generate().string());
  //   item1->set_routing(1);
  //   item1->set_changed(true);
  //   item1->set_allocated_ds_metadata(obj1);

  //   calculator.update(store);

  //   item1->set_routing(8);

  //   calculator.update(store);
  //   updateExpectedGainMatrix(EpsToEarMetadataConverter::convert(*obj1), 8,
  //   referenceCalculator,
  //                            expectedDirect);

  //   auto directGains = calculator.directGains();
  //   CHECK_THAT(directGains, IsApprox(expectedDirect));
  // }

  SECTION("negative routing") {
    auto item1 = store.add_monitoring_items();
    item1->set_connection_id(communication::ConnectionId::generate().string());
    item1->set_routing(-1);
    item1->set_changed(true);
    item1->set_allocated_ds_metadata(obj1);

    calculator.update(store);

    auto directGains = calculator.directGains();
    REQUIRE(directGains.isZero());
  }

  SECTION("out of bounds routing") {
    auto item1 = store.add_monitoring_items();
    item1->set_connection_id(communication::ConnectionId::generate().string());
    item1->set_routing(62);
    item1->set_changed(true);
    item1->set_allocated_ds_metadata(obj1);

    calculator.update(store);

    auto directGains = calculator.directGains();
    REQUIRE(directGains.isZero());
  }

  SECTION("honor change flag") {
    // if the changed flag is false, it is expected that calculated gains do NOT
    // change, i.e. are not re-calculated to test this, we change the object
    // metadata _on purpose_ while setting the changed flag to false. While this
    // would be an unexpected input under normal runtime conditions, we can use
    // it to see if the gain values are really not re-calculated

    auto item1 = store.add_monitoring_items();
    item1->set_connection_id(communication::ConnectionId::generate().string());
    item1->set_routing(1);
    item1->set_changed(true);
    item1->set_allocated_ds_metadata(obj1);

    calculator.update(store);
    updateExpectedGainMatrix(EpsToEarMetadataConverter::convert(*obj1),
                             item1->routing(), referenceCalculator,
                             expectedDirect);

    auto obj2 = proto::convertSpeakerSetupToEpsMetadata(6);
    item1->set_allocated_ds_metadata(obj2);
    item1->set_changed(false);

    calculator.update(store);

    auto diffuseGains = calculator.diffuseGains();
    auto directGains = calculator.directGains();
    CHECK_THAT(directGains, IsApprox(expectedDirect));
  }
}
