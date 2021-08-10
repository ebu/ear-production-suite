#pragma once

#include "ear/dsp/block_convolver.hpp"
#include <vector>
#include <memory>
#include <Eigen/Core>

namespace ear {
namespace plugin {

/**
 * Combines multiple ear::convolver to support convoling of multiple input
 * channes with a  single interface
 */
class MultichannelConvolver {
 public:
  MultichannelConvolver(std::vector<std::vector<float>> filters,
                        std::size_t blockSize);

  void process(const Eigen::Ref<const Eigen::MatrixXf>& in,
               Eigen::Ref<Eigen::MatrixXf> out);

 private:
  std::vector<std::unique_ptr<dsp::block_convolver::BlockConvolver>>
      convolvers_;
  std::size_t blockSize_;
};
}  // namespace plugin
}  // namespace ear
