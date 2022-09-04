#include <gmock/gmock.h>
#include <pluginsuite.h>
#include <projectnode.h>

namespace admplug {

class MockPluginSuite : public PluginSuite {
 public:
  MOCK_METHOD2(onCreateProject,
      void(const ProjectNode &rootNode, const ReaperAPI &api));
  MOCK_METHOD2(onCreateObjectTrack,
      void(TrackElement&, ReaperAPI const& api));
  MOCK_METHOD2(onCreateDirectTrack,
      void(TrackElement&, ReaperAPI const& api));
  MOCK_METHOD2(onCreateHoaTrack,
      void(TrackElement&, ReaperAPI const& api));
  MOCK_METHOD2(onCreateGroup,
      void(TrackElement&, ReaperAPI const& api));
  MOCK_METHOD2(onObjectAutomation,
      void(ObjectAutomation const&, ReaperAPI const& api));
  MOCK_METHOD2(onDirectSpeakersAutomation,
      void(DirectSpeakersAutomation const&, ReaperAPI const& api));
  MOCK_METHOD2(onHoaAutomation,
      void(HoaAutomation const&, ReaperAPI const& api));
  MOCK_METHOD1(pluginSuiteUsable,
      bool(ReaperAPI const& api));
  MOCK_METHOD2(reorderAndFilter,
      std::vector<ADMChannel>(std::vector<ADMChannel> &channels, ReaperAPI const& api));
  MOCK_METHOD2(applyFXPreset,
      bool(HoaAutomation const&, ReaperAPI const& api));
};

}  // namespace admplug
