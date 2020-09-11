#pragma once
#include <gmock/gmock.h>
#include <pcmgroup.h>

namespace admplug {

class MockIPCMGroup : public IPCMGroup {
 public:
  MOCK_CONST_METHOD0(trackIndices,
      std::vector<int> const&());
  MOCK_CONST_METHOD0(name,
      std::string());
};

}  // namespace admplug
