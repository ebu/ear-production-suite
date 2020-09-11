#pragma once
#include <gmock/gmock.h>
#include <channelrouter.h>

namespace admplug {

class MockIChannelRouter : public IPCMWriter {
 public:
  MOCK_METHOD1(write,
      void(IPCMBlock const& block));
  MOCK_METHOD0(group,
      PCMGroup());
  MOCK_METHOD0(source,
      PCM_source*());
  MOCK_METHOD0(fileName,
      std::string());
};

}  // namespace admplug
