#include "nodefactory.h"
#include "pcmsourcecreator.h"
#include "mediatakeelement.h"
#include "mediatrackelement.h"
#include "importelement.h"
#include "objectautomationelement.h"
#include "directspeakerautomationelement.h"
#include "hoaautomationelement.h"
#include "projectnode.h"

using namespace admplug;

NodeCreator::NodeCreator(std::shared_ptr<IPCMSourceCreator> pcmCreator, MediaItem* fromMediaItem) :
    pcmCreator{std::move(pcmCreator)},
    originalMediaItem{ fromMediaItem }
{
}

std::unique_ptr<RootElement> admplug::NodeCreator::createRootElement(MediaItem* fromMediaItem) {
    return std::make_unique<ImportElement>(fromMediaItem);
}

std::shared_ptr<ProjectNode> admplug::NodeCreator::createObjectTrackNode(std::shared_ptr<const adm::AudioObject> representedAudioObject, std::shared_ptr<const adm::AudioTrackUid> representedAudioTrackUid, std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentGroupTrack)
{
    auto track = std::make_shared<ObjectTrack>(elements, parentGroupTrack);
    track->setRepresentedAudioObject(representedAudioObject);
    track->setRepresentedAudioTrackUid(representedAudioTrackUid);
    return std::make_shared<ProjectNode>(track);
}

std::shared_ptr<ProjectNode> NodeCreator::createDirectTrackNode(std::shared_ptr<const adm::AudioObject> representedAudioObject, std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentGroupTrack)
{
    auto track = std::make_shared<DirectTrack>(elements, parentGroupTrack);
    track->setRepresentedAudioObject(representedAudioObject);
    return std::make_shared<ProjectNode>(track);
}

std::shared_ptr<ProjectNode> NodeCreator::createHoaTrackNode(std::shared_ptr<const adm::AudioObject> representedAudioObject, std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentGroupTrack)
{
    auto track = std::make_shared<HoaTrack>(elements, parentGroupTrack);
    track->setRepresentedAudioObject(representedAudioObject);
    return std::make_shared<ProjectNode>(track);
}

std::shared_ptr<ProjectNode> admplug::NodeCreator::createGroupNode(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentGroupTrack)
{
    auto track = std::make_unique<GroupTrack>(elements, parentGroupTrack, std::make_unique<TrackGroup>(currentGroup));
    currentGroup += 1;
    return std::make_shared<ProjectNode>(std::move(track));
}

std::shared_ptr<ProjectNode> NodeCreator::createTakeNode(std::shared_ptr<const adm::AudioObject> parentObject, std::shared_ptr<TrackElement> parentTrack)
{
    auto projectElement = std::make_shared<MediaTakeElement>(parentObject, parentTrack, originalMediaItem);
    pcmCreator->addTake(projectElement);
    auto projectNode = std::make_shared<ProjectNode>(projectElement);
    return projectNode;
}

std::shared_ptr<ProjectNode> admplug::NodeCreator::createAutomationNode(ADMChannel channel, std::shared_ptr<TrackElement> parentTrack, std::shared_ptr<TakeElement> parentTake)
{
    auto channelFormat = channel.channelFormat();
    if(channelFormat) {
        if(!channelFormat->getElements<adm::AudioBlockFormatDirectSpeakers>().empty()) {
            return std::make_shared<ProjectNode>(std::make_unique<DirectSpeakersAutomationElement>(channel, parentTrack, parentTake));
        }
        if(!channelFormat->getElements<adm::AudioBlockFormatHoa>().empty()) {
            return std::make_shared<ProjectNode>(std::make_unique<HoaAutomationElement>(channel, parentTrack, parentTake));
        }
    }

    // if no blocks default to object track
    return std::make_shared<ProjectNode>(std::make_unique<ObjectAutomationElement>(channel, parentTrack, parentTake));

}
