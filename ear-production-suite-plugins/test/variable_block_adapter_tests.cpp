#define EIGEN_RUNTIME_NO_MALLOC
#include <catch2/catch_all.hpp>
#include "variable_block_adapter.hpp"
#include <vector>
#include <list>

using namespace ear;

namespace ear {
namespace plugin {

class TestBuffer {
 public:
  TestBuffer(std::size_t samples, std::size_t channels)
      : channels_(channels), size_(samples), data_(samples * channels, 0.f) {
    // if the buffer shall be empty, nevertheless add one sample per channel
    // so we can at least get a valid pointer for each channel (wich should
    // never the used) this is only required to make the test code run smoothly
    if (data_.size() == 0) {
      data_ = std::vector<float>(channels_, 0.f);
    }
  }
  TestBuffer(const TestBuffer&) = default;
  TestBuffer& operator=(const TestBuffer&) = default;
  TestBuffer(TestBuffer&&) = default;
  TestBuffer& operator=(TestBuffer&&) = default;

  std::size_t channels() const { return channels_; }
  std::size_t size() const { return size_; }
  float* getChannel(std::size_t channel) {
    return &(data_[0]) + channel * size_;
  }
  const float* getChannel(std::size_t channel) const {
    return &(data_[0]) + channel * size_;
  }

  template <typename It>
  void assign(std::size_t channel, It begin, It end) {
    auto outIt = data_.begin() + (channel * size_);
    float v = *outIt;
    std::copy(begin, end, outIt);
  }

 private:
  std::size_t channels_;
  std::size_t size_;
  std::vector<float> data_;
};

template <>
struct BufferTraits<TestBuffer> {
  using Buffer = TestBuffer;
  using SampleType = float;
  static Eigen::Index channelCount(const Buffer& b) { return b.channels(); };
  static Eigen::Index size(const Buffer& b) { return b.size(); }
  static const SampleType* getChannel(const Buffer& b, std::size_t n) {
    return b.getChannel(n);
  }
  static SampleType* getChannel(Buffer& b, std::size_t n) {
    return b.getChannel(n);
  }
};
}  // namespace plugin
}  // namespace ear

TEST_CASE("test") {
  // dummy process function and parameters
  Eigen::Index inner_block_size = 512;
  Eigen::Index channels_in = 2;
  Eigen::Index channels_out = 4;

  auto do_process = [&](const Eigen::Ref<const Eigen::MatrixXf>& in,
                        Eigen::Ref<Eigen::MatrixXf> out) {
    out(Eigen::all, 0) = in(Eigen::all, 0) * 2;
    out(Eigen::all, 1) = in(Eigen::all, 1) * 3;
    out(Eigen::all, 2) = in(Eigen::all, 0) * 4;
    out(Eigen::all, 3) = in(Eigen::all, 1) * 5;
  };

  auto process = [&](const Eigen::Ref<const Eigen::MatrixXf>& in,
                     Eigen::Ref<Eigen::MatrixXf> out) {
    REQUIRE(in.rows() == inner_block_size);
    REQUIRE(out.rows() == inner_block_size);

    do_process(in, out);
  };

  // wrap this in an adapter
  ear::plugin::VariableBlockSizeAdapter<float> adapter(
      inner_block_size, channels_in, channels_out, process);

  // the test structure
  Eigen::VectorXi block_sizes(5);
  block_sizes << 0, 512, 1024, 300, 500;
  int test_len = block_sizes.sum();

  // generate an input and the expected output, which is shifted by
  Eigen::MatrixXf input = Eigen::MatrixXf::Random(test_len, channels_in);

  std::list<ear::plugin::TestBuffer> blockedInput;
  Eigen::Index offset = 0;
  for (int block_size : block_sizes) {
    auto block = Eigen::seqN(offset, block_size);
    ear::plugin::TestBuffer blockBuffer(block_size, channels_in);
    for (unsigned int n = 0; n < channels_in; ++n) {
      auto v = input(block, n);
      blockBuffer.assign(n, v.begin(), v.end());
    }

    blockedInput.push_back(blockBuffer);
    offset += block_size;
  }

  Eigen::MatrixXf expected_output =
      Eigen::MatrixXf::Zero(test_len, channels_out);

  do_process(input(Eigen::seqN(0, test_len - inner_block_size), Eigen::all),
             expected_output(
                 Eigen::seqN(inner_block_size, test_len - inner_block_size),
                 Eigen::all));

  //   // run with the defined block sizes
  Eigen::MatrixXf output = Eigen::MatrixXf::Random(test_len, channels_out);
  offset = 0;
  std::list<ear::plugin::TestBuffer> blockedOutput;
  for (auto& block : blockedInput) {
    ear::plugin::TestBuffer outputBlock(block.size(), channels_out);
    Eigen::internal::set_is_malloc_allowed(false);
    adapter.process(block, outputBlock);
    Eigen::internal::set_is_malloc_allowed(true);
    blockedOutput.push_back(outputBlock);
    offset += block.size();
  }

  offset = 0;
  for (auto& outFragment : blockedOutput) {
    using MatrixType = typename Eigen::Matrix<float, Eigen::Dynamic, 1>;
    using MapType = typename Eigen::Map<MatrixType>;

    auto block = Eigen::seqN(offset, outFragment.size());
    for (Eigen::Index n = 0; n < channels_out; ++n) {
      output(block, n) = MapType(outFragment.getChannel(n), outFragment.size());
    }

    offset += outFragment.size();
  }

  // check the output
  REQUIRE(expected_output == output);
}
