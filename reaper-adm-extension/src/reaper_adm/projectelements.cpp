#include "projectelements.h"
#include "elementcomparator.h"
#include "color.h"
#include <helper/container_helpers.hpp>

namespace {
    std::vector<Color>rgbColorScheme{ {51,34,136}, {136, 204, 238}, {68, 170, 153}, {17, 119, 51}, {153, 153, 51}, {221, 204, 119}, {204, 102, 119}, {136, 34, 85}, {170, 68, 153} };
}

using namespace admplug;



TrackGroup::TrackGroup(int idVal) : idNum{idVal}
{

}

int TrackGroup::id() const {
    return idNum;
}

Color TrackGroup::color() const {
    return rgbColorScheme[idNum%rgbColorScheme.size()]; // TODO: Should check to see if this group is a child of another group and inherit that colour.
}



bool ProjectElement::hasAdmElement(adm::ElementConstVariant element) const
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

bool ProjectElement::hasAdmElements(std::vector<adm::ElementConstVariant> elements) const
{
    if(getAdmElements().size() != elements.size()) return false;

    for (auto element : elements) {
        if (!hasAdmElement(element)) {
            return false;
        }
    }
    return true;
}

bool ProjectElement::followsAdmElementSequence(std::vector<adm::ElementConstVariant> elements) const
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

bool ProjectElement::addParentProjectElement(std::weak_ptr<ProjectElement> newParentElement)
{
    // Elements without an override can't add additional parents - false return by default
    return false;
}



bool TrackElement::addParentProjectElement(std::weak_ptr<ProjectElement> newParentElement)
{
    // We can only add TrackElements as parents
    // RootElement parent is fine, but we don't need to store that
    if(auto lockedElement = newParentElement.lock()) {
        if(std::dynamic_pointer_cast<RootElement>(lockedElement)) return true;

        auto parentTrackElement = std::dynamic_pointer_cast<TrackElement>(lockedElement);
        if(!parentTrackElement) return false;
        for(auto el : parentElements) {
            if(auto lockEl = el.lock()) {
                if(lockEl == parentTrackElement) return true;
            }
        }
        parentElements.push_back(parentTrackElement);
        return true;
    }
    return false;
}

std::vector<std::shared_ptr<AutomationElement>> TrackElement::getAutomationElements()
{
    std::vector<std::shared_ptr<AutomationElement>> els;
    for(auto el : automationElements) {
        if(auto lockEl = el.lock()) {
            els.push_back(lockEl);
        }
    }
    return els;
}

void TrackElement::addAutomationElement(std::weak_ptr<AutomationElement> automation)
{
    automationElements.push_back(automation);
}

void TrackElement::setRepresentedAudioObject(std::shared_ptr<const adm::AudioObject> audioObject)
{
    representedAudioObject = audioObject;
}

std::shared_ptr<const adm::AudioObject> TrackElement::getRepresentedAudioObject()
{
    return representedAudioObject;
}

void TrackElement::setRepresentedAudioTrackUid(std::shared_ptr<const adm::AudioTrackUid> audioTrackUid)
{
    representedAudioTrackUid = audioTrackUid;
}

std::shared_ptr<const adm::AudioTrackUid> TrackElement::getRepresentedAudioTrackUid()
{
    return representedAudioTrackUid;
}

std::shared_ptr<Track> TrackElement::getTrack() const
{
    return track;
}

void TrackElement::setTrack(std::shared_ptr<Track> trk)
{
    track = trk;
}

std::shared_ptr<TakeElement> TrackElement::getTakeElement()
{
    return takeElement.lock();
}

void TrackElement::setTakeElement(std::weak_ptr<TakeElement> take)
{
    takeElement = take;
}



bool TakeElement::addParentProjectElement(std::weak_ptr<ProjectElement> newParentElement)
{
    // We can only add TrackElements as parents
    if(auto lockedElement = newParentElement.lock()) {
        auto parentTrackElement = std::dynamic_pointer_cast<TrackElement>(lockedElement);
        if(!parentTrackElement) return false;
        for(auto el : parents) {
            if(auto lockEl = el.lock()) {
                if(lockEl == parentTrackElement) return true;
            }
        }
        parents.push_back(parentTrackElement);
        return true;
    }
    return false;
}

int TakeElement::countParentElements()
{
    return parents.size();
}

bool AutomationElement::addParentProjectElement(std::weak_ptr<ProjectElement> newParentElement)
{
    // We can only add TrackElements as parents
    if(auto lockedElement = newParentElement.lock()) {
        auto parentTrackElement = std::dynamic_pointer_cast<TrackElement>(lockedElement);
        if(!parentTrackElement) return false;

        if(auto lockedCurrentParent = parentTrack_.lock()) {
            if(lockedCurrentParent == parentTrackElement) return true;
            // Can not set a parent if we already have one
            if(lockedCurrentParent) return false;
        }

        parentTrack_ = parentTrackElement;
        return true;
    }
    return false;
}