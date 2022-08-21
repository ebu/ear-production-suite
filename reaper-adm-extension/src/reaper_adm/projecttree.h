#pragma once
#include <memory>
#include <boost/variant/static_visitor.hpp>
#include <adm/element_variant.hpp>
#include "pluginsuite.h"
#include "projectnode.h"

namespace adm {
  class AudioObject;
  class AudioChannelFormat;
}

namespace admplug {

class ImportListener;
class NodeFactory;
class IPCMSourceCreator;
class ReaperAPI;

enum PackRepresentation {
    Undefined,
    SingleMultichannelTrack,
    SingleMonoTrack,
    MultipleGroupedMonoTracks
};

struct TreeState {
    std::shared_ptr<ProjectNode> currentNode;

    std::shared_ptr<adm::AudioProgramme const> currentProgramme;
    std::shared_ptr<adm::AudioContent const> currentContent;
    std::shared_ptr<adm::AudioObject const> currentObject;
    std::shared_ptr<adm::AudioPackFormat const> rootPack;
    std::vector<std::shared_ptr<adm::AudioTrackUid const>> audioTrackUids;
    std::vector<std::shared_ptr<adm::AudioChannelFormat const>> audioChannelFormats;

    PackRepresentation packRepresentation;
};

class ProjectTree : boost::static_visitor<>
{
public:
    ProjectTree(std::unique_ptr<NodeFactory> nodeFactory,
                std::shared_ptr<IPCMSourceCreator> sourceCreator,
                std::shared_ptr<ProjectNode> root,
                std::shared_ptr<ImportListener> broadcast);
    void operator()(std::shared_ptr<adm::AudioProgramme const> programme);
    void operator()(std::shared_ptr<adm::AudioContent const> content);
    void operator()(std::shared_ptr<adm::AudioObject const> object);
    void operator()(std::shared_ptr<adm::AudioPackFormat const> packFormat);
    template <typename T>
    void operator()(std::shared_ptr<T const>) {}
    void resetRoot();
    TreeState getState();
    void setState(TreeState state);
    void create(PluginSuite &pluginSet,
                ReaperAPI const& api);
private:
    void moveToNewTakeNode(std::shared_ptr<const adm::AudioObject> object);
    void moveToTakeNode();
    void addTake();
    bool moveToNewTrackNode(adm::TypeDescriptor td, std::shared_ptr<const adm::AudioObject> representedAudioObject, std::vector<adm::ElementConstVariant> elements);
    void moveToNewObjectTrackNode(std::shared_ptr<const adm::AudioObject> representedAudioObject, std::vector<adm::ElementConstVariant> elements);
    void moveToNewDirectTrackNode(std::shared_ptr<const adm::AudioObject> representedAudioObject, std::vector<adm::ElementConstVariant> elements);
    void moveToNewHoaTrackNode(std::shared_ptr<const adm::AudioObject> representedAudioObject, std::vector<adm::ElementConstVariant> elements);
    void moveToNewGroupNode(adm::ElementConstVariant element);
    void moveToNewGroupNode(std::vector<adm::ElementConstVariant> elements);
    void addAutomationNodes(std::shared_ptr<ProjectNode> trackNode);
    void addAutomation();
    bool moveToChildWithElement(adm::ElementConstVariant element);
    bool moveToTrackNodeWithElement(adm::ElementConstVariant element);
    bool moveToTrackNodeWithElements(std::vector<adm::ElementConstVariant> elements);
    bool moveToCompatibleTakeNode(std::shared_ptr<const adm::AudioObject> object, std::vector<std::shared_ptr<adm::AudioTrackUid const>> const& elements);
    void moveToNewChild(std::shared_ptr<ProjectNode> child);
    std::shared_ptr<ProjectNode> getChildWithElement(adm::ElementConstVariant element);
    std::shared_ptr<ProjectNode> getTrackNodeWithElements(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<ProjectNode> startingNode = nullptr);
    std::shared_ptr<ProjectNode> getCompatibleTakeNode(std::shared_ptr<const adm::AudioObject> object, std::vector<std::shared_ptr<adm::AudioTrackUid const>> const& elements, std::shared_ptr<ProjectNode> startingNode = nullptr);

    std::unique_ptr<NodeFactory> nodeFactory;
    std::shared_ptr<IPCMSourceCreator> sourceCreator;
    std::shared_ptr<ProjectNode> rootNode;
    std::shared_ptr<ImportListener> broadcast;
    TreeState state;
};

}
