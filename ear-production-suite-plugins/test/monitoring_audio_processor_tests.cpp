#include "monitoring_audio_processor.hpp"
#include "eigen_catch2.hpp"
#include "ear/bs2051.hpp"
#include <catch2/catch_all.hpp>
#include <ear/dsp/gain_interpolator.hpp>
#include <ear/decorrelate.hpp>
#include <Eigen/Core>

namespace ear {
namespace plugin {

template <>
struct BufferTraits<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>> {
  using Buffer = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;
  using SampleType = float;
  static Eigen::Index channelCount(const Buffer& b) { return b.cols(); };
  static Eigen::Index size(const Buffer& b) { return b.rows(); }
  static const SampleType* getChannel(const Buffer& b, std::size_t n) {
    return b.col(n).data();
  }
  static SampleType* getChannel(Buffer& b, std::size_t n) {
    return b.col(n).data();
  }
};
}  // namespace plugin
}  // namespace ear

TEST_CASE("direct_path") {
  auto layout = ear::getLayout("0+5+0").withoutLfe();
  std::size_t blockSize = 10;
  ear::plugin::MonitoringAudioProcessor processor(2, layout, blockSize);

  using Buffer = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;

  Buffer in(processor.delayInSamples() + blockSize, 2);
  in.setZero();
  in(Eigen::all, 0).setConstant(.5);
  in(Eigen::all, 1).setConstant(1.f);

  Buffer out(processor.delayInSamples() + blockSize, 5);
  out.setZero();

  ear::plugin::GainMatrix gainDirect = Eigen::MatrixXf::Zero(5, 2);
  ear::plugin::GainMatrix gainDiffuse = Eigen::MatrixXf::Zero(5, 2);

  gainDirect(0, 0) = 1.f;
  gainDirect(1, 0) = 2.f;
  gainDirect(1, 1) = 0.5f;
  gainDirect(2, 1) = 1.f;
  gainDirect(4, 1) = 0.25f;

  Buffer expectedOutput(processor.delayInSamples() + blockSize,
                        layout.channels().size());
  expectedOutput.setZero();
  auto targetSignalIndizes = Eigen::seqN(processor.delayInSamples(), blockSize);
  // it's tempting to use the something like Eigen::VectorXf::Linspaced to
  // calculate the interpolation result, but this unfortuenatly interpolates a
  // closed interval, where our implemenation interpolation within a open
  // interval
  expectedOutput(targetSignalIndizes, 0) << 0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3,
      0.35, 0.4, 0.45;
  expectedOutput(targetSignalIndizes, 1) << 0, 0.15, 0.3, 0.45, 0.6, 0.75, 0.9,
      1.05, 1.2, 1.35;
  expectedOutput(targetSignalIndizes, 2) << 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6,
      0.7, 0.8, 0.9;
  expectedOutput(targetSignalIndizes, 4) << 0, 0.025, 0.05, 0.075, 0.1, 0.125,
      0.15, 0.175, 0.2, 0.225;

  processor.process(in, out, gainDirect, gainDiffuse);

  REQUIRE(out.isApprox(expectedOutput));
}

std::vector<std::unique_ptr<ear::dsp::block_convolver::BlockConvolver>>
makeConvolvers(const ear::Layout& layout, std::size_t blockSize) {
  auto decorrelators = ear::designDecorrelators(layout);
  std::vector<std::unique_ptr<ear::dsp::block_convolver::BlockConvolver>>
      convolvers;
  auto context =
      ear::dsp::block_convolver::Context(blockSize, ear::get_fft_kiss<float>());
  for (const auto& decorrelator : decorrelators) {
    ear::dsp::block_convolver::Filter filter(context, decorrelator.size(),
                                             decorrelator.data());
    convolvers.push_back(
        std::make_unique<ear::dsp::block_convolver::BlockConvolver>(context,
                                                                    filter));
  }
  return convolvers;
}

TEST_CASE("diffuse_path") {
  auto layout = ear::getLayout("0+5+0").withoutLfe();
  std::size_t blockSize = 10;
  ear::plugin::MonitoringAudioProcessor processor(2, layout, blockSize);

  auto convolvers = makeConvolvers(layout, blockSize);
  using Buffer = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;

  Buffer in(processor.delayInSamples() + blockSize, 2);
  in.setZero();
  in(Eigen::all, 0).setConstant(.5);
  in(Eigen::all, 1).setConstant(1.f);

  Buffer out(processor.delayInSamples() + blockSize, 5);
  out.setZero();

  ear::plugin::GainMatrix gainDirect = Eigen::MatrixXf::Zero(5, 2);
  ear::plugin::GainMatrix gainDiffuse = Eigen::MatrixXf::Zero(5, 2);

  gainDiffuse(0, 0) = 1.f;
  gainDiffuse(1, 0) = 2.f;
  gainDiffuse(1, 1) = 0.5f;
  gainDiffuse(2, 1) = 1.f;
  gainDiffuse(4, 1) = 0.25f;

  Buffer decorrelatorInput(blockSize, layout.channels().size());
  decorrelatorInput.setZero();
  // it's tempting to use the something like Eigen::VectorXf::Linspaced to
  // calculate the interpolation result, but this unfortuenatly interpolates a
  // closed interval, where our implemenation interpolation within a open
  // interval
  decorrelatorInput.col(0) << 0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4,
      0.45;
  decorrelatorInput.col(1) << 0, 0.15, 0.3, 0.45, 0.6, 0.75, 0.9, 1.05, 1.2,
      1.35;
  decorrelatorInput.col(2) << 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9;
  decorrelatorInput.col(4) << 0, 0.025, 0.05, 0.075, 0.1, 0.125, 0.15, 0.175,
      0.2, 0.225;

  Buffer expectedOutput(processor.delayInSamples() + blockSize,
                        layout.channels().size());
  Buffer tmpBuffer(blockSize, layout.channels().size());
  expectedOutput.setZero();
  for (std::size_t n = 0; n < layout.channels().size(); ++n) {
    convolvers.at(n)->process(decorrelatorInput.col(n).data(),
                              tmpBuffer.col(n).data());
  }
  auto targetSignalIndizes = Eigen::seqN(blockSize, blockSize);
  expectedOutput(targetSignalIndizes, Eigen::all) = tmpBuffer;
  decorrelatorInput.col(0).setConstant(0.5f);

  decorrelatorInput.col(1).setConstant(1.5f);
  decorrelatorInput.col(2).setConstant(1.f);
  decorrelatorInput.col(4).setConstant(0.25f);
  for (std::size_t n = 0; n < layout.channels().size(); ++n) {
    convolvers.at(n)->process(decorrelatorInput.col(n).data(),
                              tmpBuffer.col(n).data());
  }

  targetSignalIndizes = Eigen::seqN(processor.delayInSamples(), blockSize);
  expectedOutput(targetSignalIndizes, Eigen::all) = tmpBuffer;

  processor.process(in, out, gainDirect, gainDiffuse);

  CHECK_THAT(out, IsApprox(expectedOutput));
}
