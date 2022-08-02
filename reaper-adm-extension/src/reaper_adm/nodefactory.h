#ifndef NODEFACTORY_H
#define NODEFACTORY_H
#include <memory>
#include <adm/element_variant.hpp>
#include "reaperapi.h"
#include "admchannel.h"

namespace admplug {

class ProjectNode;

class RootElement;
class TrackElement;
class TakeElement;
class ObjectAutomation;
class MediaTrackElement;

class IPCMSourceCreator;

class NodeFactory
{
public:
  NodeFactory() = default;
  virtual ~NodeFactory() = default;
/**
 * @brief NodeFactory::addTrackNode
 * @param element
 * @return created node
 * Creates and returns a new TrackNode, which represents a Reaper MediaTrack.
 * On creation, the track will be named after the supplied element.
 */
  virtual std::shared_ptr<ProjectNode> createObjectTrackNode(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentGroupTrack) = 0;
  virtual std::shared_ptr<ProjectNode> createDirectTrackNode(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentGroupTrack) = 0;
  virtual std::shared_ptr<ProjectNode> createHoaTrackNode(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentGroupTrack) = 0;
  virtual std::shared_ptr<ProjectNode> createGroupNode(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentGroupTrack) = 0;
/**
 * @brief NodeFactory::createTakeNode
 * @param object
 * @param parent
 * @param trackUid
 * @return created node
 * Creates and returns a new TakeNode, which represents a Reaper MediaTake.
 * If A trackUID is supplied, the node will represent a single channel node with the referenced track.
 * If trackUid == nullptr, the take will reference all of the tracks referenced by uids in the supplied object.
 * On creation, the take will have length and position derived from the supplied object
 */
  virtual std::shared_ptr<ProjectNode> createTakeNode(std::shared_ptr<const adm::AudioObject> object,
                                                    std::shared_ptr<TrackElement> parentTrack) = 0;
  virtual std::shared_ptr<ProjectNode> createAutomationNode(ADMChannel channel,
                                                    std::shared_ptr<TrackElement> parentTrack,
                                                    std::shared_ptr<TakeElement> parentTake) = 0;
};

class NodeCreator : public NodeFactory {
public:
    NodeCreator(std::shared_ptr<IPCMSourceCreator> pcmCreator, MediaItem* fromMediaItem = nullptr);
    NodeCreator(NodeFactory const& other) = delete;
    NodeCreator& operator=(NodeFactory const& other) = delete;
    std::shared_ptr<ProjectNode> createObjectTrackNode(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentGroupTrack) override;
    std::shared_ptr<ProjectNode> createDirectTrackNode(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentGroupTrack) override;
    std::shared_ptr<ProjectNode> createHoaTrackNode(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentGroupTrack) override;
    std::shared_ptr<ProjectNode> createGroupNode(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<TrackElement> parentGroupTrack) override;
    std::shared_ptr<ProjectNode> createTakeNode(std::shared_ptr<const adm::AudioObject> parentObject,
                                                std::shared_ptr<TrackElement> parentTrack) override;
    std::shared_ptr<ProjectNode> createAutomationNode(ADMChannel channel,
                                                      std::shared_ptr<TrackElement> parentTrack,
                                                      std::shared_ptr<TakeElement> parentTake) override;
    std::unique_ptr<RootElement> createRootElement(MediaItem *fromMediaItem);
private:
    std::shared_ptr<IPCMSourceCreator> pcmCreator;
    MediaItem* originalMediaItem;
    int currentGroup{0};

};

}

#endif // NODEFACTORY_H
