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
std::shared_ptr<ProjectNode> createObjectTrackNode(std::shared_ptr<const adm::AudioObject> representedAudioObject, std::shared_ptr<const adm::AudioTrackUid> representedAudioTrackUid, std::vector<adm::ElementConstVariant> el, std::shared_ptr<TrackElement> parentTrack)
{
    auto track = std::make_shared<ObjectTrack>(el, parentTrack);
    track->setRepresentedAudioObject(representedAudioObject);
    track->setRepresentedAudioTrackUid(representedAudioTrackUid);
    return std::make_shared<ProjectNode>(track);
}

std::shared_ptr<ProjectNode> createDirectTrackNode(std::shared_ptr<const adm::AudioObject> representedAudioObject, std::vector<adm::ElementConstVariant> el, std::shared_ptr<TrackElement> parentTrack)
{
    auto track = std::make_shared<DirectTrack>(el, parentTrack);
    track->setRepresentedAudioObject(representedAudioObject);
    return std::make_shared<ProjectNode>(track);
}

std::shared_ptr<ProjectNode> createHoaTrackNode(std::shared_ptr<const adm::AudioObject> representedAudioObject, std::vector<adm::ElementConstVariant> el, std::shared_ptr<TrackElement> parentTrack)
{
    auto track = std::make_shared<HoaTrack>(el, parentTrack);
    track->setRepresentedAudioObject(representedAudioObject);
    return std::make_shared<ProjectNode>(track);
}

std::shared_ptr<ProjectNode> createGroupNode(std::vector<adm::ElementConstVariant> el, std::shared_ptr<TrackElement> parentTrack)
{
    return std::make_shared<ProjectNode>(std::make_unique<GroupTrack>(el, parentTrack, std::make_unique<TrackGroup>(0)));
}

std::shared_ptr<ProjectNode> createTakeNode(std::shared_ptr<const adm::AudioObject> el,
    std::shared_ptr<TrackElement> parentTrack)
{
    return std::make_shared<ProjectNode>(std::make_unique<MediaTakeElement>(el, parentTrack));
}

std::shared_ptr<ProjectNode> createAutomationNode(ADMChannel channel,
                                                std::shared_ptr<TrackElement> parentTrack,
                                                std::shared_ptr<TakeElement> parentTake)
{
    return std::make_shared<ProjectNode>(std::make_unique<ObjectAutomationElement>(channel, parentTrack, parentTake));
}
};

class MockNodeFactory : public NodeFactory {
public:
  MOCK_METHOD4(createObjectTrackNode,
      std::shared_ptr<ProjectNode>(std::shared_ptr<const adm::AudioObject> representedAudioObject, std::shared_ptr<const adm::AudioTrackUid> representedAudioTrackUid, std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentTrack));
  MOCK_METHOD3(createDirectTrackNode,
      std::shared_ptr<ProjectNode>(std::shared_ptr<const adm::AudioObject> representedAudioObject, std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentTrack));
  MOCK_METHOD3(createHoaTrackNode,
      std::shared_ptr<ProjectNode>(std::shared_ptr<const adm::AudioObject> representedAudioObject, std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentTrack));
  MOCK_METHOD2(createGroupNode,
      std::shared_ptr<ProjectNode>(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentTrack));
  MOCK_METHOD2(createTakeNode,
      std::shared_ptr<ProjectNode>(std::shared_ptr<const adm::AudioObject> object, std::shared_ptr<TrackElement> parentTrack));
  MOCK_METHOD3(createAutomationNode,
      std::shared_ptr<ProjectNode>(ADMChannel channel, std::shared_ptr<TrackElement> parentTrack, std::shared_ptr<TakeElement> parentTake));
  void delegateToFake() {
      ON_CALL(*this, createObjectTrackNode(_, _, _, _)).WillByDefault(Invoke(&fake, &FakeCreator::createObjectTrackNode));
      ON_CALL(*this, createDirectTrackNode(_, _, _)).WillByDefault(Invoke(&fake, &FakeCreator::createDirectTrackNode));
      ON_CALL(*this, createHoaTrackNode(_, _, _)).WillByDefault(Invoke(&fake, &FakeCreator::createHoaTrackNode));
      ON_CALL(*this, createGroupNode(_, _)).WillByDefault(Invoke(&fake, &FakeCreator::createGroupNode));
      ON_CALL(*this, createTakeNode(_, _)).WillByDefault(Invoke(&fake, &FakeCreator::createTakeNode));
      ON_CALL(*this, createAutomationNode(_, _, _)).WillByDefault(Invoke(&fake, &FakeCreator::createAutomationNode));
  }
private:
  FakeCreator fake;

};

}  // namespace admplug
