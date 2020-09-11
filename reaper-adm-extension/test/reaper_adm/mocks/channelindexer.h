#pragma once
#include <gmock/gmock.h>
#include <channelindexer.h>

namespace admplug {

class MockIChannelIndexer : public IChannelIndexer {
 public:
  MOCK_CONST_METHOD1(indexOf,
      int(std::shared_ptr<adm::AudioTrackUid const> uid));
};

}  // namespace admplug
