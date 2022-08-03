#include <gmock/gmock.h>
#include <projectelements.h>
#include <admchannel.h>
#include <parameter.h>
#include <parametised.h>
#include <plugin.h>
#include <track.h>

namespace admplug {

class MockProjectElement : public ProjectElement {
 public:
  MOCK_METHOD2(createProjectElements,
      void(PluginSuite&, ReaperAPI const& api));
  MOCK_CONST_METHOD1(hasAdmElement, bool(adm::ElementConstVariant));
  MOCK_CONST_METHOD1(hasAdmElements, bool(std::vector<adm::ElementConstVariant>));
  MOCK_CONST_METHOD0(getAdmElements, std::vector<adm::ElementConstVariant>());
};

class MockRootElement : public RootElement {
  MOCK_METHOD2(createProjectElements,
      void(PluginSuite&, ReaperAPI const& api));
  MOCK_CONST_METHOD0(getAdmElements, std::vector<adm::ElementConstVariant>());
  MOCK_CONST_METHOD0(startTime, double());
  MOCK_CONST_METHOD0(startOffset, double());
};

class MockTrackElement : public TrackElement {
public:
  MOCK_METHOD2(createProjectElements,
      void(PluginSuite&, ReaperAPI const& api));
  MOCK_CONST_METHOD0(getTrack,
                     std::shared_ptr<Track>());
  MOCK_CONST_METHOD1(hasAdmElement, bool(adm::ElementConstVariant));
  MOCK_CONST_METHOD1(hasAdmElements, bool(std::vector<adm::ElementConstVariant>));
  MOCK_METHOD1(addParentTrack, void(std::shared_ptr<TrackElement> parentTrack));
  MOCK_METHOD1(addMediaItem, MediaItem*(ReaperAPI const& api));
  MOCK_CONST_METHOD0(groups, std::vector<TrackGroup>());
  MOCK_CONST_METHOD0(getAdmElements, std::vector<adm::ElementConstVariant>());
  MOCK_CONST_METHOD0(slaveOfGroups, std::vector<TrackGroup>());
  MOCK_CONST_METHOD0(masterOfGroup, TrackGroup());
  MOCK_METHOD1(addParentProjectElement, bool(std::shared_ptr<ProjectElement> newParentElement));
};

class MockTakeElement : public TakeElement {
public:
  MOCK_METHOD2(createProjectElements,
      void(PluginSuite&, ReaperAPI const& api));
  MOCK_CONST_METHOD1(hasAdmElement, bool(adm::ElementConstVariant));
  MOCK_CONST_METHOD1(hasAdmElements, bool(std::vector<adm::ElementConstVariant>));
  MOCK_CONST_METHOD1(followsAdmElementSequence, bool(std::vector<adm::ElementConstVariant>));
  MOCK_METHOD1(setSource, void(PCM_source* source));
  MOCK_METHOD1(addTrackUid, void(std::shared_ptr<adm::AudioTrackUid const> uid));
  MOCK_METHOD1(hasTrackUid, bool(std::shared_ptr<adm::AudioTrackUid const> uid));
  MOCK_CONST_METHOD0(trackUids, std::vector<std::shared_ptr<adm::AudioTrackUid const>>());
  MOCK_CONST_METHOD0(trackUidCount, int());
  MOCK_CONST_METHOD0(startTime, double());
  MOCK_CONST_METHOD0(getAdmElements, std::vector<adm::ElementConstVariant> ());
  MOCK_METHOD1(addParentProjectElement, bool(std::shared_ptr<ProjectElement> newParentElement));
};


class MockObjectAutomation : public ObjectAutomation {
public:
  MOCK_METHOD2(createProjectElements,
      void(PluginSuite&, ReaperAPI const& api));
  MOCK_CONST_METHOD1(hasAdmElement, bool(adm::ElementConstVariant));
  MOCK_CONST_METHOD1(hasAdmElements, bool(std::vector<adm::ElementConstVariant>));
  MOCK_CONST_METHOD0(getAdmElements, std::vector<adm::ElementConstVariant>());
  MOCK_CONST_METHOD0(blocks,
      adm::BlockFormatsConstRange<adm::AudioBlockFormatObjects>());
  MOCK_CONST_METHOD0(startTime,
               double());
  MOCK_CONST_METHOD0(getTrack, std::shared_ptr<Track>());
  MOCK_CONST_METHOD0(parentTake,
      std::shared_ptr<TakeElement const>());
  MOCK_CONST_METHOD0(takeChannels, std::vector<ADMChannel>());
  MOCK_CONST_METHOD0(channel, ADMChannel());
  MOCK_CONST_METHOD2(apply, void(PluginParameter const&, Plugin const&));
  MOCK_CONST_METHOD2(apply, void(TrackParameter const&, Track const&));
};

class MockDirectSpeakersAutomation : public DirectSpeakersAutomation {
public:
  MOCK_METHOD2(createProjectElements,
      void(PluginSuite&, ReaperAPI const& api));
  MOCK_CONST_METHOD1(hasAdmElement, bool(adm::ElementConstVariant));
  MOCK_CONST_METHOD1(hasAdmElements, bool(std::vector<adm::ElementConstVariant>));
  MOCK_CONST_METHOD0(getAdmElements, std::vector<adm::ElementConstVariant>());
  MOCK_CONST_METHOD0(blocks,
      adm::BlockFormatsConstRange<adm::AudioBlockFormatDirectSpeakers>());
  MOCK_CONST_METHOD0(startTime,
               double());
  MOCK_CONST_METHOD0(getTrack, std::shared_ptr<Track>());
  MOCK_CONST_METHOD0(parentTake,
      std::shared_ptr<TakeElement const>());
  MOCK_CONST_METHOD0(takeChannels, std::vector<ADMChannel>());
  MOCK_CONST_METHOD0(channel, ADMChannel());
  MOCK_CONST_METHOD0(channelIndex, int());
  MOCK_CONST_METHOD2(apply, void(PluginParameter const&, Plugin const&));
  MOCK_CONST_METHOD2(apply, void(TrackParameter const&, Track const&));
};

class MockHoaAutomation : public HoaAutomation {
public:
    MOCK_METHOD2(createProjectElements,
                 void(PluginSuite&, ReaperAPI const& api));
    MOCK_CONST_METHOD1(hasAdmElement, bool(adm::ElementConstVariant));
    MOCK_CONST_METHOD1(hasAdmElements, bool(std::vector<adm::ElementConstVariant>));
    MOCK_CONST_METHOD0(getAdmElements, std::vector<adm::ElementConstVariant>());
    MOCK_CONST_METHOD0(blocks,
                       adm::BlockFormatsConstRange<adm::AudioBlockFormatHoa>());
    MOCK_CONST_METHOD0(startTime,
                       double());
    MOCK_CONST_METHOD0(getTrack, std::shared_ptr<Track>());
    MOCK_CONST_METHOD0(parentTake,
                       std::shared_ptr<TakeElement const>());
    MOCK_CONST_METHOD0(takeChannels, std::vector<ADMChannel>());
    MOCK_CONST_METHOD0(channel, ADMChannel());
    MOCK_CONST_METHOD0(channelIndex, int());
    MOCK_CONST_METHOD2(apply, void(PluginParameter const&, Plugin const&));
    MOCK_CONST_METHOD2(apply, void(TrackParameter const&, Track const&));
};
}  // namespace admplug

