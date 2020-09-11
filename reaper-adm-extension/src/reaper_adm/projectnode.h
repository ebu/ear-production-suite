#pragma once
#include <memory>
#include <vector>
#include "projectelements.h"
//#include <adm/element_variant.hpp>

namespace admplug {
class PluginSuite;
class ReaperAPI;
class ImportListener;

class ProjectElement;

class ProjectNode {
public:
    ProjectNode(std::shared_ptr<ProjectElement> element);
    //std::shared_ptr<ProjectNode> addChild(std::shared_ptr<ProjectElement> child);
    bool addChildNode(std::shared_ptr<ProjectNode> childNode);
    //std::shared_ptr<ProjectNode> addExistingNodeAsChild(std::shared_ptr<ProjectNode> child);
    std::vector<std::shared_ptr<ProjectNode>> children() const;
    std::shared_ptr<ProjectElement> getProjectElement() const;
    std::shared_ptr<ProjectNode> getChildWithElement(adm::ElementConstVariant element) const;
    void createProjectElements(PluginSuite &pluginSuite, const ReaperAPI &api, admplug::ImportListener &broadcast);
private:
    std::shared_ptr<ProjectElement> projectElement;
    std::vector<std::shared_ptr<ProjectNode>> childNodes;
    bool createProjectElementsCalled{ false };
};

}
