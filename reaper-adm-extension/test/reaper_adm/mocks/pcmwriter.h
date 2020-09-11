#pragma once
#include <gmock/gmock.h>
#include <pcmwriter.h>

namespace admplug {

class MockIPCMWriter : public IPCMWriter {
 public:
  MOCK_METHOD1(write,
      void(IPCMBlock const& block));
  MOCK_METHOD0(Die, void());
  MOCK_METHOD0(fileName, std::string());
  virtual ~MockIPCMWriter(){ Die(); }
};

}  // namespace admplug
