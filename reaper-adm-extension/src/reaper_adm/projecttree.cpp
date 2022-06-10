#include <adm/adm.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <algorithm>

#include "projecttree.h"
#include "projectelements.h"
#include "reaperapi.h"
#include "nodefactory.h"
#include "admchannel.h"
#include "importaction.h"

using namespace admplug;

namespace {

bool shouldCreateGroup(adm::AudioProgramme const& programme) {
    auto doc = programme.getParent().lock();
    assert(doc);
    return doc->getElements<adm::AudioProgramme>().size() > 1;
}

bool shouldCreateGroup(adm::AudioContent const& content) {
    auto doc = content.getParent().lock();
    assert(doc);

    auto contentId = content.get<adm::AudioContentId>();
    auto programmes = doc->getElements<adm::AudioProgramme>();

    for (auto programme : programmes) {
        auto progContents = programme->getReferences<adm::AudioContent>();
        for (auto progContent : progContents) {
            if (contentId == progContent->get<adm::AudioContentId>()) {
                if (progContents.size() > 1) return true;
            }
        }
    }

    return false;
}

template<typename ParentT>
bool objectHasMultipleParents(adm::AudioObject const& object, adm::Document const& doc) {
    auto targetId = object.get<adm::AudioObjectId>();
    auto parents = doc.getElements<ParentT>();
    auto refCount = 0;
    for(auto element : parents) {
        auto objectRefs = element->template getReferences<adm::AudioObject>();
        for(auto ref : objectRefs) {
            if(ref->template get<adm::AudioObjectId>() == targetId) {
                refCount++;
                if(refCount > 1) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool objectHasMultipleParents(adm::AudioObject const& object, adm::Document const& doc) {
    return objectHasMultipleParents<adm::AudioObject>(object, doc) ||
        objectHasMultipleParents<adm::AudioContent>(object, doc);
}

bool shouldCreateGroup(adm::AudioObject const& object) {
    // If this object has child that has multiple parents, each parent should create a group to show these associations.
    // Future possiblility/User option?: Whether just having multiple children is also a sufficient reason to create a group.
    auto doc = object.getParent().lock();
    assert(doc);
    auto childObjects = object.getReferences<adm::AudioObject>();
    for (auto childObject : childObjects) {
        if(objectHasMultipleParents(*childObject, *doc)) return true;
    }

    return false;
}

bool packFormatIsObjectType(adm::AudioPackFormat const& pack) {
    return pack.has<adm::TypeDescriptor>() && pack.get<adm::TypeDescriptor>() == adm::TypeDefinition::OBJECTS;
}

int getFlattenedPackFormatChannelCount(adm::AudioPackFormat const& pack) {
    int chCount = static_cast<int>(pack.getReferences<adm::AudioChannelFormat>().size());
    auto childPacks = pack.getReferences<adm::AudioPackFormat>();
    for (auto childPack : childPacks) {
        chCount += getFlattenedPackFormatChannelCount(*childPack);
    }
    return chCount;
}

PackRepresentation getPackRepresentation(adm::AudioPackFormat const& pack)
{
    if (getFlattenedPackFormatChannelCount(pack) > 1) {
        if (packFormatIsObjectType(pack)) {
            return PackRepresentation::MultipleGroupedMonoTracks;
        }
        else {
            return PackRepresentation::SingleMultichannelTrack;
        }
    }
    else {
        return PackRepresentation::SingleMonoTrack;
    }
}

bool nodeIsTrackWithElements(ProjectNode const& node, std::vector<adm::ElementConstVariant> const& elements) {
    return std::dynamic_pointer_cast<TrackElement>(node.getProjectElement()) && node.getProjectElement()->hasAdmElements(elements);
}

}


ProjectTree::ProjectTree(std::unique_ptr<NodeFactory> nodeFactory,
                         std::shared_ptr<ProjectNode> root,
                         std::shared_ptr<ImportListener> broadcast) : nodeFactory{std::move(nodeFactory)},
    rootNode{std::move(root)},
    broadcast{std::move(broadcast)},
    state{rootNode,
          nullptr,
          nullptr,
          nullptr,
          nullptr,
          nullptr,
          nullptr,
          PackRepresentation::Undefined}
{
}

void ProjectTree::resetRoot()
{
    state.currentNode = rootNode;

    state.currentProgramme = nullptr;
    state.currentContent = nullptr;
    state.currentObject = nullptr;
    state.rootPack = nullptr;
    state.currentPack = nullptr;
    state.currentUid = nullptr;

    state.packRepresentation = PackRepresentation::Undefined;
}

void ProjectTree::operator()(std::shared_ptr<const adm::AudioProgramme> programme)
{
    state.currentProgramme = programme;
    if (!moveToChildWithElement(programme)) {
        if (shouldCreateGroup(*programme)) {
            moveToNewGroupNode(programme);
        }
    }
}

void ProjectTree::operator()(std::shared_ptr<const adm::AudioContent> content)
{
    state.currentContent = content;
    if (!moveToChildWithElement(content)) {
        if (shouldCreateGroup(*content)) {
            moveToNewGroupNode(content);
        }
    }
}

void ProjectTree::operator()(std::shared_ptr<const adm::AudioObject> object)
{
    state.currentObject = object;
    if (!moveToTrackNodeWithElement(object)) {
        if (shouldCreateGroup(*object)) {
            moveToNewGroupNode(object);
        }
    }
    if (object->getReferences<adm::AudioPackFormat>().size() == 1) {
        (*this)(object->getReferences<adm::AudioPackFormat>().front());
    }
}

void ProjectTree::operator()(std::shared_ptr<const adm::AudioTrackUid> trackUid)
{
    state.currentUid = trackUid;
    state.currentPack = trackUid->getReference<adm::AudioPackFormat>();
}

void ProjectTree::operator()(std::shared_ptr<const adm::AudioChannelFormat> channelFormat)
{
    if(state.packRepresentation == PackRepresentation::Undefined) {
        return;
    }
    auto channel = ADMChannel{state.currentObject, channelFormat, state.rootPack, state.currentUid};

    if (state.packRepresentation == PackRepresentation::SingleMultichannelTrack) {
        std::vector<adm::ElementConstVariant> relatedAdmElements{ state.currentObject, state.rootPack };
        if (!moveToTrackNodeWithElements(relatedAdmElements)) {
            moveToNewTrackAndTakeNode(channel, relatedAdmElements);
        } else {
            assert(!state.currentNode->children().empty());
            state.currentNode = state.currentNode->children().front();
            auto take = std::dynamic_pointer_cast<TakeElement>(state.currentNode->getProjectElement());
            assert(take);
            if(take && !take->hasChannel(channel)) { // We may have traversed to this via different routes, so check!
                take->addChannel(channel);
            }
        }
        moveToNewAutomationNode(channel);
    }

    else if (state.packRepresentation == PackRepresentation::MultipleGroupedMonoTracks) {
        std::vector<adm::ElementConstVariant> relatedAdmElements{ state.currentObject, state.rootPack };
        if (!moveToTrackNodeWithElements(relatedAdmElements)) {
            moveToNewGroupNode(relatedAdmElements);
        }
        relatedAdmElements.push_back(state.currentUid);
        if (!moveToTrackNodeWithElements(relatedAdmElements)) {
            moveToNewTrackAndTakeNode(channel, relatedAdmElements);
            moveToNewAutomationNode(channel);
        }
    }
    else if (state.packRepresentation == PackRepresentation::SingleMonoTrack) {
        std::vector<adm::ElementConstVariant> relatedAdmElements{ state.currentObject, state.rootPack, state.currentUid };
        if (!moveToTrackNodeWithElements(relatedAdmElements)) {
            moveToNewTrackAndTakeNode(channel, relatedAdmElements);
            moveToNewAutomationNode(channel);
        }
    }
}

void ProjectTree::operator()(std::shared_ptr<const adm::AudioPackFormat> packFormat)
{
    state.rootPack = packFormat;
    state.packRepresentation = getPackRepresentation(*state.rootPack);
}


TreeState ProjectTree::getState()
{
    return state;
}

void ProjectTree::setState(TreeState newState)
{
    state = newState;
    if(!state.currentNode) {
        state.currentNode = rootNode;
    }
}

void ProjectTree::create(PluginSuite& pluginSet,
                         const ReaperAPI &api)
{
    pluginSet.onCreateProject(*rootNode, api);
    rootNode->createProjectElements(pluginSet, api, *broadcast);

    api.Main_OnCommandEx(ReaperAPI::BUILD_MISSING_PEAKS, 0, nullptr);

    api.UpdateArrangeForAutomation();
}

void ProjectTree::moveToNewTrackAndTakeNode(ADMChannel channel,
                                            std::vector<adm::ElementConstVariant> relatedAdmElements) {
    auto channelFormat = channel.channelFormat();

    if(channelFormat) {
        auto td = channelFormat->get<adm::TypeDescriptor>();
        if(td == adm::TypeDefinition::OBJECTS) {
            moveToNewObjectTrackNode(relatedAdmElements);
        } else if(td == adm::TypeDefinition::HOA) {
            moveToNewHoaTrackNode(relatedAdmElements);
        } else if(td == adm::TypeDefinition::DIRECT_SPEAKERS){
            moveToNewDirectTrackNode(relatedAdmElements);
        }

        moveToNewTakeNode(state.currentObject, state.currentUid);
        auto take = std::static_pointer_cast<TakeElement>(state.currentNode->getProjectElement());
        if(take && !take->hasChannel(channel)) { // We may have traversed to this via different routes, so check!
            take->addChannel(channel);
        }
    }
}


void ProjectTree::moveToNewTakeNode(std::shared_ptr<const adm::AudioObject> admObjectElement, std::shared_ptr<const adm::AudioTrackUid> trackUid)
{
    auto parentTrack = std::static_pointer_cast<TrackElement>(state.currentNode->getProjectElement());
    assert(std::dynamic_pointer_cast<TrackElement>(parentTrack)); // Has to have parent
    auto takeNode = nodeFactory->createTakeNode(admObjectElement, parentTrack, trackUid);
    assert(takeNode);
    broadcast->elementAdded();
    moveToNewChild(takeNode);
}

void ProjectTree::moveToNewObjectTrackNode(std::vector<adm::ElementConstVariant> elements)
{
    auto parentTrack = std::dynamic_pointer_cast<TrackElement>(state.currentNode->getProjectElement());
    auto trackNode = nodeFactory->createObjectTrackNode(elements, parentTrack);
    broadcast->elementAdded();
    moveToNewChild(trackNode);
}

void ProjectTree::moveToNewDirectTrackNode(std::vector<adm::ElementConstVariant> elements)
{
    auto parentTrack = std::dynamic_pointer_cast<TrackElement>(state.currentNode->getProjectElement());
    auto trackNode = nodeFactory->createDirectTrackNode(elements, parentTrack);
    broadcast->elementAdded();
    moveToNewChild(trackNode);
}

void ProjectTree::moveToNewHoaTrackNode(std::vector<adm::ElementConstVariant> elements)
{
    auto parentTrack = std::dynamic_pointer_cast<TrackElement>(state.currentNode->getProjectElement());
    auto trackNode = nodeFactory->createHoaTrackNode(elements, parentTrack);
    moveToNewChild(trackNode);
}

void ProjectTree::moveToNewGroupNode(adm::ElementConstVariant element)
{
    moveToNewGroupNode(std::vector<adm::ElementConstVariant>(1, element));
}
void ProjectTree::moveToNewGroupNode(std::vector<adm::ElementConstVariant> elements)
{
    auto parentTrack = std::dynamic_pointer_cast<TrackElement>(state.currentNode->getProjectElement());
    auto trackNode = nodeFactory->createGroupNode(elements, parentTrack);
    assert(trackNode);
    broadcast->elementAdded();
    moveToNewChild(trackNode);
}

void ProjectTree::moveToNewAutomationNode(ADMChannel channel)
{
    auto parentTake = std::static_pointer_cast<TakeElement>(state.currentNode->getProjectElement());
    assert(std::dynamic_pointer_cast<TakeElement>(parentTake)); // Has to have parent
    auto autoNode = nodeFactory->createAutomationNode(channel, parentTake);
    assert(autoNode);
    broadcast->elementAdded();
    moveToNewChild(autoNode);
}

std::shared_ptr<ProjectNode> ProjectTree::getChildWithElement(adm::ElementConstVariant element) {
    if(state.currentNode) {
    	return state.currentNode->getChildWithElement(element);
    }
    return nullptr;
}

std::shared_ptr<ProjectNode> admplug::ProjectTree::getTrackNodeWithElements(std::vector<adm::ElementConstVariant> elements, std::shared_ptr<ProjectNode> startingNode)
{
    if (!startingNode) {
        startingNode = rootNode;
    }
    if (nodeIsTrackWithElements(*startingNode, elements)) {
        return startingNode;
    }
    auto children = startingNode->children();
    for (auto child : children) {
        if (std::dynamic_pointer_cast<TrackElement>(child->getProjectElement())) { // Only process child if track node (i.e, track/group)!
            auto matchingNode = getTrackNodeWithElements(elements, child);
            if (matchingNode) {
                return matchingNode;
            }
        }
    }
    return nullptr;
}

bool ProjectTree::moveToChildWithElement(adm::ElementConstVariant element) {
    auto existingChild = getChildWithElement(element);
    if(existingChild) {
        state.currentNode = existingChild;
        moveToChildWithElement(element);
        return true;
    }
    return false;
}

bool admplug::ProjectTree::moveToTrackNodeWithElement(adm::ElementConstVariant element) {
    return moveToTrackNodeWithElements(std::vector<adm::ElementConstVariant>(1, element));
}

bool admplug::ProjectTree::moveToTrackNodeWithElements(std::vector<adm::ElementConstVariant> elements)
{
    auto existingNode = getTrackNodeWithElements(elements);
    if (existingNode) {
        moveToNewChild(existingNode);
        return true;
    }
    return false;
}

void ProjectTree::moveToNewChild(std::shared_ptr<ProjectNode> child)
{
    auto success = state.currentNode->addChildNode(child);
    assert(success);
    state.currentNode = child;
}
