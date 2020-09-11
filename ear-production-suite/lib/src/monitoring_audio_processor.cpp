#include "monitoring_audio_processor.hpp"
#include "ear/dsp/gain_interpolator.hpp"
#include "ear/decorrelate.hpp"
#include <functional>

#include <iostream>

using std::placeholders::_1;
using std::placeholders::_2;

namespace ear {
namespace plugin {

inline std::vector<std::vector<float>> convertToVec(GainMatrix m) {
  std::vector<std::vector<float>> vec;
  for (int i = 0; i < m.cols(); ++i) {
    vec.push_back(
        std::vector<float>(m.col(i).data(), m.col(i).data() + m.rows()));
  }
  return vec;
}

MonitoringAudioProcessor::MonitoringAudioProcessor(
    std::size_t inputChannelCount, Layout layout, std::size_t blockSize)
    : inputChannelCount_(inputChannelCount),
      internalBlockSize_(blockSize),
      blockAdapter_(
          blockSize, inputChannelCount, layout.channels().size(),
          std::bind(&MonitoringAudioProcessor::doBlockedProcess, this, _1, _2)),
      directPathDelay_(layout.channels().size(), blockSize),
      bufferA_(blockSize, layout.channels().size()),
      bufferB_(blockSize, layout.channels().size()),
      currentDirectGains_(layout.channels().size(), inputChannelCount_),
      currentDiffuseGains_(layout.channels().size(), inputChannelCount_),
      nextDirectGains_(layout.channels().size(), inputChannelCount_),
      nextDiffuseGains_(layout.channels().size(), inputChannelCount_),
      convolver_(ear::designDecorrelators<float>(layout), blockSize) {
  currentDirectGains_.setZero();
  currentDiffuseGains_.setZero();
  nextDirectGains_.setZero();
  nextDiffuseGains_.setZero();
}

std::size_t MonitoringAudioProcessor::delayInSamples() const {
  return blockAdapter_.get_delay() + directPathDelay_.get_delay();
}

void MonitoringAudioProcessor::doBlockedProcess(
    const Eigen::Ref<const Eigen::MatrixXf>& in,
    Eigen::Ref<Eigen::MatrixXf> out) {
  // in -> gain_interp_direct -> buffer_a
  // buffer_a -> delay_buffer -> out
  // in -> gain_interp_diffuse -> buffer_a
  // buffer_a -> convolvers > buffer_b
  // out += buffer_b

  ear::dsp::PtrAdapterConst in_p(in.cols());
  in_p.set_eigen(in);
  ear::dsp::PtrAdapter out_p(out.cols());
  out_p.set_eigen(out);
  dsp::PtrAdapter bufferA_p(bufferA_.cols());
  bufferA_p.set_eigen(bufferA_);

  // Apply gain ramp for direct path
  std::vector<std::vector<float>> currentDirectGains_v =
      convertToVec(currentDirectGains_);
  std::vector<std::vector<float>> nextDirectGains_v =
      convertToVec(nextDirectGains_);

  dsp::LinearInterpMatrix::apply_interp(
      in_p.ptrs(), bufferA_p.ptrs(), 0, internalBlockSize_, 0, 0,
      internalBlockSize_, currentDirectGains_v, nextDirectGains_v);
  currentDirectGains_ = nextDirectGains_;

  // delay direct path to align with diffuse path
  directPathDelay_.process(internalBlockSize_, bufferA_p.ptrs(), out_p.ptrs());

  // apply gain ramp for diffuse path
  std::vector<std::vector<float>> currentDiffuseGains_v =
      convertToVec(currentDiffuseGains_);
  std::vector<std::vector<float>> nextDiffuseGains_v =
      convertToVec(nextDiffuseGains_);
  dsp::LinearInterpMatrix::apply_interp(
      in_p.ptrs(), bufferA_p.ptrs(), 0, internalBlockSize_, 0, 0,
      internalBlockSize_, currentDiffuseGains_v, nextDiffuseGains_v);
  currentDiffuseGains_ = nextDiffuseGains_;

  convolver_.process(bufferA_, bufferB_);
  out += bufferB_;
}

}  // namespace plugin
}  // namespace ear
