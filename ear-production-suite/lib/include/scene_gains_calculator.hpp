#pragma once
#include <vector>
#include <memory>
#include <boost/variant.hpp>
#include "scene_store.pb.h"
#include "communication/common_types.hpp"
#include <ear/ear.hpp>
#include <Eigen/Eigen>
#include <map>
#include <vector>

namespace ear {
namespace plugin {

struct GainHolder {
  Eigen::MatrixXf direct;
  Eigen::MatrixXf diffuse;
};

struct Routing {
  int track;
  int size;
};

class SceneGainsCalculator {
 public:
  SceneGainsCalculator(Layout outputLayout, int inputChannelCount);
  bool update(proto::SceneStore store);
  Eigen::MatrixXf directGains();
  Eigen::MatrixXf diffuseGains();

 private:
  // returns ids that had been present at the last `update()` call,`
  // but are no longer in the given scene store
  std::vector<communication::ConnectionId> removedIds(
      const proto::SceneStore &store) const;

  // updates internal routing cache, adding new items and changing routing
  // values as needed
  //
  // @returns Map of item ids whose routing has changed with their
  // _previous_ routing values
  std::vector<Routing> updateRoutingCache(const proto::SceneStore &store);

  int currentIndex_;
  void resize(ear::Layout &ouputLayout, std::size_t inputChannelCount);
  void clear();
  std::vector<std::vector<float>> direct_;
  std::vector<std::vector<float>> diffuse_;
  ear::GainCalculatorObjects objectCalculator_;
  ear::GainCalculatorDirectSpeakers directSpeakersCalculator_;
  std::map<communication::ConnectionId, Routing> routingCache_;
};

}  // namespace plugin
}  // namespace ear
