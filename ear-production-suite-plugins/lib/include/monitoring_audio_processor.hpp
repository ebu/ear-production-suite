#pragma once
#include "variable_block_adapter.hpp"
#include "multichannel_convolver.hpp"
#include "ear/dsp/dsp.hpp"
#include "ear/dsp/ptr_adapter.hpp"
#include "ear/layout.hpp"
#include <cstddef>
#include <vector>

namespace ear {
namespace plugin {

using GainMatrix = Eigen::MatrixXf;

/**
 * @brief Monitoring plugin dsp implementation
 *
 * This class handles the actual number crunching,
 * i.e. given a target gain matrix for the direct and diffuse rendering path
 * plus a set of input samples and a place to store the output,
 * it will generate loudspeaker signals accordingly.
 */
class MonitoringAudioProcessor {
 public:
  using DecorrelationFilter = std::vector<float>;

  /**
   * @brief
   * Initializes audio processor and subcomponents.
   *
   * Number of output channels is implicitly given by `layout`.
   *
   * The monitoring is be able to accept variable blocksizes larger/smaler
   * then `blockSize`, but the actual processing will be done in `blockSize`
   * blocks of audio.
   *
   * @param inputChannelCount number of input channels to process
   * @param Layout layout speaker layout
   * @param blockSize (internal) processing block size
   */
  MonitoringAudioProcessor(std::size_t inputChannelCount, Layout layout,
                           std::size_t blockSize = 512);

  MonitoringAudioProcessor(const MonitoringAudioProcessor&) = delete;
  MonitoringAudioProcessor(MonitoringAudioProcessor&&) = delete;
  MonitoringAudioProcessor& operator=(const MonitoringAudioProcessor&) = delete;
  MonitoringAudioProcessor& operator=(MonitoringAudioProcessor&&) = delete;

  template <typename InBuffer, typename OutBuffer>
  void process(const InBuffer& in, OutBuffer& out, const GainMatrix& direct,
               const GainMatrix& diffuse) {
    nextDirectGains_ = direct;
    nextDiffuseGains_ = diffuse;
    blockAdapter_.process(in, out);
  }

  std::size_t delayInSamples() const;

 private:
  void doBlockedProcess(const Eigen::Ref<const Eigen::MatrixXf>& in,
                        Eigen::Ref<Eigen::MatrixXf> out);
  std::size_t inputChannelCount_;
  std::size_t internalBlockSize_;
  VariableBlockSizeAdapter<float> blockAdapter_;
  dsp::DelayBuffer directPathDelay_;
  Eigen::MatrixXf bufferA_;
  Eigen::MatrixXf bufferB_;
  GainMatrix currentDirectGains_;
  GainMatrix currentDiffuseGains_;
  GainMatrix nextDirectGains_;
  GainMatrix nextDiffuseGains_;
  MultichannelConvolver convolver_;
};

}  // namespace plugin
}  // namespace ear
