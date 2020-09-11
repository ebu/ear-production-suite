#pragma once
#include "projectelements.h"
#include "admchannel.h"
#include "automationpoint.h"

namespace admplug {
class Plugin;
class Track;
class Parameter;
class PluginParameter;
class TrackParameter;
class Track;

class ObjectAutomationElement : public ObjectAutomation {
public:
    ObjectAutomationElement(ADMChannel channel, std::shared_ptr<TakeElement> parentTake = nullptr);
    void createProjectElements(PluginSuite &pluginSuite, const ReaperAPI &api) override;
    adm::BlockFormatsConstRange<adm::AudioBlockFormatObjects> blocks() const override;
    double startTime() const override;
    std::shared_ptr<Track> getTrack() const override;
    ADMChannel channel() const override;
    void apply(const PluginParameter &parameter, const Plugin &plugin) const override;
    void apply(const TrackParameter &parameter, const Track &track) const override;
private:
    std::vector<adm::ElementConstVariant> getAdmElements() const override;
    std::vector<AutomationPoint> pointsFor(const Parameter &parameter) const;
    ADMChannel admChannel;
};
}
