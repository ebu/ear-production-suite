#pragma once

#pragma once
#include <Eigen/Core>
#include <functional>
#include "ear/helpers/assert.hpp"

namespace ear {
namespace plugin {

// Basically the same as libear `VariableBlockSizeAdapter` with the important
// difference that this implementation can work with non-continous input/output
// memory regions, i.e. where each channel is represented by a pointer by
// itself.
//
// Essenantially, this class can wrap a callabke that expecteds fixed block
// sizes to make it work with variable block sizes.

template <typename Buffer>
struct BufferTraits {
  // data type used to represent audio samples 
  //   using SampleType = float;
  // number of channels in the buffer
  //   static Eigen::Index channelCount(const Buffer& b);
  // number of samples-per-channel in the buffer
  //   static Eigen::Index size(const Buffer& b);
  // pointer to the first sample of channel @a n for reading
  //   static const SampleType* getChannel(const Buffer& b, std::size_t n);
  // pointer to the first sample of channel @a n for writing
  //   static SampleType* getChannel(Buffer& b, std::size_t n);
};



template <typename SampleType>
class VariableBlockSizeAdapter {
 public:
  using Samples = Eigen::Matrix<SampleType, Eigen::Dynamic, Eigen::Dynamic>;
  using ProcessFunc = void(const Eigen::Ref<const Samples>& in,
                           Eigen::Ref<Samples> out);

  VariableBlockSizeAdapter(Eigen::Index block_size,
                           Eigen::Index num_channels_in,
                           Eigen::Index num_channels_out,
                           std::function<ProcessFunc> process_func)
      : process_func(process_func),
        block_size(block_size),
        input_buffer(Samples::Zero(block_size, num_channels_in)),
        output_buffer(Samples::Zero(block_size, num_channels_out)),
        samples_in_input(0) {}

  template <typename InputBuffer, typename OutputBuffer>
  void process(const InputBuffer& in, OutputBuffer& out) {
    using InTraits = BufferTraits<InputBuffer>;
    using OutTraits = BufferTraits<OutputBuffer>;
    if (InTraits::size(in) != OutTraits::size(out)) {
      throw invalid_argument("in and out must have same number of samples");
    }
    if (InTraits::channelCount(in) < input_buffer.cols()) {
      throw invalid_argument(
          "in does not have the expected number of channels");
    }
    if (OutTraits::channelCount(out) < output_buffer.cols()) {
      throw invalid_argument(
          "out does not have the expected number of channels");
    }

    Eigen::Index sample = 0;
    while (sample < InTraits::size(in)) {
      // move in -> input_buffer and out -> output_buffer until out of samples
      // (or input_buffer is full and output_buffer is empty)
      Eigen::Index to_transfer =
          std::min(InTraits::size(in) - sample, block_size - samples_in_input);

      auto ext_block = Eigen::seqN(sample, to_transfer);
      auto int_block = Eigen::seqN(samples_in_input, to_transfer);
      for (Eigen::Index ch = 0; ch < input_buffer.cols(); ++ch) {
        using MatrixType =
            typename Eigen::Matrix<SampleType, Eigen::Dynamic, 1>;
        using MapType = typename Eigen::Map<const MatrixType>;
        auto inref = MapType(InTraits::getChannel(in, ch), InTraits::size(in));
        input_buffer(int_block, ch) = inref(ext_block, Eigen::all);
      }
      for (Eigen::Index ch = 0; ch < output_buffer.cols(); ++ch) {
        using MatrixType =
            typename Eigen::Matrix<SampleType, Eigen::Dynamic, 1>;
        using MapType = typename Eigen::Map<MatrixType>;
        auto outref =
            MapType(OutTraits::getChannel(out, ch), OutTraits::size(out));
        outref(ext_block, Eigen::all) = output_buffer(int_block, ch);
      }

      sample += to_transfer;
      samples_in_input += to_transfer;

      // run process from input_buffer to output_buffer
      bool run_process = samples_in_input == block_size;
      if (run_process) {
        process_func(input_buffer, output_buffer);
        samples_in_input = 0;
      }

      // check that we made progress
      ear_assert(run_process || to_transfer > 0, "no progress made");
    }

    // ear_assert(sample == in.rows(), "processed more samples than expected");
  }

  // the delay introduced by the variable block size processing, not
  // accounting for any delay introduced by the inner process
  Eigen::Index get_delay() const { return block_size; }

 private:
  std::function<ProcessFunc> process_func;
  Eigen::Index block_size;
  // Buffers for input and output samples, both block_size long.
  // input_buffer contains samples_in_input samples, starting at the start.
  // output_buffer contains (block_size - samples_in_input) samples, starting
  // at samples_in_input (so, aligned at the end of the buffer).
  Samples input_buffer;
  Samples output_buffer;
  Eigen::Index samples_in_input;
};
}  // namespace plugin
}  // namespace ear
