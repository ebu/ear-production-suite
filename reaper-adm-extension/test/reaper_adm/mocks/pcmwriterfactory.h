#pragma once
#include <gmock/gmock.h>
#include <pcmwriterfactory.h>

namespace admplug {

class MockPCMWriterFactory : public PCMWriterFactory {
 public:
  MOCK_CONST_METHOD3(createGroupWriter,
      std::unique_ptr<IPCMWriter>(IPCMGroup const& group, std::string path, std::string fnPrefix));
};

}  // namespace admplug
