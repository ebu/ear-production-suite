#pragma once
#include <gmock/gmock.h>
#include <pcmgroupregistry.h>

namespace admplug {

class MockIPCMGroupRegistry : public IPCMGroupRegistry {
 public:
  MOCK_METHOD2(add,
      void(std::shared_ptr<TakeElement> take, IPCMGroup const& group));
  MOCK_CONST_METHOD0(allGroups,
      std::vector<IPCMGroup const*>());
  MOCK_CONST_METHOD2(setTakeSourceFor,
      void(const IPCMGroup &group, PCM_source *source));
};

}  // namespace admplug
