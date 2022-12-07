#include <adm/adm.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <algorithm>

#include "projecttree.h"
#include "projectelements.h"
#include "reaperapi.h"
#include "nodefactory.h"
#include "admchannel.h"
#include "importaction.h"
#include "pcmsourcecreator.h"
#include "mediatakeelement.h"
#include <helper/container_helpers.hpp>
#include <helper/common_definition_helper.h>

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
    if(packFormatIsObjectType(pack) && getFlattenedPackFormatChannelCount(pack) > 1) {
        return PackRepresentation::MultipleGroupedTracks;
    }
    return PackRepresentation::SingleTrack;
}

bool nodeIsTrackWithElements(ProjectNode const& node, std::vector<adm::ElementConstVariant> const& elements) {
    return std::dynamic_pointer_cast<TrackElement>(node.getProjectElement()) && node.getProjectElement()->hasAdmElements(elements);
}

bool nodeIsTakeWithCompatibleElements(ProjectNode const& node, std::vector<uint32_t> const& channelsOfOriginal, std::shared_ptr<const adm::AudioObject> object) {
    auto take = std::dynamic_pointer_cast<MediaTakeElement>(node.getProjectElement());
    if(!take) return false;
    // Check start and duration
    if(object->has<adm::Start>() != take->getAudioObject()->has<adm::Start>()) {
        return false;
    } else if(object->has<adm::Start>() && object->get<adm::Start>().get() != take->getAudioObject()->get<adm::Start>().get()) {
        return false;
    }
    if(object->has<adm::Duration>() != take->getAudioObject()->has<adm::Duration>()) {
        return false;
    } else if(object->has<adm::Duration>() && object->get<adm::Duration>().get() != take->getAudioObject()->get<adm::Duration>().get()) {
        return false;
    }
    // Check channels
    int lim = std::min(take->channelsOfOriginal().size(), channelsOfOriginal.size());
    for(int i = 0; i < lim; i++) {
        if(take->channelsOfOriginal()[i] != channelsOfOriginal[i]) {
            return false;
        }
    }
    return true;
}

bool isDescendantOf(std::shared_ptr<const adm::AudioChannelFormat> cf, std::shared_ptr<const adm::AudioPackFormat> pf) {
    if(contains(pf->getReferences<adm::AudioChannelFormat>(), cf)) return true;
    for(auto childPf : pf->getReferences<adm::AudioPackFormat>()) {
        if(isDescendantOf(cf, childPf)) return true;
    }
    return false;
}

void recursePackFormatsForChannelFormats(std::shared_ptr<const adm::AudioPackFormat> fromPackFormat, std::vector<std::shared_ptr<const adm::AudioChannelFormat>> &cfOrder) {
    // Do other pack formats first (these channels will want to appear before the ones directly in this pack format)
    auto subPackFormats = fromPackFormat->getReferences<adm::AudioPackFormat>();
    for (auto subPackFormat : subPackFormats) {
        recursePackFormatsForChannelFormats(subPackFormat, cfOrder);
    }
    // Find Channel formats
    auto channelFormats = fromPackFormat->getReferences<adm::AudioChannelFormat>();
    for (auto cf : channelFormats) {
        cfOrder.push_back(cf);
    }
}

void sortToVectorsByChannelFormatOrder(const std::map<std::shared_ptr<const adm::AudioChannelFormat>, std::shared_ptr<const adm::AudioTrackUid>> &pairs,
                                       std::vector<std::shared_ptr<const adm::AudioChannelFormat>> &cfOut,
                                       std::vector<std::shared_ptr<const adm::AudioTrackUid>> &uidOut,
                                       std::shared_ptr<const adm::AudioPackFormat> pf) {

    // First, work out the order the CF's should be in
    std::vector<std::shared_ptr<const adm::AudioChannelFormat>> cfOrder;
    recursePackFormatsForChannelFormats(pf, cfOrder);

    for(auto cf : cfOrder) {
        cfOut.push_back(cf);
        auto it = pairs.find(cf);
        if(it != pairs.end()) {
            uidOut.push_back(it->second);
        } else {
            uidOut.push_back(nullptr);
        }
    }
}

}


ProjectTree::ProjectTree(std::unique_ptr<NodeFactory> nodeFactory,
                         std::shared_ptr<IPCMSourceCreator> sourceCreator,
                         std::shared_ptr<ProjectNode> root,
                         std::shared_ptr<ImportListener> broadcast) : nodeFactory{ std::move(nodeFactory) },
    rootNode{ std::move(root) },
    sourceCreator{ std::move(sourceCreator) },
    broadcast{ std::move(broadcast) }
{
    resetRoot();
}

void ProjectTree::resetRoot()
{
    state.currentNode = rootNode;

    state.currentProgramme = nullptr;
    state.currentContent = nullptr;
    state.currentObject = nullptr;
    state.rootPack = nullptr;
    state.audioTrackUids.clear();
    state.audioChannelFormats.clear();

    state.packRepresentation = PackRepresentation::Undefined;
}

