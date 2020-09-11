#pragma once
#include <memory>
#include <nodefactory.h>
#include <projectelements.h>
#include <mediatrackelement.h>
#include <mediatakeelement.h>
#include <objectautomationelement.h>
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::Invoke;

namespace admplug {
class FakeCreator : public NodeFactory {
public:
    std::shared_ptr<ProjectNode> createObjectTrackNode(std::vector<adm::ElementConstVariant> el, std::shared_ptr<TrackElement> parentTrack)
  {
      auto mte = std::make_shared<ObjectTrack>(el, parentTrack);
      return std::make_shared<ProjectNode>(mte);
  }

std::shared_ptr<ProjectNode> createDirectTrackNode(std::vector<adm::ElementConstVariant> el, std::shared_ptr<TrackElement> parentTrack)
  {
      auto track = std::make_shared<DirectTrack>(el, parentTrack);
      return std::make_shared<ProjectNode>(track);
  }

    std::shared_ptr<ProjectNode> createHoaTrackNode(std::vector<adm::ElementConstVariant> el, std::shared_ptr<TrackElement> parentTrack)
    {
        auto track = std::make_shared<DirectTrack>(el, parentTrack);
        return std::make_shared<ProjectNode>(track);
    }
    std::shared_ptr<ProjectNode> createGroupNode(std::vector<adm::ElementConstVariant> el, std::shared_ptr<TrackElement> parentTrack)
  {
        return std::make_shared<ProjectNode>(std::make_unique<GroupTrack>(el, parentTrack, std::make_unique<TrackGroup>(0)));
  }

    std::shared_ptr<ProjectNode> createTakeNode(std::shared_ptr<const adm::AudioObject> el,
        std::shared_ptr<TrackElement> parentTrack,
                                              std::shared_ptr<const adm::AudioTrackUid> uid)
  {
      return std::make_shared<ProjectNode>(std::make_unique<MediaTakeElement>(el, parentTrack, nullptr));
  }

  std::shared_ptr<ProjectNode> createAutomationNode(ADMChannel channel,
                                                    std::shared_ptr<TakeElement> parentTake)
  {
      return std::make_shared<ProjectNode>(std::make_unique<ObjectAutomationElement>(channel, parentTake));
  }
};

class MockNodeFactory : public NodeFactory {
public:
  MOCK_METHOD2(createObjectTrackNode,
      std::shared_ptr<ProjectNode>(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentTrack));
  MOCK_METHOD2(createDirectTrackNode,
      std::shared_ptr<ProjectNode>(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentTrack));
  MOCK_METHOD2(createHoaTrackNode,
      std::shared_ptr<ProjectNode>(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentTrack));
  MOCK_METHOD2(createGroupNode,
      std::shared_ptr<ProjectNode>(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentTrack));
  MOCK_METHOD3(createTakeNode,
      std::shared_ptr<ProjectNode>(std::shared_ptr<const adm::AudioObject> object, std::shared_ptr<TrackElement> parentTrack, std::shared_ptr<adm::AudioTrackUid const> trackUid));
  MOCK_METHOD2(createAutomationNode,
      std::shared_ptr<ProjectNode>(ADMChannel channel, std::shared_ptr<TakeElement> parentTake));
  void delegateToFake() {
      ON_CALL(*this, createObjectTrackNode(_, _)).WillByDefault(Invoke(&fake, &FakeCreator::createObjectTrackNode));
      ON_CALL(*this, createDirectTrackNode(_, _)).WillByDefault(Invoke(&fake, &FakeCreator::createDirectTrackNode));
      ON_CALL(*this, createHoaTrackNode(_, _)).WillByDefault(Invoke(&fake, &FakeCreator::createHoaTrackNode));
      ON_CALL(*this, createGroupNode(_, _)).WillByDefault(Invoke(&fake, &FakeCreator::createGroupNode));
      ON_CALL(*this, createTakeNode(_, _, _)).WillByDefault(Invoke(&fake, &FakeCreator::createTakeNode));
      ON_CALL(*this, createAutomationNode(_, _)).WillByDefault(Invoke(&fake, &FakeCreator::createAutomationNode));
  }
private:
  FakeCreator fake;

};

}  // namespace admplug
