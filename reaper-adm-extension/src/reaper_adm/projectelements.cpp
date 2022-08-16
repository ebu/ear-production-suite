#include "projectelements.h"
#include "elementcomparator.h"
#include "color.h"
#include <helper/container_helpers.hpp>

namespace {
    std::vector<Color>rgbColorScheme{ {51,34,136}, {136, 204, 238}, {68, 170, 153}, {17, 119, 51}, {153, 153, 51}, {221, 204, 119}, {204, 102, 119}, {136, 34, 85}, {170, 68, 153} };
}

bool admplug::ProjectElement::hasAdmElement(adm::ElementConstVariant element) const
{
    auto currentElements = getAdmElements();
    for (auto currentElement : currentElements) {
        ElementComparator elementComparator;
        if (boost::apply_visitor(elementComparator, element, currentElement)) {
            return true;
        }
    }
    return false;
}

bool admplug::ProjectElement::hasAdmElements(std::vector<adm::ElementConstVariant> elements) const
{
    if(getAdmElements().size() != elements.size()) return false;

    for (auto element : elements) {
        if (!hasAdmElement(element)) {
            return false;
        }
    }
    return true;
}

bool admplug::ProjectElement::followsAdmElementSequence(std::vector<adm::ElementConstVariant> elements) const
{
    auto currentElements = getAdmElements();
    if(currentElements.size() > elements.size()) return false;
    ElementComparator elementComparator;
    for(int i = 0; i < currentElements.size(); i++) {
        if (!boost::apply_visitor(elementComparator, elements[i], currentElements[i])) {
            return false;
        }
    }
    return true;
}

bool admplug::ProjectElement::addParentProjectElement(std::shared_ptr<ProjectElement> newParentElement)
{
    // Elements without an override can't add additional parents - false return by default
    return false;
}

admplug::TrackGroup::TrackGroup(int idVal) : idNum{idVal}
{

}

int admplug::TrackGroup::id() const {
    return idNum;
}

Color admplug::TrackGroup::color() const {
    return rgbColorScheme[idNum%rgbColorScheme.size()]; // TODO: Should check to see if this group is a child of another group and inherit that colour.
}

bool admplug::TakeElement::addParentProjectElement(std::shared_ptr<ProjectElement> newParentElement)
{
    // We can only add TrackElements as parents
    auto parentTrackElement = std::dynamic_pointer_cast<TrackElement>(newParentElement);
    if (!parentTrackElement) return false;
    if (contains(parents, parentTrackElement)) return true;
    parents.push_back(parentTrackElement);
    return true;
}

int admplug::TakeElement::countParentElements()
{
    return parents.size();
}

bool admplug::TrackElement::addParentProjectElement(std::shared_ptr<ProjectElement> newParentElement)
{
    // We can only add TrackElements as parents
    // RootElement parent is fine, but we don't need to store that
    if (std::dynamic_pointer_cast<RootElement>(newParentElement)) return true;

    auto parentTrackElement = std::dynamic_pointer_cast<TrackElement>(newParentElement);
    if (!parentTrackElement) return false;

    if (std::find(parentElements.begin(), parentElements.end(), parentTrackElement) == parentElements.end()) {
        parentElements.push_back(parentTrackElement);
    }
    return true;
}

bool admplug::ObjectAutomation::addParentProjectElement(std::shared_ptr<ProjectElement> newParentElement)
{
    // We can only add TrackElements as parents
    auto parentTrackElement = std::dynamic_pointer_cast<TrackElement>(newParentElement);
    if (!parentTrackElement) return false;
    if (parentTrack_ == parentTrackElement) return true;

    // Can not set a parent if we already have one
    if (parentTrack_) return false;

    parentTrack_ = parentTrackElement;
    return true;
}

bool admplug::DirectSpeakersAutomation::addParentProjectElement(std::shared_ptr<ProjectElement> newParentElement)
{
    // We can only add TrackElements as parents
    auto parentTrackElement = std::dynamic_pointer_cast<TrackElement>(newParentElement);
    if (!parentTrackElement) return false;
    if (parentTrack_ == parentTrackElement) return true;

    // Can not set a parent if we already have one
    if (parentTrack_) return false;

    parentTrack_ = parentTrackElement;
    return true;
}

bool admplug::HoaAutomation::addParentProjectElement(std::shared_ptr<ProjectElement> newParentElement) {
    // We can only add TrackElements as parents
    auto parentTrackElement = std::dynamic_pointer_cast<TrackElement>(newParentElement);
    if (!parentTrackElement) return false;
    if (parentTrack_ == parentTrackElement) return true;

    // Can not set a parent if we already have one
    if (parentTrack_) return false;

    parentTrack_ = parentTrackElement;
    return true;
    //TODO: Why not put in ProjectElement to prevent code repetition?
}
