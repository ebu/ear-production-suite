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
#include "helper/adm_preset_definitions_helper.h"

namespace ear {
namespace plugin {

struct GainHolder {
  Eigen::MatrixXf direct;
  Eigen::MatrixXf diffuse;
};

struct ItemGains {
  int inputStartingChannel;
  std::vector<std::vector<float>> direct_;
  std::vector<std::vector<float>> diffuse_;
};

class SceneGainsCalculator {
 public:
  SceneGainsCalculator(Layout outputLayout, int inputChannelCount);
  bool update(proto::SceneStore store);
  Eigen::MatrixXf directGains();
  Eigen::MatrixXf diffuseGains();

 private:
  int totalOutputChannels;
  int totalInputChannels;

  void removeItem(const communication::ConnectionId &itemId);
  void addOrUpdateItem(const proto::MonitoringItemMetadata &item);

  ear::GainCalculatorObjects objectCalculator_;
  ear::GainCalculatorDirectSpeakers directSpeakersCalculator_;
  ear::GainCalculatorHOA hoaCalculator_;

  std::map<communication::ConnectionId, ItemGains> routingCache_;

  std::mutex commonDefinitionHelperMutex_;
  AdmPresetDefinitionsHelper commonDefinitionHelper_{};
};

}  // namespace plugin
}  // namespace ear
