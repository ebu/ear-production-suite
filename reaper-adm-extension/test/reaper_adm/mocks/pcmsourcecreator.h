#pragma once
#include <gmock/gmock.h>
#include <pcmsourcecreator.h>
#include <projectelements.h>
#include <adm/elements.hpp>

namespace admplug {

class MockIPCMSourceCreator : public IPCMSourceCreator {
 public:
  MOCK_METHOD1(addTake,
               void(std::shared_ptr<TakeElement> take));
  MOCK_METHOD1(linkSources, void(ReaperAPI const&));
  MOCK_METHOD2(extractSources, void(std::string outputDir, ImportContext const& context));
  MOCK_METHOD1(channelForTrackUid, int(std::shared_ptr<const adm::AudioTrackUid> trackUid));
};

}  // namespace admplug
