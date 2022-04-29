#pragma once
#include <vector>
#include <memory>
#include <boost/variant.hpp>
#include "scene_store.pb.h"
#include "communication/common_types.hpp"
#include <ear/ear.hpp>
#include <Eigen/Eigen>
#include <map>
#include <string>
#include <vector>
#include "helper/common_definition_helper.h"

namespace ear {
namespace plugin {

struct GainHolder {
  Eigen::MatrixXf direct;
  Eigen::MatrixXf diffuse;
};

struct ItemRouting {
  int inputStartingChannel;
  int inputChannelCount;
};

class SceneGainsCalculator {
 public:
  SceneGainsCalculator(Layout outputLayout, int inputChannelCount);
  bool update(proto::SceneStore store);
  Eigen::MatrixXf directGains();
  Eigen::MatrixXf diffuseGains();

 private:
  void resize(ear::Layout &ouputLayout, std::size_t inputChannelCount);
  void removeItem(const communication::ConnectionId &itemId);
  void addOrUpdateItem(const proto::MonitoringItemMetadata &item);

  std::vector<std::vector<float>> direct_;
  std::vector<std::vector<float>> diffuse_;

  ear::GainCalculatorObjects objectCalculator_;
  ear::GainCalculatorDirectSpeakers directSpeakersCalculator_;
  ear::GainCalculatorHOA hoaCalculator_;

  std::map<communication::ConnectionId, ItemRouting> routingCache_;

  std::mutex commonDefinitionHelperMutex_;
  AdmCommonDefinitionHelper commonDefinitionHelper_{};
};

}  // namespace plugin
}  // namespace ear
