#pragma once
#include <gmock/gmock.h>
#include <pcmreader.h>

namespace admplug {

class MockPCMReader : public PCMReader {
 public:
  MOCK_METHOD0(read,
      std::shared_ptr<IPCMBlock>());
  MOCK_METHOD0(totalFrames,
      std::size_t());
};

}  // namespace admplug

namespace admplug {

class MockIPCMBlock : public IPCMBlock {
 public:
  MOCK_CONST_METHOD0(frameCount,
      std::size_t());
  MOCK_CONST_METHOD0(channelCount,
      std::size_t());
  MOCK_CONST_METHOD0(sampleRate,
      std::size_t());
  MOCK_CONST_METHOD0(data,
      const std::vector<float>&());
};

}  // namespace admplug
