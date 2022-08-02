#include "projectnode.h"
#include "importaction.h"
#include "progress/importlistener.h"

using namespace admplug;

ProjectNode::ProjectNode(std::shared_ptr<ProjectElement> element) :
    projectElement{ element }, createProjectElementsCalled{ false }
{
}

bool admplug::ProjectNode::addChildNode(std::shared_ptr<ProjectNode> childNode)
{
    if (childNode->getProjectElement()->addParentProjectElement(projectElement)) {
        if (std::find(childNodes.begin(), childNodes.end(), childNode) == childNodes.end()) {
            childNodes.emplace_back(childNode);
        }
        return true;
    }
    return false;
}

std::vector<std::shared_ptr<ProjectNode> > ProjectNode::children() const
{
    return childNodes;
}

std::shared_ptr<ProjectElement> ProjectNode::getProjectElement() const
{
    return projectElement;
}

std::shared_ptr<ProjectNode> ProjectNode::getChildWithElement(adm::ElementConstVariant element) const
{
    for(auto const& child : childNodes) {
        auto const& el = child->getProjectElement();
        if (el->hasAdmElement(element)) {
            return child;
        }
    }
    return nullptr;
}

void ProjectNode::createProjectElements(PluginSuite & pluginSuite,
                                        const ReaperAPI & api,
                                        ImportListener & broadcast)
{
    // Breadth-first-like approach to ensure all tracks are ready before doing the take and automation
    createProjectElementsByType<TrackElement>(pluginSuite, api, broadcast);
    createProjectElementsByType<TakeElement>(pluginSuite, api, broadcast);
    createProjectElementsByType<AutomationElement>(pluginSuite, api, broadcast);
}

template <typename T>
void ProjectNode::createProjectElementsByType(PluginSuite &pluginSuite,
                                        const ReaperAPI &api,
                                        ImportListener& broadcast)
{

    auto castProjectElement = std::dynamic_pointer_cast<T>(projectElement);
    if(castProjectElement) {
        if (createProjectElementsCalled) return;
        createProjectElementsCalled = true;
        projectElement->createProjectElements(pluginSuite, api);
        broadcast.elementCreated();
    }
    for(auto const& child : childNodes) {
        child->createProjectElementsByType<T>(pluginSuite, api, broadcast);
    }
}