void ProjectTree::operator()(std::shared_ptr<const adm::AudioProgramme> programme)
{
    state.currentProgramme = programme;
    //Grouping
    if (!moveToChildWithElement(programme)) {
        if (shouldCreateGroup(*programme)) {
            moveToNewGroupNode(programme);
        }
    }
}

void ProjectTree::operator()(std::shared_ptr<const adm::AudioContent> content)
{
    state.currentContent = content;
    //Grouping
    if (!moveToChildWithElement(content)) {
        if (shouldCreateGroup(*content)) {
            moveToNewGroupNode(content);
        }
    }
}

void ProjectTree::operator()(std::shared_ptr<const adm::AudioObject> object)
{
    state.currentObject = object;
    //Grouping
    if (!moveToTrackNodeWithElement(object)) {
        if (shouldCreateGroup(*object)) {
            moveToNewGroupNode(object);
        }
    }
}

void ProjectTree::operator()(std::shared_ptr<const adm::AudioPackFormat> packFormat)
{
    // PackFormat is end of route - collate whatever data we need from here.

    state.rootPack = packFormat;
    auto td = packFormat->get<adm::TypeDescriptor>();
    state.packRepresentation = getPackRepresentation(*state.rootPack);

    if(state.packRepresentation == PackRepresentation::Undefined) return;

    // We need to collect the channel formats and match their ordering to that listed in their parent packformat(s)
    // (Important for matching Common Definitions ordering that the input plugins use for audio channel ordering)
    std::map<std::shared_ptr<const adm::AudioChannelFormat>, std::shared_ptr<const adm::AudioTrackUid>> channels;
    // Visit TrackUIDs
    auto uids = state.currentObject->getReferences<adm::AudioTrackUid>();
    for(auto uid : uids) {
        // Visit associated ChannelFormat
        if(auto tf = uid->getReference<adm::AudioTrackFormat>()) {
            if(auto sf = tf->getReference<adm::AudioStreamFormat>()) {
                if(auto cf = sf->getReference<adm::AudioChannelFormat>()) {
                    channels.emplace(cf, uid);
                }
            }
        }
    }

    if(channels.size() == 0) return;
    sortToVectorsByChannelFormatOrder(channels, state.audioChannelFormats, state.audioTrackUids, packFormat);

    if (state.packRepresentation == PackRepresentation::SingleTrack) {
        // DS, HOA, or single Object

        std::vector<adm::ElementConstVariant> relatedAdmElements{ state.currentObject, state.rootPack };
        if(!moveToTrackNodeWithElements(relatedAdmElements)) {
            if(moveToNewTrackNode(td, state.currentObject, state.audioTrackUids[0], relatedAdmElements)) {
                addTake();
                addAutomation();
            }
        }

    }
    else if (state.packRepresentation == PackRepresentation::MultipleGroupedTracks) {
        // Object type with multiple channels (sub-objects)

        std::vector<adm::ElementConstVariant> relatedAdmElements{ state.currentObject, state.rootPack };
        if (!moveToTrackNodeWithElements(relatedAdmElements)) {
            moveToNewGroupNode(relatedAdmElements);
        }

        auto cachedState = state;
        for(int i = 0; i < state.audioChannelFormats.size(); i++) {
            state = cachedState;
            relatedAdmElements = { state.audioChannelFormats[i], state.currentObject, state.rootPack};
            if(!moveToTrackNodeWithElements(relatedAdmElements)) {
                if(moveToNewTrackNode(td, state.currentObject, state.audioTrackUids[i], relatedAdmElements)) {
                    addTake(i);
                    addAutomation(i);
                }
            }
        }

    }
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

bool admplug::ProjectTree::moveToNewTrackNode(adm::TypeDescriptor td, std::shared_ptr<const adm::AudioObject> representedAudioObject, std::shared_ptr<const adm::AudioTrackUid> representedAudioTrackUid, std::vector<adm::ElementConstVariant> relatedAdmElements)
{
    if(td == adm::TypeDefinition::OBJECTS) {
        moveToNewObjectTrackNode(representedAudioObject, representedAudioTrackUid, relatedAdmElements);
    } else if(td == adm::TypeDefinition::HOA) {
        moveToNewHoaTrackNode(representedAudioObject, relatedAdmElements);
    } else if(td == adm::TypeDefinition::DIRECT_SPEAKERS){
        moveToNewDirectTrackNode(representedAudioObject, relatedAdmElements);
    } else {
        return false;
    }
    return true;
}

void ProjectTree::moveToNewTakeNode(std::shared_ptr<const adm::AudioObject> admObjectElement)
{
    auto parentTrack = std::static_pointer_cast<TrackElement>(state.currentNode->getProjectElement());
    assert(std::dynamic_pointer_cast<TrackElement>(parentTrack)); // Has to have parent
    auto takeNode = nodeFactory->createTakeNode(admObjectElement, parentTrack);
    assert(takeNode);
    broadcast->elementAdded();
    moveToNewChild(takeNode);
}

void admplug::ProjectTree::addTake(int specificAtuIndex)
{
    std::vector<uint32_t> channelsOfOriginal;
    if(specificAtuIndex >= 0 && specificAtuIndex < state.audioTrackUids.size()) {
        auto channel = sourceCreator->channelForTrackUid(state.audioTrackUids[specificAtuIndex]);
        channelsOfOriginal.push_back(channel);
    } else {
        std::for_each(state.audioTrackUids.begin(), state.audioTrackUids.end(),
                      [&channelsOfOriginal, this](std::shared_ptr<adm::AudioTrackUid const> elm) {
            auto channel = this->sourceCreator->channelForTrackUid(elm);
            channelsOfOriginal.push_back(channel);
        });
    }

    auto trackNode = state.currentNode;
    auto trackElement = std::dynamic_pointer_cast<TrackElement>(trackNode->getProjectElement());
    if(!moveToCompatibleTakeNode(state.currentObject, channelsOfOriginal)) {
        moveToNewTakeNode(state.currentObject);
    }
    auto takeElement = std::dynamic_pointer_cast<TakeElement>(state.currentNode->getProjectElement());
    if(trackElement && takeElement) {
        trackElement->setTakeElement(takeElement);
    }
    // Add any additional channels
    for(int i = takeElement->channelCount(); i < channelsOfOriginal.size(); i++) {
        takeElement->addChannelOfOriginal(channelsOfOriginal[i]);
    }
    state.currentNode = trackNode; // Reset pos as this is an add operation, not a move
}

void ProjectTree::moveToNewObjectTrackNode(std::shared_ptr<const adm::AudioObject> representedAudioObject, std::shared_ptr<const adm::AudioTrackUid> representedAudioTrackUid, std::vector<adm::ElementConstVariant> elements)
{
    auto parentTrack = std::dynamic_pointer_cast<TrackElement>(state.currentNode->getProjectElement());
    auto trackNode = nodeFactory->createObjectTrackNode(representedAudioObject, representedAudioTrackUid, elements, parentTrack);
    broadcast->elementAdded();
    moveToNewChild(trackNode);
}

void ProjectTree::moveToNewDirectTrackNode(std::shared_ptr<const adm::AudioObject> representedAudioObject, std::vector<adm::ElementConstVariant> elements)
{
    auto parentTrack = std::dynamic_pointer_cast<TrackElement>(state.currentNode->getProjectElement());
    auto trackNode = nodeFactory->createDirectTrackNode(representedAudioObject, elements, parentTrack);
    broadcast->elementAdded();
    moveToNewChild(trackNode);
}

void ProjectTree::moveToNewHoaTrackNode(std::shared_ptr<const adm::AudioObject> representedAudioObject, std::vector<adm::ElementConstVariant> elements)
{
    auto parentTrack = std::dynamic_pointer_cast<TrackElement>(state.currentNode->getProjectElement());
    auto trackNode = nodeFactory->createHoaTrackNode(representedAudioObject, elements, parentTrack);
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

void admplug::ProjectTree::addAutomation(int specificAtuAcfIndex)
{
    auto parentTrack = std::static_pointer_cast<TrackElement>(state.currentNode->getProjectElement());
    auto parentTake = std::static_pointer_cast<TakeElement>(parentTrack->getTakeElement());
    int start = (specificAtuAcfIndex >= 0 && specificAtuAcfIndex < state.audioChannelFormats.size()) ? specificAtuAcfIndex : 0;
    int end = (specificAtuAcfIndex >= 0 && specificAtuAcfIndex < state.audioChannelFormats.size()) ? specificAtuAcfIndex + 1 : state.audioChannelFormats.size();
    for(int i = start; i < end; i++) {
        ADMChannel channel{ state.currentObject, state.audioChannelFormats[i], state.rootPack, state.audioTrackUids[i], sourceCreator->channelForTrackUid(state.audioTrackUids[i]) };
        auto autoNode = nodeFactory->createAutomationNode(channel, parentTrack, parentTake);
        assert(autoNode);
        broadcast->elementAdded();
        auto success = state.currentNode->addChildNode(autoNode);
        assert(success);
        auto autoEl = std::static_pointer_cast<AutomationElement>(autoNode->getProjectElement());
        parentTrack->addAutomationElement(autoEl);
    }
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

std::shared_ptr<ProjectNode> admplug::ProjectTree::getCompatibleTakeNode(std::shared_ptr<const adm::AudioObject> object, std::vector<uint32_t> const& channelsOfOriginal, std::shared_ptr<ProjectNode> startingNode)
{
    if (!startingNode) {
        startingNode = rootNode;
    }
    if (nodeIsTakeWithCompatibleElements(*startingNode, channelsOfOriginal, object)) {
        return startingNode;
    }
    auto children = startingNode->children();
    for (auto child : children) {
        auto matchingNode = getCompatibleTakeNode(object, channelsOfOriginal, child);
        if (matchingNode) {
            return matchingNode;
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

bool admplug::ProjectTree::moveToCompatibleTakeNode(std::shared_ptr<const adm::AudioObject> object, std::vector<uint32_t> const& channelsOfOriginal)
{
    auto existingNode = getCompatibleTakeNode(object, channelsOfOriginal);
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
